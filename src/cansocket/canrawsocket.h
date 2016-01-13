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

class CanRawSocketPrivate;
class CanRawFilterArrayData;

class CANSOCKET_EXPORT CanRawFilter
{
public:

    CanRawFilter(uint id = 0, uint mask = 0);

    CanRawFilter(const CanRawFilter &rhs);

    void setupFilter(uint id, uint mask);
//    inline void setupFilter(qquint3232 canId, qquint3232 mask,
//                            CanFrame::CanFrameFormat format = CanFrame::StandardFrameFormat,
//                            CanFrame::CanFrameType type = CanFrame::DataFrame);

    inline void setFilterId(uint id) { this->id = id; }
    inline uint filterId() const { return id; }

    inline void setFilterMask(uint mask) { this->mask = mask; }
    inline uint filterMask() const { return mask; }

    inline bool operator ==(const CanRawFilter &rhs) const {
        return (id == rhs.id) && (mask == rhs.mask);
    }
    inline bool operator !=(const CanRawFilter &rhs) const { return !operator==(rhs); }

private:
    quint32 id;
    quint32 mask;
};
Q_DECLARE_METATYPE(CanRawFilter)

class CANSOCKET_EXPORT CanRawFilterArray
{
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
    Q_PROPERTY(CanFrame::CanFrameErrors errorFilterMask READ errorFilterMask WRITE setErrorFilterMask NOTIFY errorFilterMaskChanged)
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

    void setCanFilter(const CanRawFilterArray &filter);
    CanRawFilterArray canFilter();

    void setErrorFilterMask(const CanFrame::CanFrameErrors mask);
    CanFrame::CanFrameErrors errorFilterMask();

    void setLoopback(Loopback loopback);
    Loopback loopback();

    void setReceiveOwnMessages(ReceiveOwnMessages ownMessages);
    ReceiveOwnMessages receiveOwnMessages();

    void setFlexibleDataRateFrames(FlexibleDataRateFrames fdFrames);
    FlexibleDataRateFrames flexibleDataRateFrames();

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

#endif // CANRAWSOCKET_H
