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

#include "canframe.h"
#include "canframe_p.h"

#include <qdebug.h>
#include <qdatastream.h>

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &stream, const CanFrame &frame)
{
    if (!frame.isValid())
        return stream;

    stream << frame.d->id
           << frame.d->dlen
           << frame.d->flags
           << frame.d->res0
           << frame.d->res1;

    for (int i = 0; i < frame.d->data.length(); ++i) {
        stream << (qint8)frame.d->data[i];
    }

    return stream;
}

QDataStream &operator>>(QDataStream &stream, CanFrame &frame)
{
    stream >> frame.d->id
            >> frame.d->dlen
            >> frame.d->flags
            >> frame.d->res0
            >> frame.d->res1;

    int dataLength = dataLengthFromResBytes(frame.d->res0, frame.d->res1);

    if (dataLength) {
        frame.d->data = QVector<char>(dataLength, '0');
        qint8 idata;
        for (int i = 0; i < dataLength; ++i) {
            stream >> idata;
            frame.d->data[i] = idata;
        }
    }
    else {
        frame.setCanId(static_cast<uint>(CanFrame::UnknownCanFrameError));
        frame.toErrorFrame();
    }

    return stream;
}

#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const CanFrame &frame)
{
    dbg.noquote();
    dbg << QString::number(frame.canId(), 16)
        << frame.dataLength()
        << QByteArray(frame.constData(), frame.dataLength()).toHex();

    if (frame.isErrorFrame())
        dbg << QByteArray("ERRORFRAME");
    else if (frame.isRtrFrame())
        dbg << QByteArray("RTRFRAME");

    return dbg;
}

#endif //QT_NO_DEBUG_STREAM


CanFrame::CanFrame()
    : d(new CanFrameData)
{
}

CanFrame::CanFrame(CanFrame::CanFrameType type)
    : d(new CanFrameData)
{
    setFrameType(type);
}


CanFrame::CanFrame(const CanFrame &rhs)
    : d(rhs.d)
{
}

CanFrame::~CanFrame()
{
}

void CanFrame::swap(CanFrame &other)
{
    qSwap(d, other.d);
}

bool CanFrame::isValid() const
{
    return frameType() != UnknownFrame;
}

bool CanFrame::isEmpty() const
{
    return d->dlen == 0;
}

bool CanFrame::isNull() const
{
    return d->data.length() == 0;
}

void CanFrame::clear()
{
    d->clear();
}

int CanFrame::maxDataLength() const
{
    if (isDataFrame())
        return CAN_MAX_DLEN;
    else if (isFdFrame())
        return CANFD_MAX_DLEN;
    else if (isErrorFrame())
        return CAN_MAX_DLEN;
    else if (isRtrFrame())
        return 0;
    else
        return -1;
}

int CanFrame::maxDataTransferUnit() const
{
    if (isDataFrame())
        return CAN_MTU;
    else if (isFdFrame())
        return CANFD_MTU;
    else if (isErrorFrame())
        return CAN_MTU;
    else if (isRtrFrame())
        return CAN_MTU;
    else
        return -1;
}

void CanFrame::setFrameType(CanFrameType frameType)
{
    switch (frameType) {
    case CanFrame::DataFrame:
        d->toDataFrame();
        break;
    case CanFrame::FdFrame:
        d->toFdFrame();
        break;
    case CanFrame::ErrorFrame:
        d->toErrorFrame();
        break;
    case CanFrame::RtrFrame:
        d->toRtrFrame();
        break;
    default:
        clear();
    }
}

CanFrame::CanFrameType CanFrame::frameType() const
{
    if (isDataFrame())
        return CanFrame::DataFrame;
    else if (isFdFrame())
        return CanFrame::FdFrame;
    else if (isErrorFrame())
        return CanFrame::ErrorFrame;
    else if (isRtrFrame())
        return CanFrame::RtrFrame;
    else
        return CanFrame::UnknownFrame;
}

bool CanFrame::isDataFrame() const
{
    return d->isDataFrame();
}

bool CanFrame::isFdFrame() const
{
    return d->isFdFrame();
}

bool CanFrame::isErrorFrame() const
{
    return d->isErrorFrame();
}

bool CanFrame::isRtrFrame() const
{
    return d->isRtrFrame();
}

void CanFrame::toDataFrame()
{
    d->toDataFrame();
}

void CanFrame::toFdFrame()
{
    d->toFdFrame();
}

void CanFrame::toErrorFrame()
{
    d->toErrorFrame();
}

void CanFrame::toRtrFrame()
{
    d->toRtrFrame();
}

bool CanFrame::operator ==(const CanFrame &rhs) const
{
    return d == rhs.d;
}

bool CanFrame::operator >(const CanFrame &rhs) const
{
    return (canId() & CAN_EFF_MASK) > (rhs.canId() & CAN_EFF_MASK);
}

bool CanFrame::operator <(const CanFrame &rhs) const
{
    return (canId() & CAN_EFF_MASK) < (rhs.canId() & CAN_EFF_MASK);
}

uint CanFrame::canId() const
{
    return d->id & CAN_EFF_MASK;
}

void CanFrame::setCanId(uint id)
{
    if ((d->id & CAN_EFF_MASK) != (id & CAN_EFF_MASK)) {
        d->id &= ~CAN_EFF_MASK;
        d->id |= id & CAN_EFF_MASK;
    }
}

void CanFrame::setId(uint id)
{
    d->id = id;
}

uint CanFrame::id() const
{
    return d->id;
}

void CanFrame::setFrameFormat(CanFrameFormat format)
{
    if (format == ExtendedFrameFormat && d->effFlag() == false)
        d->setEffFlag(true);
    else if (format == StandardFrameFormat && d->effFlag() == true)
        d->setEffFlag(false);
}


CanFrame::CanFrameFormat CanFrame::frameFormat() const
{
    if (d->effFlag())
        return CanFrame::ExtendedFrameFormat;

    return CanFrame::StandardFrameFormat;
}

bool CanFrame::setDataLength(int bytes)
{
    if (!isValid())
        return false;
    else if (dataLength() == bytes)
        return true;
    else if (isRtrFrame() || isErrorFrame())
        return false;
    else if (bytes >= 0 && bytes <= maxDataLength()) {
        d->dlen = bytes;
        return true;
    }
    else
        return false;
}

int CanFrame::dataLength() const
{
    return d->dlen;
}

void CanFrame::setData(const char *data, int len)
{
    if (len == -1 || len > maxDataLength())
        len = maxDataLength();

    for (int i = 0; i < len; ++i) {
        d->data[i] = data[i];
    }
}


char* CanFrame::data()
{    
    return d->data.data();
}

const char* CanFrame::data() const
{   
    return d->data.data();
}

const char &CanFrame::at(int i) const
{
    return d->data.at(i);
}

const char &CanFrame::operator [](int i) const
{
    return d->data[i];
}

char &CanFrame::operator [](int i)
{
    return d->data[i];
}

CanFrame::CanFrameErrors CanFrame::error() const
{
    CanFrameErrors errors = NoError;

    if (!isErrorFrame())
        return errors;

    if (canId() & CAN_ERR_TX_TIMEOUT)
        errors |= TXTimeoutError;
    if (canId() & CAN_ERR_LOSTARB)
        errors |= LostArbitrationError;
    if (canId() & CAN_ERR_CRTL)
        errors |= ControllerProblemsError;
    if (canId() & CAN_ERR_PROT)
        errors |= ProtocolViolationsError;
    if (canId() & CAN_ERR_TRX)
        errors |= TransceiverStatusError;
    if (canId() & CAN_ERR_ACK)
        errors |= NoACKOnTransmissionError;
    if (canId() & CAN_ERR_BUSOFF)
        errors |= BusOffError;
    if (canId() & CAN_ERR_BUSERROR)
        errors |= BusError;
    if (canId() & CAN_ERR_RESTARTED)
        errors |= ControllerRestartedError;

    return errors;
}
