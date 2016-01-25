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

#include "canrawsocket.h"
#include "canabstractsocket.h"
#include "canabstractsocket_p.h"
#include "canrawsocket_p.h"
#include "canframe_p.h"

#include <QtCore/qshareddata.h>
#include <QtCore/qmap.h>

#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define CAN_RAW_READ_CHUNK_SIZE 1152 // 72 CAN Frames or 16 FD CAN Frames
#define CAN_RAW_INITIAL_BUFFER_SIZE 18432 // x16

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

CanRawFilter::CanRawFilter(uint id, uint mask)
    : id(id)
    , mask(mask)
{
}

CanRawFilter::CanRawFilter(const CanRawFilter &rhs)
    : id(rhs.id)
    , mask(rhs.mask)
{
}

void CanRawFilter::setupFilter(uint id, uint mask)
{
    this->id = id;
    this->mask = mask;
}



class CanRawFilterArrayData : public QSharedData
{
public:
    CanRawFilterArrayData() : QSharedData(), filterArray() {}
    CanRawFilterArrayData(int size) : QSharedData(), filterArray(size)  {}
    CanRawFilterArrayData(int size, const CanRawFilter &filter)
        : QSharedData()
        , filterArray(size, filter) {}
    ~CanRawFilterArrayData() {}


    QVector<CanRawFilter> filterArray;

};

CanRawFilterArray::CanRawFilterArray() : d(new CanRawFilterArrayData())
{
}

CanRawFilterArray::CanRawFilterArray(int size) : d(new CanRawFilterArrayData(size))
{
}

CanRawFilterArray::CanRawFilterArray(int size, const CanRawFilter &filter)
    : d(new CanRawFilterArrayData(size, filter))
{
}

CanRawFilterArray::CanRawFilterArray(const CanRawFilterArray &rhs)
    : d(rhs.d)
{
}

CanRawFilterArray::~CanRawFilterArray()
{
}

void CanRawFilterArray::swap(CanRawFilterArray &other)
{
    qSwap(d, other.d);
}

bool CanRawFilterArray::isEmpty() const
{
    return d->filterArray.isEmpty();
}

int CanRawFilterArray::size() const
{
    return d->filterArray.size();
}

void CanRawFilterArray::resize(int size)
{
    return d->filterArray.resize(size);
}

void CanRawFilterArray::append(const CanRawFilter &filter)
{
    d->filterArray.append(filter);
}

CanRawFilter *CanRawFilterArray::data()
{
    return d->filterArray.data();
}


const CanRawFilter *CanRawFilterArray::data() const
{
    return d->filterArray.constData();
}


const CanRawFilter *CanRawFilterArray::constData() const
{
    return d->filterArray.data();
}

bool CanRawFilterArray::operator ==(const CanRawFilterArray &rhs) const
{
    return d->filterArray == rhs.d->filterArray;
}

CanRawSocket::CanRawSocket(QObject *parent)
    : CanAbstractSocket(RawSocket,
                        *new CanRawSocketPrivate(CAN_RAW_READ_CHUNK_SIZE, CAN_RAW_INITIAL_BUFFER_SIZE),
                        parent)
{
}

CanRawSocket::~CanRawSocket()
{
}

void CanRawSocket::setSocketOption(CanRawSocket::CanRawSocketOption option, const QVariant &value)
{
    Q_D(CanRawSocket);
    d->setSocketOption(option, value);
}

QVariant CanRawSocket::socketOption(CanRawSocket::CanRawSocketOption option)
{
    Q_D(CanRawSocket);
    return d->socketOption(option);
}

void CanRawSocket::setCanFilter(const CanRawFilterArray &filter)
{
    setSocketOption(CanFilterOption, QVariant::fromValue(filter));
}

CanRawFilterArray CanRawSocket::canFilter()
{
    return socketOption(CanFilterOption).value<CanRawFilterArray>();
}

void CanRawSocket::setErrorFilterMask(const CanFrame::CanFrameErrors mask)
{
    setSocketOption(CanRawSocket::ErrorFilterMaskOption, QVariant::fromValue(mask));
}

CanFrame::CanFrameErrors CanRawSocket::errorFilterMask()
{
    return socketOption(CanRawSocket::ErrorFilterMaskOption).value<CanFrame::CanFrameErrors>();
}

void CanRawSocket::setLoopback(Loopback loopback)
{
    setSocketOption(CanRawSocket::LoopbackOption, QVariant::fromValue(loopback));
}

CanRawSocket::Loopback CanRawSocket::loopback()
{
    return socketOption(CanRawSocket::LoopbackOption).value<CanRawSocket::Loopback>();
}

void CanRawSocket::setReceiveOwnMessages(ReceiveOwnMessages ownMessages)
{
    return setSocketOption(CanRawSocket::ReceiveOwnMessagesOption, QVariant::fromValue(ownMessages));
}

CanRawSocket::ReceiveOwnMessages CanRawSocket::receiveOwnMessages()
{
    return socketOption(CanRawSocket::ReceiveOwnMessagesOption).value<CanRawSocket::ReceiveOwnMessages>();
}

void CanRawSocket::setFlexibleDataRateFrames(CanRawSocket::FlexibleDataRateFrames fdFrames)
{
    setSocketOption(CanRawSocket::FlexibleDataRateFramesOption, QVariant::fromValue(fdFrames));
}

CanRawSocket::FlexibleDataRateFrames CanRawSocket::flexibleDataRateFrames()
{
    return socketOption(CanRawSocket::FlexibleDataRateFramesOption).value<CanRawSocket::FlexibleDataRateFrames>();
}

CanRawSocketPrivate::CanRawSocketPrivate(qint32 readChunkSize, qint64 initialBufferSize)
    : CanAbstractSocketPrivate(readChunkSize, initialBufferSize)
    , canFilter(1, CanRawFilter())
    , errorFilterMask(CanFrame::NoError)
    , loopback(CanRawSocket::EnabledLoopback)
    , receiveOwnMessages(CanRawSocket::DisabledOwnMessages)
    , flexibleDataRateFrames(CanRawSocket::DisabledFdFrames)
{
}

CanRawSocketPrivate::~CanRawSocketPrivate()
{
}

bool CanRawSocketPrivate::connectToInterface(const QString &interfaceName)
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

    if (!setSocketOption(CanRawSocket::CanFilterOption, QVariant::fromValue(canFilter))
            || !setSocketOption(CanRawSocket::ErrorFilterMaskOption, QVariant::fromValue(errorFilterMask))
            || !setSocketOption(CanRawSocket::LoopbackOption, QVariant::fromValue(loopback))
            || !setSocketOption(CanRawSocket::ReceiveOwnMessagesOption, QVariant::fromValue(receiveOwnMessages))
            || !setSocketOption(CanRawSocket::FlexibleDataRateFramesOption, QVariant::fromValue(flexibleDataRateFrames))) {
        return false;
    }

    if (::bind(descriptor, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        setError(getSystemError());
        return false;
    }

    return true;
}

bool CanRawSocketPrivate::setSocketOption(CanRawSocket::CanRawSocketOption option, const QVariant &value)
{
    Q_Q(CanRawSocket);

    switch (option) {
    case CanRawSocket::CanFilterOption:
        if (value.canConvert<CanRawFilterArray>()) {
            CanRawFilterArray newCanFilter = value.value<CanRawFilterArray>();
            if (::setsockopt(descriptor,
                             SOL_CAN_RAW,
                             CAN_RAW_FILTER,
                             newCanFilter.data(),
                             newCanFilter.size()*sizeof(CanRawFilter)) == -1 ) {
                setError(getSystemError());
                break;
            }
            if (newCanFilter != canFilter) {
                canFilter = newCanFilter;
                emit q->canFilterChanged();
            }
            return true;
        }
        break;
    case CanRawSocket::ErrorFilterMaskOption:
        if (value.canConvert<CanFrame::CanFrameErrors>()) {
            CanFrame::CanFrameErrors newErrorFilterMask = value.value<CanFrame::CanFrameErrors>();
            if (::setsockopt(descriptor,
                             SOL_CAN_RAW,
                             CAN_RAW_ERR_FILTER,
                             &newErrorFilterMask,
                             sizeof(int)) == -1 ) {
                setError(getSystemError());
                break;
            }
            if (newErrorFilterMask != errorFilterMask) {
                errorFilterMask = newErrorFilterMask;
                emit q->errorFilterMaskChanged();
            }
            return true;
        }
        break;
    case CanRawSocket::LoopbackOption:
        if (value.canConvert<int>()) {
            CanRawSocket::Loopback newLoopback = value.value<CanRawSocket::Loopback>();
            if (::setsockopt(descriptor,
                             SOL_CAN_RAW,
                             CAN_RAW_LOOPBACK,
                             &newLoopback,
                             sizeof(int)) == -1 ) {
                setError(getSystemError());
                break;
            }
            if (newLoopback != loopback) {
                loopback = newLoopback;
                emit q->loopbackChanged();
            }
            return true;
        }
        break;
    case CanRawSocket::ReceiveOwnMessagesOption:
        if (value.canConvert<int>()) {
            CanRawSocket::ReceiveOwnMessages newOwnMessages = value.value<CanRawSocket::ReceiveOwnMessages>();
            if (::setsockopt(descriptor,
                             SOL_CAN_RAW,
                             CAN_RAW_RECV_OWN_MSGS,
                             &newOwnMessages,
                             sizeof(int)) == -1 ) {
                setError(getSystemError());
                break;
            }
            if (newOwnMessages != receiveOwnMessages) {
                receiveOwnMessages = newOwnMessages;
                emit q->receiveOwnMessagesChanged();
            }
            return true;
        }
        break;
    case CanRawSocket::FlexibleDataRateFramesOption:
        if (value.canConvert<int>()) {
            CanRawSocket::FlexibleDataRateFrames newFlexibleDataRateFrames = value.value<CanRawSocket::FlexibleDataRateFrames>();

#ifndef CAN_RAW_FD_FRAMES
            if (newFlexibleDataRateFrames == CanRawSocket::EnabledFdFrames)
                break;

            return true;

#else
            //check if device supports fd frames
            struct ifreq ifr;

            ::strcpy(ifr.ifr_name, interfaceName.toLocal8Bit().constData());
            if (::ioctl(descriptor, SIOCGIFINDEX, &ifr) < 0) {
                setError(getSystemError());
                break;
            }
            if (::ioctl(descriptor, SIOCGIFMTU, &ifr) == -1) {
                if (newFlexibleDataRateFrames == CanRawSocket::EnabledFDFrames) {
                    setError(getSystemError());
                    break;
                }
                else
                    return true;
            }
            else if (ifr.ifr_mtu != CANFD_MTU) {
                if (newFlexibleDataRateFrames == CanRawSocket::EnabledFDFrames) {
                    setError(CanAbstractSocketErrorInfo(CanAbstractSocket::UnsupportedSocketOperationError, CanRawSocket::tr("Device doesn't support flexible data rate frames")));
                    break;
                }
            }

            if (::setsockopt(descriptor,
                             SOL_CAN_RAW,
                             CAN_RAW_FD_FRAMES,
                             &newFlexibleDataRateFrames,
                             sizeof(int)) == -1 ) {
                setError(getSystemError());
                break;
            }
            if (newFlexibleDataRateFrames != flexibleDataRateFrames) {
                flexibleDataRateFrames = newFlexibleDataRateFrames;
                emit q->flexibleDataRateFramesChanged();
            }
            return true;
#endif
        }
        break;

    }

    return false;

}

QVariant CanRawSocketPrivate::socketOption(CanRawSocket::CanRawSocketOption option)
{
    QVariant result;

    switch (option) {
    case CanRawSocket::CanFilterOption:
        result.setValue(canFilter);
        break;
    case CanRawSocket::ErrorFilterMaskOption:
        result.setValue(errorFilterMask);
        break;
    case CanRawSocket::LoopbackOption:
        result.setValue(loopback);
        break;
    case CanRawSocket::ReceiveOwnMessagesOption:
        result.setValue(receiveOwnMessages);
        break;
    case CanRawSocket::FlexibleDataRateFramesOption:
        result.setValue(flexibleDataRateFrames);
        break;
    }

    return result;
}

qint64 CanRawSocketPrivate::readFromSocket(char *data, qint64 maxSize)
{
    size_t frameSize = CAN_MTU;
#ifdef CANFD_MTU
    if (flexibleDataRateFrames == CanRawSocket::EnabledFdFrames)
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
            data[RES0_BYTE] = res0FromCanMtu(CAN_MTU);
            data[RES1_BYTE] = res1FromCanMtu(CAN_MTU);
        }
#ifdef CANFD_MTU
        else if (ret == CANFD_MTU && static_cast<int>(frameSize) == ret) {
            // received canfd frame in canfd mode
            data[RES0_BYTE] = res0FromCanMtu(CANFD_MTU);
            data[RES1_BYTE] = res1FromCanMtu(CANFD_MTU);
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
    size_t frameSize = CAN_MTU;
#ifdef CANFD_MTU
    if (flexibleDataRateFrames == CanRawSocket::EnabledFdFrames)
        frameSize = CANFD_MTU;
#endif
    size_t bytesToWrite;
    quint8 res0;
    quint8 res1;

    qint64 writtenBytes = 0;
    int ret;

    forever {

        //get reserved bytes that define frame type (can or canfd)
        res0 = data[RES0_BYTE];
        res1 = data[RES1_BYTE];

        if (maxSize - writtenBytes < static_cast<qint64>(frameSize))  {
            //leftof size smaller then frame size
            break;
        }

        if (res0 == res0FromCanMtu(CAN_MTU)
                && res1 == res1FromCanMtu(CAN_MTU)) {
            //standard can frame can be written in can and in canfd mode
            bytesToWrite = CAN_MTU;
        }
#ifdef CANFD_MTU
        else if (res0 == res0FromCanMtu(CANFD_MTU)
                 && res1 == res1FromCanMtu(CANFD_MTU)
                 && flexibleDataRateFrames == CanRawSocket::EnabledFdFrames) {
            //fd frame can only be written in fd mode
            bytesToWrite = CANFD_MTU;
        }
#endif
        else {
            //data for reserved bytes is incorrect
            return -1;
        }

        ret = ::write(descriptor, data, bytesToWrite);

        if (ret == 0) {
            break;
        }
        else if (ret < 0) {
            if (errno == ENOBUFS)
                break;
            return -1;
        }
        else if (ret != static_cast<int>(frameSize)) {
            return -1;
        }

        data += ret;
        writtenBytes += ret;
    }
    return writtenBytes;
}

#include "moc_canrawsocket.cpp"
