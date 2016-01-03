#include "canrawsocket_p.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qsocketnotifier.h>
#include <QtCore/qmap.h>

#ifdef Q_OS_LINUX
#   include <sys/socket.h>
#   include <linux/can.h>
#   include <linux/can/raw.h>
#   include <linux/can/error.h>
#   include <sys/ioctl.h>
#   include <sys/time.h>
#   include <sys/types.h>
//#   include <linux/if.h>
#   include <errno.h>
#   include <unistd.h>
#   include <fcntl.h>
#else
#   error Unsupported OS
#endif

#ifndef CAN_MTU
#   define CAN_MTU sizeof(can_frame)
#endif

#ifndef CAN_MAX_DLC
#   define CAN_MAX_DLC 8
#endif

#ifndef CAN_MAX_DLEN
#   define CAN_MAX_DLEN 8
#endif

//reserved bytes according to can.h
#define RES0_BYTE 6
#define RES1_BYTE 7

quint8 res0FromCanMTU(int mtu)
{
    switch(mtu) {
    case CAN_MTU:
        return CAN_MAX_DLEN;
#ifdef CANFD_MTU
    case CANFD_MTU:
        return CANFD_MAX_DLEN;
#endif
    default:
        return 0;
    }
}

quint8 res1FromCanMTU(int mtu)
{
    switch(mtu) {
    case CAN_MTU:
        return CAN_MTU;
#ifdef CANFD_MTU
    case CANFD_MTU:
        return CANFD_MTU;
#endif
    default:
        return 0;
    }
}


class ReadNotifier : public QSocketNotifier
{
public:
    ReadNotifier(CanRawSocketPrivate *d, QObject *parent)
        : QSocketNotifier(d->descriptor, QSocketNotifier::Read, parent)
        , dptr(d)
    {
    }

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE
    {
        if (e->type() == QEvent::SockAct) {
            dptr->readNotification();
            return true;
        }
        return QSocketNotifier::event(e);
    }

private:
    CanRawSocketPrivate *dptr;
};

class WriteNotifier : public QSocketNotifier
{
public:
    WriteNotifier(CanRawSocketPrivate *d, QObject *parent)
        : QSocketNotifier(d->descriptor, QSocketNotifier::Write, parent)
        , dptr(d)
    {
    }

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE
    {
        if (e->type() == QEvent::SockAct) {
            dptr->completeAsyncWrite();
            return true;
        }
        return QSocketNotifier::event(e);
    }

private:
    CanRawSocketPrivate *dptr;
};

bool CanRawSocketPrivate::open(QIODevice::OpenMode mode)
{
    struct ifreq ifr;
    struct sockaddr_can addr;

    descriptor = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (descriptor == -1) {
        setError(getSystemError());
        return false;
    }

    if (::fcntl(descriptor, F_SETFL , O_NONBLOCK) == -1) {
        setError(getSystemError());
        return false;
    }

    addr.can_family = AF_CAN;

    if (interfaceName.isEmpty())
        addr.can_ifindex = 0;
    else {
        ::strcpy(ifr.ifr_name, interfaceName.toLocal8Bit().constData());
        if (::ioctl(descriptor, SIOCGIFINDEX, &ifr) == -1) {
            setError(getSystemError());
            return false;
        }
        addr.can_ifindex = ifr.ifr_ifindex;
    }

    if (::bind(descriptor, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        setError(getSystemError());
        return false;
    }

    if (mode & QIODevice::ReadOnly)
        setReadNotficationEnabled(true);

    return true;
}

void CanRawSocketPrivate::close()
{
    if (readNotifier) {
        delete readNotifier;
        readNotifier = Q_NULLPTR;
    }

    if (writeNotifier) {
        delete writeNotifier;
        writeNotifier = Q_NULLPTR;
    }

    if (::close(descriptor) == -1)
        setError(getSystemError());

    descriptor = -1;
    pendingBytesWritten = 0;
    writeSequenceStarted = false;
}

bool CanRawSocketPrivate::flush()
{
    return completeAsyncWrite();
}

bool CanRawSocketPrivate::waitForReadyRead(int msecs)
{
    QElapsedTimer stopWatch;
    stopWatch.start();

    do {
        bool readyToRead = false;
        bool readyToWrite = false;

        if (!waitForReadOrWrite(&readyToRead, &readyToWrite, true, !writeBuffer.isEmpty(),
                                timeoutValue(msecs, stopWatch.elapsed()))) {
            return false;
        }

        if (readyToRead) {
            return readNotification();
        }

        if (readyToWrite && !completeAsyncWrite())
            return false;
    } while (msecs == -1 || timeoutValue(msecs,stopWatch.elapsed()) > 0);
    return false;
}

bool CanRawSocketPrivate::waitForBytesWritten(int msecs)
{
    if (writeBuffer.isEmpty() && pendingBytesWritten <= 0)
        return false;

    QElapsedTimer stopWatch;
    stopWatch.start();

    forever {
        bool readyToRead = false;
        bool readyToWrite = false;
        if (!waitForReadOrWrite(&readyToRead, &readyToWrite, true, !writeBuffer.isEmpty(),
                                timeoutValue(msecs, stopWatch.elapsed()))) {
            return false;
        }

        if (readyToRead && !readNotification())
            return false;

        if (readyToWrite)
            return completeAsyncWrite();
    }
    return false;
}

bool CanRawSocketPrivate::setErrorFilterMask(CanRawSocket::ErrorFilterMask errorFilterMask)
{
    can_err_mask_t err_mask = 0;

    if (!errorFilterMask.testFlag(CanRawSocket::NoError)) {

        if (errorFilterMask.testFlag(CanRawSocket::TXTimeoutError))
            err_mask = err_mask | CAN_ERR_TX_TIMEOUT;

        if (errorFilterMask.testFlag(CanRawSocket::LostArbitrationError))
            err_mask = err_mask | CAN_ERR_LOSTARB;

        if (errorFilterMask.testFlag(CanRawSocket::ControllerProblemsError))
            err_mask = err_mask | CAN_ERR_CRTL;

        if (errorFilterMask.testFlag(CanRawSocket::ProtocolViolationsError))
            err_mask = err_mask | CAN_ERR_PROT;

        if (errorFilterMask.testFlag(CanRawSocket::TransceiverStatusError))
            err_mask = err_mask | CAN_ERR_TRX;

        if (errorFilterMask.testFlag(CanRawSocket::ReceivedNoACKOnTransmissionError))
            err_mask = err_mask | CAN_ERR_ACK;

        if (errorFilterMask.testFlag(CanRawSocket::BusOffError))
            err_mask = err_mask | CAN_ERR_BUSOFF;

        if (errorFilterMask.testFlag(CanRawSocket::BusError))
            err_mask = err_mask | CAN_ERR_BUSERROR;

        if (errorFilterMask.testFlag(CanRawSocket::ControllerRestartedError))
            err_mask = err_mask | CAN_ERR_RESTARTED;
    }

    if (::setsockopt(descriptor, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask)) == -1) {
        setError(getSystemError());
        return false;
    }

    return true;
}

bool CanRawSocketPrivate::setLoopback(CanRawSocket::Loopback loopback)
{
    int can_raw_loopback;

    if (loopback == CanRawSocket::EnabledLoopback)
        can_raw_loopback = 1;
    else
        can_raw_loopback = 0;

    if (::setsockopt(descriptor, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &can_raw_loopback, sizeof(can_raw_loopback)) == -1) {
        setError(getSystemError());
        return false;
    }

    return true;
}

bool CanRawSocketPrivate::setReceiveOwnMessages(CanRawSocket::ReceiveOwnMessages receiveOwnMessages)
{
    int recv_own_msgs;

    if (receiveOwnMessages == CanRawSocket::EnabledOwnMessages)
        recv_own_msgs = 1;
    else
        recv_own_msgs = 0;

    if (::setsockopt(descriptor, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof(recv_own_msgs)) == -1) {
        setError(getSystemError());
        return false;
    }

    return true;
}

bool CanRawSocketPrivate::setFlexibleDataRateFrames(CanRawSocket::FlexibleDataRateFrames flexibleDataRateFrames)
{

#ifdef CANFD_MTU
    int can_raw_fd_frames;

    if (flexibleDataRateFrames == CanRawSocket::EnabledFDFrames)
        can_raw_fd_frames = 1;
    else
        can_raw_fd_frames = 0;

    //check if device supports fd frames
    struct ifreq ifr;
    if (can_raw_fd_frames) {
        if (::ioctl(descriptor, SIOCGIFMTU, &ifr) == -1) {
            setError(getSystemError());
            return false;
        }
        else if (ifr.ifr_mtu != CANFD_MTU) {
            setError(CanSocketErrorInfo(CanRawSocket::UnsupportedOperationError, CanRawSocket::tr("Device doesn't support flexible data rate frames")));
            return false;
        }
    }

    if (::setsockopt(descriptor, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &can_raw_fd_frames, sizeof(can_raw_fd_frames)) == -1) {
        setError(getSystemError());
        return false;
    }

    return true;
#else
    Q_UNUSED(flexibleDataRateFrames)
    return false;
#endif
}

bool CanRawSocketPrivate::readNotification()
{
    Q_Q(CanRawSocket);

#ifdef CANFD_MTU
    qint64 frameSize;
    if (flexibleDataRateFrames == CanRawSocket::EnabledFDFrames)
        frameSize = CANFD_MTU;
    else
        frameSize = CAN_MTU;
#else
    qint64 frameSize = CAN_MTU;
#endif


    // Always buffered, read data from the socket into the read buffer
    qint64 newBytes = buffer.size();
    qint64 bytesToRead = ReadChunkSize;

    if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - buffer.size())) {
        bytesToRead = readBufferMaxSize - buffer.size();
        if (bytesToRead < frameSize) {
            // Buffer is full. User must read data from the buffer
            // before we can read more from the can socket.
            return false;
        }
    }

    char *ptr = buffer.reserve(bytesToRead);
    const qint64 readBytes = readFromSocket(ptr, bytesToRead);

    if (readBytes < 0) {
        CanSocketErrorInfo error = getSystemError();
        if (error.errorCode != CanRawSocket::ResourceError)
            error.errorCode = CanRawSocket::ReadError;
        else
            setReadNotficationEnabled(false);
        setError(error);
        buffer.chop(bytesToRead);
        return false;
    }

    buffer.chop(bytesToRead - qMax(readBytes, qint64(0)));

    newBytes = buffer.size() - newBytes;

    // If read buffer is full, disable the read notifier.
    if (readBufferMaxSize && buffer.size() > (readBufferMaxSize - frameSize) )
        setReadNotficationEnabled(false);

    // only emit readyRead() when not recursing, and only if there is data available
    const bool hasData = newBytes > 0;

    if (!emittedReadyRead && hasData) {
        emittedReadyRead = true;
        emit q->readyRead();
        emittedReadyRead = false;
    }

    return true;
}

bool CanRawSocketPrivate::startAsyncWrite()
{
    if (writeBuffer.isEmpty() || writeSequenceStarted)
        return true;

    // Attempt to write it all in one chunk.
    qint64 writtenBytes = writeToSocket(writeBuffer.readPointer(), writeBuffer.nextDataBlockSize());
    if (writtenBytes < 0) {
        CanSocketErrorInfo error = getSystemError();
        if (error.errorCode != CanRawSocket::ResourceError)
            error.errorCode = CanRawSocket::WriteError;
        setError(error);
        return false;
    }

    writeBuffer.free(writtenBytes);
    pendingBytesWritten += writtenBytes;
    writeSequenceStarted = true;

    if (!isWriteNotificationEnabled())
        setWriteNotificationEnabled(true);
    return true;
}

bool CanRawSocketPrivate::completeAsyncWrite()
{
    Q_Q(CanRawSocket);

    if (pendingBytesWritten > 0) {
        if (!emittedBytesWritten) {
            emittedBytesWritten = true;
            emit q->bytesWritten(pendingBytesWritten);
            pendingBytesWritten = 0;
            emittedBytesWritten = false;
        }
    }

    writeSequenceStarted = false;

    if (writeBuffer.isEmpty()) {
        setWriteNotificationEnabled(false);
        return true;
    }

    return startAsyncWrite();
}

CanSocketErrorInfo CanRawSocketPrivate::getSystemError(int systemErrorCode) const
{
    if (systemErrorCode == -1)
        systemErrorCode = errno;

    CanSocketErrorInfo error;
    error.errorString = qt_error_string(systemErrorCode);

    switch (systemErrorCode) {
    case ENODEV:
        error.errorCode = CanRawSocket::DeviceNotFoundError;
        error.errorString = CanRawSocket::tr("No such device system error");
        break;
    case EACCES:
        error.errorCode = CanRawSocket::PermissionError;
        error.errorString = CanRawSocket::tr("Permission denied system error");
        break;
    case EAFNOSUPPORT:
        error.errorCode = CanRawSocket::UnsupportedOperationError;
        error.errorString = CanRawSocket::tr("Address family not supported by protocol system error");
        break;
    case EINVAL:
        error.errorCode = CanRawSocket::UnsupportedOperationError;
        error.errorString = CanRawSocket::tr("Invalid argument system error");
        break;
    case EMFILE:
        error.errorCode = CanRawSocket::ResourceError;
        error.errorString = CanRawSocket::tr("Too many open files system error");
        break;
    case ENFILE:
        error.errorCode = CanRawSocket::ResourceError;
        error.errorString = CanRawSocket::tr("File table overflow system error");
        break;
    case ENOBUFS:
        error.errorCode = CanRawSocket::ResourceError;
        error.errorString = CanRawSocket::tr("No buffer space available system error");
        break;
    case ENOMEM:
        error.errorCode = CanRawSocket::ResourceError;
        error.errorString = CanRawSocket::tr("Out of memory system error");
        break;
    case EPROTONOSUPPORT:
        error.errorCode = CanRawSocket::UnsupportedOperationError;
        error.errorString = CanRawSocket::tr("Protocol not supported system error");
        break;
    default:
        error.errorCode = CanRawSocket::UnknownError;
        break;
    }
    return error;
}

qint64 CanRawSocketPrivate::writeData(const char *data, qint64 maxSize)
{
    ::memcpy(writeBuffer.reserve(maxSize), data, maxSize);
    if (!writeBuffer.isEmpty() && !isWriteNotificationEnabled())
        setWriteNotificationEnabled(true);
    return maxSize;
}

bool CanRawSocketPrivate::isReadNotificationEnabled() const
{
    return readNotifier && readNotifier->isEnabled();
}

void CanRawSocketPrivate::setReadNotficationEnabled(bool enable)
{
    Q_Q(CanRawSocket);

    if (readNotifier) {
        readNotifier->setEnabled(enable);
    } else if (enable) {
        readNotifier = new ReadNotifier(this, q);
        readNotifier->setEnabled(true);
    }
}

bool CanRawSocketPrivate::isWriteNotificationEnabled() const
{
    return writeNotifier && writeNotifier->isEnabled();
}

void CanRawSocketPrivate::setWriteNotificationEnabled(bool enable)
{
    Q_Q(CanRawSocket);

    if(writeNotifier) {
        writeNotifier->setEnabled(enable);
    } else if (enable) {
        writeNotifier = new WriteNotifier(this, q);
        writeNotifier->setEnabled(true);
    }
}

bool CanRawSocketPrivate::waitForReadOrWrite(bool *selectForRead, bool *selectForWrite, bool checkRead, bool checkWrite, int msecs)
{
    Q_ASSERT(selectForRead);
    Q_ASSERT(selectForWrite);

    fd_set fdread;
    FD_ZERO(&fdread);
    if (checkRead)
        FD_SET(descriptor, &fdread);

    fd_set fdwrite;
    FD_ZERO(&fdwrite);
    if (checkWrite)
        FD_SET(descriptor, &fdwrite);

    struct timeval tv;
    tv.tv_sec = msecs / 1000;
    tv.tv_usec = (msecs % 1000) * 1000;

    const int ret = ::select(descriptor+1, &fdread, &fdwrite, 0, msecs < 0 ? 0 : &tv);
    if (ret < 0) {
        setError(getSystemError());
        return false;
    }
    if (ret == 0) {
        setError(CanSocketErrorInfo(CanRawSocket::TimeoutError));
        return false;
    }

    *selectForRead = FD_ISSET(descriptor, &fdread);
    *selectForWrite = FD_ISSET(descriptor, &fdwrite);

    return true;
}

qint64 CanRawSocketPrivate::readFromSocket(char *data, qint64 maxSize)
{
    size_t frameSize = CAN_MTU;

#ifdef CANFD_MTU
    if (flexibleDataRateFrames == CanRawSocket::EnabledFDFrames)
        frameSize = CANFD_MTU;
#endif

    qint64 readBytes = 0;
    int ret;

    while (readBytes <= maxSize - (qint64)frameSize) {

        ret = ::read(descriptor, data, frameSize);

        if (ret < 0) {
            if (errno == EAGAIN)
                break;
            return -1;
        }

        if (ret == 0)
            break;

        /* we add aditional data (max dlen and mtu) to reserved pading bytes
            (__res0 and __res1, see can.h) in order to distinct between the two frame types
        */
        if (ret == CAN_MTU) {
            // received can frame in can or canfd mode
            data[RES0_BYTE] = res0FromCanMTU(CAN_MTU);
            data[RES1_BYTE] = res1FromCanMTU(CAN_MTU);
        }
#ifdef CANFD_MTU
        else if (ret == CANFD_MTU && static_cast<int>(frameSize) == ret) {
            // received canfd frame in canfd mode
            data[RES0_BYTE] = res0FromCanMTU(CANFD_MTU);
            data[RES1_BYTE] = res1FromCanMTU(CANFD_MTU);
        }
#endif
        else
            return -1; // ret is not valid

        data += ret;
        readBytes += ret;
    }
    return readBytes;
}

qint64  CanRawSocketPrivate::writeToSocket(const char *data, qint64 maxSize)
{
    size_t frameSize;
    quint8 res0;
    quint8 res1;

    qint64 writtenBytes = 0;
    int ret;

    forever {

        //get reserved bytes that define frame type (can or canfd)
        res0 = data[RES0_BYTE];
        res1 = data[RES1_BYTE];

        if (res0 == res0FromCanMTU(CAN_MTU)
                && res1 == res1FromCanMTU(CAN_MTU) ) {
            //standard can frame can be written in can and in canfd mode
            frameSize = CAN_MTU;
        }
#ifdef CANFD_MTU
        else if (res0 == res0FromCanMTU(CANFD_MTU)
                 && res1 == res1FromCanMTU(CANFD_MTU)
                 && flexibleDataRateFrames == CanRawSocket::EnabledFDFrames) {
            //fd frame can only be written in fd mode
            frameSize = CANFD_MTU;
        }
#endif
        else {
            //data for reserved bytes is incorrect
            break;
        }


        if (maxSize - writtenBytes < static_cast<qint64>(frameSize))  {
            //leftof size smaller then frame size
            break;
        }


        ret = ::write(descriptor, data, frameSize);

        if (ret == 0) {
            break;
        }
        else if (ret < 0) {
            if (errno == ENOBUFS)
                break;
            return -1;
        }
        else if (ret < static_cast<int>(frameSize)) {
            return -1;
        }

        data += ret;
        writtenBytes += ret;
    }
    return writtenBytes;
}
