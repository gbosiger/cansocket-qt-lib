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

#ifndef CANRAWSOCKET_H
#define CANRAWSOCKET_H

#include <CanSocket/canabstractsocket.h>
#include <CanSocket/canframe.h>
#include <QtCore/qvariant.h>
#include <QtCore/qshareddata.h>

class CanRawSocketPrivate;
class CanRawFilterArrayData;

typedef CanFrame::CanFrameErrors CanFrameErrors;

class CANSOCKET_EXPORT CanRawFilter {

public:
    typedef CanFrame::CanFrameFormat CanFrameFormat;

    CanRawFilter(quint32 fullId = 0, quint32 fullIdMask = 0)
        : m_fullId(fullId)
        , m_fullIdMask(fullIdMask)
    {
    }

    CanRawFilter(const CanRawFilter &rhs)
        : m_fullId(rhs.fullId())
        , m_fullIdMask(rhs.fullIdMask())
    {
    }

    inline void setFilter(quint32 fullId, quint32 fullIdMask) {
        m_fullId = fullId;
        m_fullIdMask = fullIdMask;
    }

    inline quint32 fullId() const { return m_fullId; }
    inline void setFullId(quint32 id) { m_fullId = id; }

    inline quint32 fullIdMask() const { return m_fullIdMask; }
    inline void setFullIdMask(quint32 mask) { m_fullIdMask = mask; }

    inline bool operator ==(const CanRawFilter &rhs) const
    {
        return (m_fullId == rhs.m_fullId) && (m_fullIdMask == rhs.m_fullIdMask);
    }

    inline bool operator !=(const CanRawFilter &rhs) const { return !operator==(rhs); }

private:
    quint32 m_fullId;
    quint32 m_fullIdMask;
};
Q_DECLARE_METATYPE(CanRawFilter)

class CANSOCKET_EXPORT CanRawFilterArray {

public:
    CanRawFilterArray();
    CanRawFilterArray(int size);
    CanRawFilterArray(int size, const CanRawFilter &filter);
    CanRawFilterArray(const CanRawFilterArray &rhs);
    ~CanRawFilterArray();

    void swap(CanRawFilterArray &);

    bool isEmpty() const;
    int size() const;
    void resize(int size);

    void append(const CanRawFilter &);
//    void append(const CanRawFilterArray &);

//    void remove(int i);
//    void remove(const CanFrame &);

//    void clear();
//    int count() const;

    CanRawFilter *data();
    const CanRawFilter *data() const;
    const CanRawFilter *constData() const;

    bool operator ==(const CanRawFilterArray &rhs) const;
    inline bool operator !=(const CanRawFilterArray &rhs) const { return !operator==(rhs); }

//    const CanRawFilter &at(int i) const;

//    CanRawFilter &operator<<(const T &value);

//    CanRawFilter &operator[](int i);
//    const CanRawFilter &operator[](int i) const;

private:
    QSharedDataPointer<CanRawFilterArrayData> d;

    friend class CanRawSocketPrivate;
};

Q_DECLARE_SHARED(CanRawFilterArray)

Q_DECLARE_METATYPE(CanRawFilterArray)

class CANSOCKET_EXPORT CanRawSocket : public CanAbstractSocket
{
    Q_OBJECT

    Q_PROPERTY(CanRawFilterArray canFilter READ canFilter WRITE setCanFilter NOTIFY canFilterChanged)
    Q_PROPERTY(CanFrameErrors errorFilterMask READ errorFilterMask WRITE setErrorFilterMask NOTIFY errorFilterMaskChanged)
    Q_PROPERTY(Loopback loopback READ loopback WRITE setLoopback NOTIFY loopbackChanged)
    Q_PROPERTY(ReceiveOwnMessages receiveOwnMessages READ receiveOwnMessages WRITE setReceiveOwnMessages NOTIFY receiveOwnMessagesChanged)
    Q_PROPERTY(FlexibleDataRateFrames flexibleDataRateFrames READ flexibleDataRateFrames WRITE setFlexibleDataRateFrames NOTIFY flexibleDataRateFramesChanged)

public:
    enum CanRawSocketOption {
        CanFilterOption,
        ErrorFilterMaskOption,
        LoopbackOption,
        ReceiveOwnMessagesOption,
        FlexibleDataRateFramesOption
    };
    Q_ENUM(CanRawSocketOption)

    enum Loopback {
        DisabledLoopback = 0,
        EnabledLoopback = 1,

        UndefinedLoopback = -1
    };
    Q_ENUM(Loopback)

    enum ReceiveOwnMessages {
        DisabledOwnMessages = 0,
        EnabledOwnMessages = 1,

        UndefinedOwnMessages = -1
    };
    Q_ENUM(ReceiveOwnMessages)

    enum FlexibleDataRateFrames {
        DisabledFDFrames = 0,
        EnabledFDFrames = 1,

        UndefinedFDFrames = -1
    };
    Q_ENUM(FlexibleDataRateFrames)

    explicit CanRawSocket(QObject *parent = Q_NULLPTR);
    virtual ~CanRawSocket();

    void setSocketOption(CanRawSocketOption option, const QVariant &value);
    QVariant socketOption(CanRawSocketOption option);

    inline void setCanFilter(const CanRawFilterArray &filter) { return setSocketOption(CanFilterOption, QVariant::fromValue(filter)); }
    inline CanRawFilterArray canFilter() { return socketOption(CanFilterOption).value<CanRawFilterArray>(); }

    inline void setErrorFilterMask(const CanFrameErrors mask) { return setSocketOption(ErrorFilterMaskOption, QVariant::fromValue(mask)); }
    inline CanFrameErrors errorFilterMask() { return socketOption(ErrorFilterMaskOption).value<CanFrameErrors>(); }

    inline void setLoopback(Loopback loopback) { return setSocketOption(LoopbackOption, QVariant::fromValue(loopback)); }
    inline Loopback loopback() { return socketOption(LoopbackOption).value<Loopback>(); }

    inline void setReceiveOwnMessages(ReceiveOwnMessages ownMessages) { return setSocketOption(ReceiveOwnMessagesOption, QVariant::fromValue(ownMessages)); }
    inline ReceiveOwnMessages receiveOwnMessages() { return socketOption(ReceiveOwnMessagesOption).value<ReceiveOwnMessages>(); }

    inline void setFlexibleDataRateFrames(FlexibleDataRateFrames fdFrames) { return setSocketOption(FlexibleDataRateFramesOption, QVariant::fromValue(fdFrames)); }
    inline FlexibleDataRateFrames flexibleDataRateFrames() { return socketOption(FlexibleDataRateFramesOption).value<FlexibleDataRateFrames>(); }

Q_SIGNALS:
    void canFilterChanged();
    void errorFilterMaskChanged();
    void loopbackChanged();
    void receiveOwnMessagesChanged();
    void flexibleDataRateFramesChanged();

private:
    Q_DISABLE_COPY(CanRawSocket)
    Q_DECLARE_PRIVATE(CanRawSocket)
};

Q_DECLARE_METATYPE(CanRawSocket::Loopback)
Q_DECLARE_METATYPE(CanRawSocket::ReceiveOwnMessages)
Q_DECLARE_METATYPE(CanRawSocket::FlexibleDataRateFrames)



#endif // CANRAWSOCKET_H
