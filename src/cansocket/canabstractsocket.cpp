/****************************************************************************
* cansocket-qt.lib - Qt socketcan library
* Copyright (C) 2016 Georgije Bosiger <gbosiger@gmail.com>
*
* This library is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library. If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "canabstractsocket.h"
#include "canabstractsocket_p.h"
#include "canframe_p.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qsocketnotifier.h>
#include <QtCore/qmap.h>

#ifdef Q_OS_LINUX
#   include <errno.h>
#   include <unistd.h>
#else
#   error Unsupported OS
#endif


CanAbstractSocket::CanAbstractSocket(SocketType socketType, QObject *parent)
    : QIODevice(*new CanAbstractSocketPrivate, parent)
{
    setSocketType(socketType);
}

CanAbstractSocket::~CanAbstractSocket()
{
    if (socketState() != UnconnectedState)
        close();
}

bool CanAbstractSocket::connectToInterface(const QString &interfaceName, OpenMode mode)
{
    Q_D(CanAbstractSocket);

    if (socketState() != CanAbstractSocket::UnconnectedState) {
        setSocketError(CanAbstractSocket::OperationError, tr("Trying to connect while connection is in progress"));
        return false;
    }

   static const OpenMode unsupportedModes = Append | Truncate | Text | Unbuffered;
   if ((mode & unsupportedModes) || mode == NotOpen) {
       setSocketError(CanAbstractSocket::UnsupportedSocketOperationError, tr("Unsupported open mode"));
       return false;
   }

   d->interfaceName = interfaceName;
   d->state = UnconnectedState;
   d->buffer.clear();
   d->writeBuffer.clear();

   QIODevice::open(mode);

   setSocketState(ConnectingState);

   if (!d->connectToInterface(interfaceName)) {
       close();
       return false;
   }

   if (mode & QIODevice::ReadOnly)
       d->setReadNotficationEnabled(true);

   setSocketState(ConnectedState);

   return true;
}

void CanAbstractSocket::disconnectFromInterface()
{
    close();
}

void CanAbstractSocket::close()
{
    Q_D(CanAbstractSocket);

    if (socketState() == UnconnectedState)
        return;

    QIODevice::close();

    setSocketState(ClosingState);

    d->disconnectFromInterface();

    setSocketState(UnconnectedState);
}

bool CanAbstractSocket::isSequential() const
{
    return true;
}

/*! \reimp

     Returns \c true if no more data is currently
     available for reading; otherwise returns \c false.

     This function is most commonly used when reading data from the
     socket in a loop.

     \sa bytesAvailable(), readyRead()
 */
bool CanAbstractSocket::atEnd() const
{
    return QIODevice::atEnd();
}

bool CanAbstractSocket::canReadLine() const
{
    return QIODevice::canReadLine();
}

bool CanAbstractSocket::flush()
{
    Q_D(CanAbstractSocket);
    return d->completeAsyncWrite();
}

QString CanAbstractSocket::interfaceName() const
{
    Q_D(const CanAbstractSocket);
    return d->interfaceName;
}

qintptr CanAbstractSocket::socketDescriptor() const
{
    Q_D(const CanAbstractSocket);
    return d->descriptor;
}

CanAbstractSocket::SocketType CanAbstractSocket::socketType() const
{
    Q_D(const CanAbstractSocket);
    return d->type;
}

CanAbstractSocket::SocketError CanAbstractSocket::error() const
{
    Q_D(const CanAbstractSocket);
    return d->error;
}

QString CanAbstractSocket::errorString() const
{
    Q_D(const CanAbstractSocket);
    return d->errorString;
}

CanAbstractSocket::SocketState CanAbstractSocket::socketState() const
{
    Q_D(const CanAbstractSocket);
    return d->state;
}

void CanAbstractSocket::clearError()
{
    setSocketError(CanAbstractSocket::NoError, QString());
}

void CanAbstractSocket::setReadBufferSize(qint64 size)
{
    Q_D(CanAbstractSocket);

    if (d->readBufferMaxSize == size)
        return;
    d->readBufferMaxSize = size;
}

qint64 CanAbstractSocket::readBufferSize() const
{
    Q_D(const CanAbstractSocket);
    return d->readBufferMaxSize;
}

qint64 CanAbstractSocket::bytesAvailable() const
{
    return QIODevice::bytesAvailable();
}

qint64 CanAbstractSocket::bytesToWrite() const
{
    Q_D(const CanAbstractSocket);
    return d->writeBuffer.size();
}

bool CanAbstractSocket::waitForReadyRead(int msecs)
{
    Q_D(CanAbstractSocket);

    if (socketState() == UnconnectedState)
        return false;

    return d->waitForReadyRead(msecs);
}

bool CanAbstractSocket::waitForBytesWritten(int msecs)
{
    Q_D(CanAbstractSocket);
    return d->waitForBytesWritten(msecs);
}


// This function does not really read anything, as we use QIODevicePrivate's
// buffer. The buffer will be read inside of QIODevice before this
// method will be called.
qint64 CanAbstractSocket::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    return qint64(0);
}

qint64 CanAbstractSocket::readLineData(char *data, qint64 maxSize)
{
    return QIODevice::readLineData(data, maxSize);
}

qint64 CanAbstractSocket::writeData(const char *data, qint64 maxSize)
{
    Q_D(CanAbstractSocket);
    return d->writeData(data, maxSize);
}

void CanAbstractSocket::setSocketType(SocketType socketType)
{
    Q_D(CanAbstractSocket);
    d->type = socketType;
}

void CanAbstractSocket::setSocketState(SocketState socketState)
{
    Q_D(CanAbstractSocket);

    SocketState old = d->state;
    d->state = socketState;
    if (old != d->state)
        emit stateChanged(socketState);
}

void CanAbstractSocket::setSocketError(SocketError socketError, QString error)
{
    Q_D(CanAbstractSocket);

    d->setError(CanAbstractSocketErrorInfo(socketError, error));
}

CanAbstractSocket::CanAbstractSocket(SocketType socketType, CanAbstractSocketPrivate &dd, QObject *parent)
    : QIODevice(dd, parent)
{
    setSocketType(socketType);
}

CanAbstractSocketErrorInfo::CanAbstractSocketErrorInfo(CanAbstractSocket::SocketError newErrorCode,
                                             const QString &newErrorString)
    : errorCode(newErrorCode)
    , errorString(newErrorString)
{
    if (errorString.isNull()) {
        switch (errorCode) {
        case CanAbstractSocket::UnkownSocketError:
            errorString = CanAbstractSocket::tr("Unknown Error");
            break;
        case CanAbstractSocket::NoSuchDeviceError:
            errorString = CanAbstractSocket::tr("No such device");
            break;
        case CanAbstractSocket::SocketAccessError:
            errorString = CanAbstractSocket::tr("Permission denied");
            break;
        case CanAbstractSocket::SocketResourceError:
            errorString = CanAbstractSocket::tr("Resource error");
            break;
        case CanAbstractSocket::SocketTimeoutError:
            errorString = CanAbstractSocket::tr("Operation timed out");
            break;
        case CanAbstractSocket::UnsupportedSocketOperationError:
            errorString = CanAbstractSocket::tr("Unsupported socket operation");
            break;
        case CanAbstractSocket::OperationError:
            errorString = CanAbstractSocket::tr("Operation error");
            break;
        case CanAbstractSocket::WriteError:
            errorString = CanAbstractSocket::tr("Write error");
            break;
        case CanAbstractSocket::ReadError:
            errorString = CanAbstractSocket::tr("Read error");
            break;
        case CanAbstractSocket::NoError:
            errorString = CanAbstractSocket::tr("No error");
            break;
        default:
            break;
        }
    }
}

class ReadNotifier : public QSocketNotifier
{
public:
    ReadNotifier(CanAbstractSocketPrivate *d, QObject *parent)
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
    CanAbstractSocketPrivate *dptr;
};

class WriteNotifier : public QSocketNotifier
{
public:
    WriteNotifier(CanAbstractSocketPrivate *d, QObject *parent)
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
    CanAbstractSocketPrivate *dptr;
};

CanAbstractSocketPrivate::CanAbstractSocketPrivate()
    : readBufferMaxSize(0)
    , writeBuffer(InitialBufferSize)
    , type(CanAbstractSocket::UnkownCanSocketType)
    , error(CanAbstractSocket::NoError)
    , state(CanAbstractSocket::UnconnectedState)
    , interfaceName()
    , descriptor(-1)
    , readNotifier(Q_NULLPTR)
    , writeNotifier(Q_NULLPTR)
    , readSocketNotifierCalled(false)
    , readSocketNotifierState(false)
    , readSocketNotifierStateSet(false)
    , emittedReadyRead(false)
    , emittedBytesWritten(false)
    , pendingBytesWritten(0)
    , writeSequenceStarted(false)
{
}

CanAbstractSocketPrivate::~CanAbstractSocketPrivate()
{
}

int CanAbstractSocketPrivate::timeoutValue(int msecs, int elapsed)
{
    if (msecs == -1)
        return msecs;
    msecs -= elapsed;
    return qMax(msecs, 0);
}

bool CanAbstractSocketPrivate::connectToInterface(const QString &interfaceName)
{
    Q_UNUSED(interfaceName);
    return false;
}

void CanAbstractSocketPrivate::disconnectFromInterface()
{
    if (readNotifier) {
        delete readNotifier;
        readNotifier = Q_NULLPTR;
    }

    if (writeNotifier) {
        delete writeNotifier;
        writeNotifier = Q_NULLPTR;
    }

    if (descriptor && ::close(descriptor) == -1)
        setError(getSystemError());

    interfaceName.clear();
    descriptor = -1;
    pendingBytesWritten = 0;
    writeSequenceStarted = false;
}

CanAbstractSocketErrorInfo CanAbstractSocketPrivate::getSystemError(int systemErrorCode) const
{
    if (systemErrorCode == -1)
        systemErrorCode = errno;

    CanAbstractSocketErrorInfo error;
    error.errorString = qt_error_string(systemErrorCode);

    switch (systemErrorCode) {
    case ENODEV:
        error.errorCode = CanAbstractSocket::NoSuchDeviceError;
        error.errorString = CanAbstractSocket::tr("No such device system error");
        break;
    case EACCES:
        error.errorCode = CanAbstractSocket::SocketAccessError;
        error.errorString = CanAbstractSocket::tr("Permission denied system error");
        break;
    case EAFNOSUPPORT:
        error.errorCode = CanAbstractSocket::UnsupportedSocketOperationError;
        error.errorString = CanAbstractSocket::tr("Address family not supported by protocol system error");
        break;
    case EINVAL:
        error.errorCode = CanAbstractSocket::UnsupportedSocketOperationError;
        error.errorString = CanAbstractSocket::tr("Invalid argument system error");
        break;
    case EMFILE:
        error.errorCode = CanAbstractSocket::SocketResourceError;
        error.errorString = CanAbstractSocket::tr("Too many open files system error");
        break;
    case ENFILE:
        error.errorCode = CanAbstractSocket::SocketResourceError;
        error.errorString = CanAbstractSocket::tr("File table overflow system error");
        break;
    case ENOBUFS:
        error.errorCode = CanAbstractSocket::SocketResourceError;
        error.errorString = CanAbstractSocket::tr("No buffer space available system error");
        break;
    case ENOMEM:
        error.errorCode = CanAbstractSocket::SocketResourceError;
        error.errorString = CanAbstractSocket::tr("Out of memory system error");
        break;
    case EPROTONOSUPPORT:
        error.errorCode = CanAbstractSocket::UnsupportedSocketOperationError;
        error.errorString = CanAbstractSocket::tr("Protocol not supported system error");
        break;
    default:
        error.errorCode = CanAbstractSocket::UnkownSocketError;
        break;
    }
    return error;
}

void CanAbstractSocketPrivate::setError(const CanAbstractSocketErrorInfo &errorInfo)
{
    Q_Q(CanAbstractSocket);

    error = errorInfo.errorCode;
    q->setErrorString(errorInfo.errorString);
    emit q->error(error);
}

qint64 CanAbstractSocketPrivate::writeData(const char *data, qint64 maxSize)
{
    ::memcpy(writeBuffer.reserve(maxSize), data, maxSize);
    if (!writeBuffer.isEmpty() && !isWriteNotificationEnabled())
        setWriteNotificationEnabled(true);
    return maxSize;
}

bool CanAbstractSocketPrivate::waitForReadyRead(int msecs)
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
    } while (msecs == -1 || timeoutValue(msecs, stopWatch.elapsed()) > 0);
    return false;
}

bool CanAbstractSocketPrivate::waitForBytesWritten(int msecs)
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

bool CanAbstractSocketPrivate::readNotification()
{
    Q_Q(CanAbstractSocket);

    int expectedDataSize = msgSize();

    // Always buffered, read data from the socket into the read buffer
    qint64 newBytes = buffer.size();
    qint64 bytesToRead = ReadChunkSize;

    if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - buffer.size())) {
        bytesToRead = readBufferMaxSize - buffer.size();
        if (bytesToRead < expectedDataSize) {
            // Buffer is full. User must read data from the buffer
            // before we can read more from the can socket.
            return false;
        }
    }

    char *ptr = buffer.reserve(bytesToRead);
    const qint64 readBytes = readFromSocket(ptr, bytesToRead);

    if (readBytes < 0) {
        CanAbstractSocketErrorInfo error = getSystemError();
        if (error.errorCode != CanAbstractSocket::SocketResourceError)
            error.errorCode = CanAbstractSocket::ReadError;
        else
            setReadNotficationEnabled(false);
        setError(error);
        buffer.chop(bytesToRead);
        return false;
    }

    buffer.chop(bytesToRead - qMax(readBytes, qint64(0)));

    newBytes = buffer.size() - newBytes;

    // If read buffer is full, disable the read notifier.
    if (readBufferMaxSize && buffer.size() > (readBufferMaxSize - expectedDataSize) )
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

bool CanAbstractSocketPrivate::startAsyncWrite()
{
    if (writeBuffer.isEmpty() || writeSequenceStarted)
        return true;

    // Attempt to write it all in one chunk.
    qint64 writtenBytes = writeToSocket(writeBuffer.readPointer(), writeBuffer.nextDataBlockSize());
    if (writtenBytes < 0) {
        CanAbstractSocketErrorInfo error = getSystemError();
        if (error.errorCode != CanAbstractSocket::SocketResourceError)
            error.errorCode = CanAbstractSocket::WriteError;
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

bool CanAbstractSocketPrivate::completeAsyncWrite()
{
    Q_Q(CanAbstractSocket);

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

bool CanAbstractSocketPrivate::isReadNotificationEnabled() const
{
    return readNotifier && readNotifier->isEnabled();
}

void CanAbstractSocketPrivate::setReadNotficationEnabled(bool enable)
{
    Q_Q(CanAbstractSocket);

    if (readNotifier) {
        readNotifier->setEnabled(enable);
    } else if (enable) {
        readNotifier = new ReadNotifier(this, q);
        readNotifier->setEnabled(true);
    }
}

bool CanAbstractSocketPrivate::isWriteNotificationEnabled() const
{
    return writeNotifier && writeNotifier->isEnabled();
}

void CanAbstractSocketPrivate::setWriteNotificationEnabled(bool enable)
{
    Q_Q(CanAbstractSocket);

    if(writeNotifier) {
        writeNotifier->setEnabled(enable);
    } else if (enable) {
        writeNotifier = new WriteNotifier(this, q);
        writeNotifier->setEnabled(true);
    }
}

bool CanAbstractSocketPrivate::waitForReadOrWrite(bool *selectForRead, bool *selectForWrite, bool checkRead, bool checkWrite, int msecs)
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
        setError(CanAbstractSocketErrorInfo(CanAbstractSocket::SocketTimeoutError));
        return false;
    }

    *selectForRead = FD_ISSET(descriptor, &fdread);
    *selectForWrite = FD_ISSET(descriptor, &fdwrite);

    return true;
}

int CanAbstractSocketPrivate::msgSize() const {
    return 0;
}

qint64 CanAbstractSocketPrivate::readFromSocket(char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

qint64 CanAbstractSocketPrivate::writeToSocket(const char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

//CanRawFilter::CanRawFilter(quint32 id, quint32 mask)
//    : m_id(id)
//    , m_mask(mask)
//{
//}

//quint32 CanRawFilter::id() const
//{
//    return m_id;
//}

//void CanRawFilter::setId(quint32 id)
//{
//    m_id = id;
//}

//quint32 CanRawFilter::mask() const
//{
//    return m_mask;
//}

//void CanRawFilter::setMask(quint32 mask)
//{
//    m_mask = mask;
//}

//bool CanRawFilter::operator ==(const CanRawFilter &rhs) const
//{
//    return (m_id == rhs.m_id && m_mask == )
//}
