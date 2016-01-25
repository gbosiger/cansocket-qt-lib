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

#include "canisotpsocket.h"
#include "canabstractsocket.h"
#include "canabstractsocket_p.h"
#include "canisotpsocket_p.h"

#include <private/qcore_unix_p.h>

#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/isotp.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define CAN_ISOTP_READ_CHUNK_SIZE 4096 // max. data
#define CAN_ISOTP_INITIAL_BUFFER_SIZE 16384 // 4x max.

struct CanIsoTpOptionsPrivate {
    CanIsoTpOptionsPrivate()
        : flags(CAN_ISOTP_DEFAULT_FLAGS)
        , frameTxTime(CAN_ISOTP_DEFAULT_FRAME_TXTIME)
        , extAddress(CAN_ISOTP_DEFAULT_EXT_ADDRESS)
        , txPadContent(CAN_ISOTP_DEFAULT_PAD_CONTENT)
        , rxPadContent(CAN_ISOTP_DEFAULT_PAD_CONTENT)
        , rxExtAddress(CAN_ISOTP_DEFAULT_EXT_ADDRESS)
    {
    }

    CanIsoTpOptionsPrivate(const CanIsoTpOptionsPrivate &rhs)
        : flags(rhs.flags)
        , frameTxTime(rhs.frameTxTime)
        , extAddress(rhs.extAddress)
        , txPadContent(rhs.txPadContent)
        , rxPadContent(rhs.rxPadContent)
        , rxExtAddress(rhs.rxExtAddress)
    {
    }

    inline bool operator ==(const CanIsoTpOptionsPrivate &rhs) const
    {
        return (flags == rhs.flags
                && frameTxTime == rhs.frameTxTime
                && extAddress == rhs.extAddress
                && txPadContent == rhs.txPadContent
                && rxPadContent == rhs.rxPadContent
                && rxExtAddress == rhs.rxExtAddress);
    }

    quint32 flags;
    quint32 frameTxTime;
    quint8 extAddress;
    quint8 txPadContent;
    quint8 rxPadContent;
    quint8 rxExtAddress;
};

struct CanIsoTpFlowControlOptionsPrivate {
    CanIsoTpFlowControlOptionsPrivate()
        : blockSize(CAN_ISOTP_DEFAULT_RECV_BS)
        , stMin(CAN_ISOTP_DEFAULT_RECV_STMIN)
        , wftMax(CAN_ISOTP_DEFAULT_RECV_WFTMAX)
    {
    }

    CanIsoTpFlowControlOptionsPrivate(const CanIsoTpFlowControlOptionsPrivate &rhs)
        : blockSize(rhs.blockSize)
        , stMin(rhs.stMin)
        , wftMax(rhs.wftMax)
    {
    }

    inline bool operator ==(const CanIsoTpFlowControlOptionsPrivate &rhs) const
    {
        return (blockSize == rhs.blockSize
                && stMin == rhs.stMin
                && wftMax == rhs.wftMax);
    }

    quint8 blockSize;
    quint8 stMin;
    quint8 wftMax;
};

struct CanIsoTpLinkLayerOptionsPrivate {
    CanIsoTpLinkLayerOptionsPrivate()
        : mtu(CAN_ISOTP_DEFAULT_LL_MTU)
        , txDlen(CAN_ISOTP_DEFAULT_LL_TX_DL)
        , txFdFlags(CAN_ISOTP_DEFAULT_LL_TX_FLAGS)
    {
    }

    CanIsoTpLinkLayerOptionsPrivate(const CanIsoTpLinkLayerOptionsPrivate &rhs)
        : mtu(rhs.mtu)
        , txDlen(rhs.txDlen)
        , txFdFlags(rhs.txFdFlags)
    {
    }

    inline bool operator ==(const CanIsoTpLinkLayerOptionsPrivate &rhs) const {
        return (mtu == rhs.mtu
                && txDlen == rhs.txDlen
                && txFdFlags == rhs.txFdFlags);
    }

    quint8 mtu;
    quint8 txDlen;
    quint8 txFdFlags;
};


CanIsoTpOptions::CanIsoTpOptions()
    : d(new CanIsoTpOptionsPrivate)
{
}

CanIsoTpOptions::CanIsoTpOptions(const CanIsoTpOptions &rhs)
    : d(new CanIsoTpOptionsPrivate(*rhs.d))
{
}

CanIsoTpOptions::~CanIsoTpOptions()
{
    delete d;
    d = Q_NULLPTR;
}

void CanIsoTpOptions::setIsoTpFlags(IsoTpFlags flags)
{
    if (d->flags != static_cast<quint32>(flags))
        d->flags = flags;
}

CanIsoTpOptions::IsoTpFlags CanIsoTpOptions::isoTpFlags() const
{
    return IsoTpFlags(d->flags);
}

void CanIsoTpOptions::setFrameTxTime(quint32 nsecs)
{
    if (d->frameTxTime != nsecs)
        d->frameTxTime = nsecs;
}

quint32 CanIsoTpOptions::frameTxTime() const
{
    return d->frameTxTime;
}

void CanIsoTpOptions::setExtendedAddress(quint8 address)
{
    if (d->extAddress != address)
        d->extAddress = address;
}

quint8 CanIsoTpOptions::extendedAddres() const
{
    return d->extAddress;
}

void CanIsoTpOptions::setTxPaddingByte(quint8 content)
{
    if (d->txPadContent != content)
        d->txPadContent = content;
}

quint8 CanIsoTpOptions::txPaddingByte() const
{
    return d->txPadContent;
}

void CanIsoTpOptions::setRxPaddingByte(quint8 content)
{
    if (d->rxPadContent != content)
        d->rxPadContent = content;
}

quint8 CanIsoTpOptions::rxPaddingByte() const
{
    return d->rxPadContent;
}

bool CanIsoTpOptions::operator ==(const CanIsoTpOptions &rhs) const
{
    return this->d == rhs.d;
}

CanIsoTpFlowControlOptions::CanIsoTpFlowControlOptions()
    : d(new CanIsoTpFlowControlOptionsPrivate)
{
}

CanIsoTpFlowControlOptions::CanIsoTpFlowControlOptions(const CanIsoTpFlowControlOptions &rhs)
    : d(new CanIsoTpFlowControlOptionsPrivate(*rhs.d))
{
}

CanIsoTpFlowControlOptions::~CanIsoTpFlowControlOptions()
{
    delete d;
    d = Q_NULLPTR;
}

void CanIsoTpFlowControlOptions::setBlockSize(quint8 size)
{
    if (d->blockSize != size)
        d->blockSize = size;
}

quint8 CanIsoTpFlowControlOptions::blockSize() const
{
    return d->blockSize;
}

void CanIsoTpFlowControlOptions::setMinSeparationTime(quint8 sepTimeCode)
{
    if (d->stMin != sepTimeCode)
        d->stMin = sepTimeCode;
}

quint8 CanIsoTpFlowControlOptions::minSeparationTime() const
{
    return d->stMin;
}

void CanIsoTpFlowControlOptions::setWaitFramesMaxNumber(quint8 waitFramesMaxNumber)
{
    if (d->wftMax != waitFramesMaxNumber)
        d->wftMax = waitFramesMaxNumber;
}

quint8 CanIsoTpFlowControlOptions::waitFramesMaxNumber() const
{
    return d->wftMax;
}

bool CanIsoTpFlowControlOptions::operator ==(const CanIsoTpFlowControlOptions &rhs) const
{
    return this->d == rhs.d;
}


CanIsoTpLinkLayerOptions::CanIsoTpLinkLayerOptions()
    : d(new CanIsoTpLinkLayerOptionsPrivate)
{
}

CanIsoTpLinkLayerOptions::CanIsoTpLinkLayerOptions(const CanIsoTpLinkLayerOptions &rhs)
    : d(new CanIsoTpLinkLayerOptionsPrivate(*rhs.d))
{
}

CanIsoTpLinkLayerOptions::~CanIsoTpLinkLayerOptions()
{
    delete d;
    d = Q_NULLPTR;
}

void CanIsoTpLinkLayerOptions::setMaxDataTransferUnit(MtuOption mtu)
{
    if (d->mtu != mtu)
        d->mtu = mtu;
}

CanIsoTpLinkLayerOptions::MtuOption CanIsoTpLinkLayerOptions::maxDataTransferUnit() const
{
    return MtuOption(d->mtu);
}

void CanIsoTpLinkLayerOptions::setTxDataLength(TxDataLengthOption dlen)
{
    if (d->txDlen != dlen)
        d->txDlen = dlen;
}

CanIsoTpLinkLayerOptions::TxDataLengthOption CanIsoTpLinkLayerOptions::txDataLength() const
{
    return TxDataLengthOption(d->txDlen);
}

void CanIsoTpLinkLayerOptions::setTxFdFrameFlags(CanFrame::CanFdFrameFlags flags)
{
    if (d->txFdFlags != flags)
        d->txFdFlags = flags;
}

CanFrame::CanFdFrameFlags CanIsoTpLinkLayerOptions::txFdFrameFlags() const
{
    return CanFrame::CanFdFrameFlags(d->txFdFlags);
}

bool CanIsoTpLinkLayerOptions::operator ==(const CanIsoTpLinkLayerOptions &rhs) const
{
    return this->d == rhs.d;
}


CanIsoTpSocket::CanIsoTpSocket(QObject *parent)
    : CanAbstractSocket(IsoTpSocket,
                        *new CanIsoTpSocketPrivate(CAN_ISOTP_READ_CHUNK_SIZE, CAN_ISOTP_INITIAL_BUFFER_SIZE),
                        parent)
{
}

CanIsoTpSocket::~CanIsoTpSocket()
{
}

bool CanIsoTpSocket::connectToInterface(const QString &interfaceName, OpenMode mode)
{
    return CanAbstractSocket::connectToInterface(interfaceName, mode);
}

bool CanIsoTpSocket::connectToInterface(const QString &interfaceName, uint txId, uint rxId, OpenMode mode)
{
    if (!setTxId(txId))
        return false;

    if (!setRxId(rxId))
        return false;

    return CanAbstractSocket::connectToInterface(interfaceName, mode);
}

void CanIsoTpSocket::setSocketOption(CanIsoTpSocketOption option, const QVariant &value)
{
    Q_D(CanIsoTpSocket);
    d->setSocketOption(option, value);
}

QVariant CanIsoTpSocket::socketOption(CanIsoTpSocketOption option)
{
    Q_D(CanIsoTpSocket);
    return d->socketOption(option);
}

bool CanIsoTpSocket::setTxId(uint id)
{
    if (socketState() != CanIsoTpSocket::UnconnectedState) {
        setSocketError(CanAbstractSocket::UnsupportedSocketOperationError, CanIsoTpSocket::tr("Cannot set Tx ID in state other than unconnected"));
        return false;
    }

    setSocketOption(CanIsoTpSocket::TxIdOption, QVariant::fromValue(id));
    return true;
}

uint CanIsoTpSocket::txId()
{
    return socketOption(CanIsoTpSocket::TxIdOption).value<uint>();
}

bool CanIsoTpSocket::setRxId(uint id)
{
    if (socketState() != CanIsoTpSocket::UnconnectedState) {
        setSocketError(CanAbstractSocket::UnsupportedSocketOperationError, CanIsoTpSocket::tr("Cannot set Rx ID in state other than unconnected"));
        return false;
    }

    setSocketOption(CanIsoTpSocket::RxIdOption, QVariant::fromValue(id));
    return true;
}

uint CanIsoTpSocket::rxId()
{
    return socketOption(CanIsoTpSocket::RxIdOption).value<uint>();
}

void CanIsoTpSocket::setIsoTpOptions(const CanIsoTpOptions &options)
{
    setSocketOption(CanIsoTpSocket::IsoTpOptions, QVariant::fromValue(options));
}

CanIsoTpOptions CanIsoTpSocket::isoTpOptions()
{
    return socketOption(CanIsoTpSocket::IsoTpOptions).value<CanIsoTpOptions>();
}

void CanIsoTpSocket::setFlowControlOptions(const CanIsoTpFlowControlOptions &options)
{
    setSocketOption(CanIsoTpSocket::FlowControlOptions, QVariant::fromValue(options));
}

CanIsoTpFlowControlOptions CanIsoTpSocket::flowControlOptions()
{
    return socketOption(CanIsoTpSocket::FlowControlOptions).value<CanIsoTpFlowControlOptions>();
}

void CanIsoTpSocket::setTxMinSepTime(uint nsecs)
{
    setSocketOption(CanIsoTpSocket::TxMinSepTimeOption, QVariant::fromValue(nsecs));
}

uint CanIsoTpSocket::txMinSepTime()
{
    return socketOption(CanIsoTpSocket::TxMinSepTimeOption).value<uint>();
}

void CanIsoTpSocket::setRxMinSepTime(uint nsecs)
{
    setSocketOption(CanIsoTpSocket::RxMinSepTimeOption, QVariant::fromValue(nsecs));
}

uint CanIsoTpSocket::rxMinSepTime()
{
    return socketOption(CanIsoTpSocket::RxMinSepTimeOption).value<uint>();
}

void CanIsoTpSocket::setLinkLayerOptions(const CanIsoTpLinkLayerOptions &options)
{
    setSocketOption(CanIsoTpSocket::LinkLayerOptions, QVariant::fromValue(options));
}

CanIsoTpLinkLayerOptions CanIsoTpSocket::linkLayerOptions()
{
    return socketOption(CanIsoTpSocket::LinkLayerOptions).value<CanIsoTpLinkLayerOptions>();
}

CanIsoTpSocketPrivate::CanIsoTpSocketPrivate(qint32 readChunkSize, qint64 initialBufferSize)
    : CanAbstractSocketPrivate(readChunkSize, initialBufferSize)
    , txId(0)
    , rxId(0)
    , isoTpOptions()
    , flowControlOptions()
    , txMinSepTime(0)
    , rxMinSepTime(0)
    , linkLayerOptions()
{
}

CanIsoTpSocketPrivate::~CanIsoTpSocketPrivate()
{
}

bool CanIsoTpSocketPrivate::connectToInterface(const QString &interfaceName)
{
    struct ifreq ifr;
    struct sockaddr_can addr;

    addr.can_addr.tp.tx_id = txId;
    addr.can_addr.tp.rx_id = rxId;

    descriptor = ::socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);

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

    if (!setSocketOption(CanIsoTpSocket::IsoTpOptions, QVariant::fromValue(isoTpOptions))
            || !setSocketOption(CanIsoTpSocket::FlowControlOptions, QVariant::fromValue(flowControlOptions))
            || !setSocketOption(CanIsoTpSocket::TxMinSepTimeOption, QVariant::fromValue(0))
            || !setSocketOption(CanIsoTpSocket::RxMinSepTimeOption, QVariant::fromValue(0))
            || !setSocketOption(CanIsoTpSocket::LinkLayerOptions, QVariant::fromValue(linkLayerOptions))) {
        return false;
    }

    if (::bind(descriptor, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        setError(getSystemError());
        return false;
    }

    return true;
}

bool CanIsoTpSocketPrivate::setSocketOption(CanIsoTpSocket::CanIsoTpSocketOption option, const QVariant &value)
{
    Q_Q(CanIsoTpSocket);

    switch (option) {
    case CanIsoTpSocket::TxIdOption:
        if (value.canConvert<quint32>()) {
            quint32 newTxId = value.value<quint32>();
            if (newTxId != txId) {
                txId = newTxId;
                emit q->txIdChanged();
            }
            return true;
        }
        break;
    case CanIsoTpSocket::RxIdOption:
        if (value.canConvert<quint32>()) {
            quint32 newRxId = value.value<quint32>();
            if (newRxId != rxId) {
                rxId = newRxId;
                emit q->rxIdChanged();
            }
            return true;
        }
        break;
    case CanIsoTpSocket::IsoTpOptions:
        if (value.canConvert<CanIsoTpOptions>()) {
            CanIsoTpOptions newIsoTpOptions = value.value<CanIsoTpOptions>();
            if (::setsockopt(descriptor,
                             SOL_CAN_ISOTP,
                             CAN_ISOTP_OPTS,
                             newIsoTpOptions.d,
                             sizeof(*newIsoTpOptions.d)) == -1) {
                setError(getSystemError());
                break;
            }
            if (newIsoTpOptions != isoTpOptions) {
                isoTpOptions = newIsoTpOptions;
                emit q->isoTpOptionsChanged();
            }
            return true;
        }
        break;
    case CanIsoTpSocket::FlowControlOptions:
        if (value.canConvert<CanIsoTpFlowControlOptions>()) {
            CanIsoTpFlowControlOptions newFlowControlOptions = value.value<CanIsoTpFlowControlOptions>();
            if (::setsockopt(descriptor,
                             SOL_CAN_ISOTP,
                             CAN_ISOTP_RECV_FC,
                             newFlowControlOptions.d,
                             sizeof(*newFlowControlOptions.d)) == -1) {
                setError(getSystemError());
                break;
            }
            if (newFlowControlOptions != flowControlOptions) {
                flowControlOptions = newFlowControlOptions;
                emit q->flowControlOptionsChanged();
            }
            return true;
        }
        break;
    case CanIsoTpSocket::TxMinSepTimeOption:
        if (value.canConvert<quint32>()) {
            quint32 newTxMinSepTime = value.value<quint32>();
            if (::setsockopt(descriptor,
                             SOL_CAN_ISOTP,
                             CAN_ISOTP_TX_STMIN,
                             &newTxMinSepTime,
                             sizeof(newTxMinSepTime)) == -1) {
                setError(getSystemError());
                break;
            }
            if (newTxMinSepTime != txMinSepTime) {
                txMinSepTime = newTxMinSepTime;
                emit q->txMinSepTimeChanged();
            }
            return true;
        }
        break;
    case CanIsoTpSocket::RxMinSepTimeOption:
        if (value.canConvert<quint32>()) {
            quint32 newRxMinSepTime = value.value<quint32>();
            if (::setsockopt(descriptor,
                             SOL_CAN_ISOTP,
                             CAN_ISOTP_TX_STMIN,
                             &newRxMinSepTime,
                             sizeof(newRxMinSepTime)) == -1) {
                setError(getSystemError());
                break;
            }
            if (newRxMinSepTime != rxMinSepTime) {
                rxMinSepTime = newRxMinSepTime;
                emit q->rxMinSepTimeChanged();
            }
            return true;
        }
        break;
    case CanIsoTpSocket::LinkLayerOptions:
        if (value.canConvert<CanIsoTpLinkLayerOptions>()) {
            CanIsoTpLinkLayerOptions newLinkLayerOptions = value.value<CanIsoTpLinkLayerOptions>();
            if (::setsockopt(descriptor,
                             SOL_CAN_ISOTP,
                             CAN_ISOTP_LL_OPTS,
                             newLinkLayerOptions.d,
                             sizeof(*newLinkLayerOptions.d)) == -1) {
                setError(getSystemError());
                break;
            }
            if (newLinkLayerOptions != linkLayerOptions) {
                linkLayerOptions = newLinkLayerOptions;
                emit q->linkLayerOptionsChanged();
            }
            return true;
        }
        break;
    }

    return false;
}

QVariant CanIsoTpSocketPrivate::socketOption(CanIsoTpSocket::CanIsoTpSocketOption option)
{
    QVariant result;

    switch (option) {
    case CanIsoTpSocket::TxIdOption:
        result.setValue(txId);
        break;
    case CanIsoTpSocket::RxIdOption:
        result.setValue(rxId);
        break;
    case CanIsoTpSocket::IsoTpOptions:
        result.setValue(isoTpOptions);
        break;
    case CanIsoTpSocket::FlowControlOptions:
        result.setValue(flowControlOptions);
        break;
    case CanIsoTpSocket::TxMinSepTimeOption:
        result.setValue(txMinSepTime);
        break;
    case CanIsoTpSocket::RxMinSepTimeOption:
        result.setValue(rxMinSepTime);
        break;
    case CanIsoTpSocket::LinkLayerOptions:
        result.setValue(linkLayerOptions);
        break;
    }

    return result;
}

qint64 CanIsoTpSocketPrivate::readFromSocket(char *data, qint64 maxSize)
{
    return qt_safe_read(descriptor, data, maxSize);
}

qint64 CanIsoTpSocketPrivate::writeToSocket(const char *data, qint64 maxSize)
{
    return qt_safe_write(descriptor, data, maxSize);
}

#include "moc_canisotpsocket.cpp"
