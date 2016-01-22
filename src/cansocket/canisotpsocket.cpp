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

#define CAN_ISOTP_READ_CHUNK_SIZE = 4096 // max. data
#define CAN_ISOTP_INITIAL_BUFFER_SIZE = 16384

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

    inline bool operator ==(const CanIsoTpOptionsPrivate &rhs) const
    {
        return (flags == rhs.flags
                && frameTxTime == rhs.frameTxTime
                && extAddress == rhs.extAddress
                && txPadContent == rhs.txPadContent
                && rxPadContent == rhs.rxPadContent
                && rxExtAddress == rhs.rxExtAddress);
    }

    inline bool operator !=(const CanIsoTpOptionsPrivate &rhs) const { return !operator==(rhs); }

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

    inline bool operator ==(const CanIsoTpFlowControlOptionsPrivate &rhs) const
    {
        return (blockSize == rhs.blockSize
                && stMin == rhs.stMin
                && wftMax == rhs.wftMax);
    }

    inline bool operator !=(const CanIsoTpFlowControlOptionsPrivate &rhs) const { return !operator==(rhs); }

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

    inline bool operator ==(const CanIsoTpLinkLayerOptionsPrivate &rhs) const {
        return (mtu == rhs.mtu
                && txDlen == rhs.txDlen
                && txFdFlags == rhs.txFdFlags);
    }
    inline bool operator !=(const CanIsoTpLinkLayerOptionsPrivate &rhs) const { return !operator==(rhs); }

    quint8 mtu;
    quint8 txDlen;
    quint8 txFdFlags;
};

CanIsoTpSocket::CanIsoTpSocket(QObject *parent)
    : CanAbstractSocket(IsoTpSocket,
                        *new CanIsoTpSocketPrivate(CAN_ISOTP_READ_CHUNK_SIZE, CAN_ISOTP_INITIAL_BUFFER_SIZE),
                        parent)
{
}

CanIsoTpSocket::~CanIsoTpSocket()
{
}


