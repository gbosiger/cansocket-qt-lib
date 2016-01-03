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

#ifndef CANFRAME_P
#define CANFRAME_P

#ifdef Q_OS_LINUX
#   include <linux/can.h>
#   include <linux/can/raw.h>
#   include <linux/can/error.h>
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

inline quint8 res0FromCanMTU(int mtu)
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

inline quint8 res1FromCanMTU(int mtu)
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

inline int dataLengthFromResBytes(const quint8 &res0, const quint8 &res1)
{
    if (res0 == CAN_MAX_DLEN && res1 == CAN_MTU)
        return CAN_MAX_DLEN;
#ifdef CANFD_MTU
    else if (res0 == CANFD_MAX_DLEN && res1 == CANFD_MTU)
        return CANFD_MAX_DLEN;
#endif
    else
        return -1;
}

#include <QtCore/qshareddata.h>
#include <QtCore/qvector.h>

quint8 res0FromCanMTU(int mtu);
quint8 res1FromCanMTU(int mtu);


class CanFrameData : public QSharedData
{
public:
    CanFrameData()
        : QSharedData()
        , id(0)
        , dlen(0)
        , flags(0)
        , res0(0)
        , res1(0)
        , data()
    {
    }

    CanFrameData(const CanFrameData &other)
        : QSharedData(other)
        , id(other.id)
        , dlen(other.dlen)
        , flags(other.flags)
        , res0(other.res0)
        , res1(other.res1)
        , data(other.data)
    {
    }

    ~CanFrameData()
    {
    }

    inline void clear() {
        id = 0;
        dlen = 0;
        flags = 0;
        res0 = 0;
        res1 = 0;
        data.clear();
    }

    inline bool errFlag() const {
        return (id & CAN_ERR_FLAG) != 0;
    }

    inline bool rtrFlag() const {
        return (id & CAN_RTR_FLAG) != 0;
    }

    inline bool effFlag() const {
        return (id & CAN_EFF_FLAG) != 0;
    }

    inline void setErrFlag(bool err)
    {
        if (err)
            id |= CAN_ERR_FLAG;
        else
            id &= ~CAN_ERR_FLAG;
    }

    inline void setRtrFlag(bool rtr)
    {
        if (rtr)
            id |= CAN_RTR_FLAG;
        else
            id &= ~CAN_RTR_FLAG;
    }

    inline void setEffFlag(bool eff)
    {
        if (eff)
            id |= CAN_EFF_FLAG;
        else
            id &= ~CAN_EFF_FLAG;
    }

    inline bool isDataFrame() const {
        return (dlen <= CAN_MAX_DLEN
                && res0 == res0FromCanMTU(CAN_MTU)
                && res1 == res1FromCanMTU(CAN_MTU)
                && data.size() == CAN_MAX_DLEN
                && !errFlag()
                && !rtrFlag());
    }

    inline bool isFDFrame() const {
        return (dlen <= CANFD_MAX_DLEN
                && res0 == res0FromCanMTU(CANFD_MTU)
                && res1 == res1FromCanMTU(CANFD_MTU)
                && data.size() == CANFD_MAX_DLEN
                && !errFlag()
                && !rtrFlag());
    }

    inline bool isErrorFrame() const {
        return (dlen == CAN_MAX_DLEN
                && res0 == res0FromCanMTU(CAN_MTU)
                && res1 == res1FromCanMTU(CAN_MTU)
                && data.size() == CAN_MAX_DLEN
                && errFlag()
                && !rtrFlag());
    }

    inline bool isRTRFrame() const {
        return (dlen == 0
                && res0 == res0FromCanMTU(CAN_MTU)
                && res1 == res1FromCanMTU(CAN_MTU)
                && data.size() == CAN_MAX_DLEN
                && !errFlag()
                && rtrFlag());
    }

    inline void toDataFrame() {

        if (dlen > CAN_MAX_DLEN)
            dlen = CAN_MAX_DLEN;

        flags = 0;

        res0 = res0FromCanMTU(CAN_MTU);
        res1 = res1FromCanMTU(CAN_MTU);

        data.resize(CAN_MAX_DLEN);

        if (rtrFlag())
            setRtrFlag(false);
        else if (errFlag())
            setErrFlag(false);
    }

    inline void toFDFrame() {

        if (dlen > CANFD_MAX_DLEN)
            dlen = CANFD_MAX_DLEN;

        res0 = res0FromCanMTU(CANFD_MTU);
        res1 = res1FromCanMTU(CANFD_MTU);

        data.resize(CANFD_MAX_DLEN);

        if (rtrFlag())
            setRtrFlag(false);
        else if (errFlag())
            setErrFlag(false);
    }

    inline void toErrorFrame() {

        if (dlen != CAN_MAX_DLEN)
            dlen = CAN_MAX_DLEN;

        flags = 0;

        res0 = res0FromCanMTU(CAN_MTU);
        res1 = res1FromCanMTU(CAN_MTU);

        data.resize(CAN_MAX_DLEN);

        if (rtrFlag())
            setRtrFlag(false);
        if (!errFlag())
            setErrFlag(true);
    }

    inline void toRTRFrame() {

        if (dlen != 0)
            dlen = 0;

        flags = 0;

        res0 = res0FromCanMTU(CAN_MTU);
        res1 = res1FromCanMTU(CAN_MTU);

        data = QVector<char>(CAN_MAX_DLEN, '0');

        if (!rtrFlag())
            setRtrFlag(true);
        if (errFlag())
            setErrFlag(false);
    }

    quint32 id;
    quint8 dlen;
    quint8 flags;
    quint8 res0;
    quint8 res1;
    QVector<char> data;
};

#endif // CANFRAME_P

