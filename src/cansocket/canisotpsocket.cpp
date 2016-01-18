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

#include "canabstractsocket_p.h"
#include "canisotpsocket.h"

struct CanIsoTpOptionsPrivate {
    CanIsoTpOptionsPrivate();

    inline bool operator ==(const CanIsoTpOptionsPrivate &rhs) const;
    inline bool operator !=(const CanIsoTpOptionsPrivate &rhs) const { return !operator==(rhs); }

    quint32 flags;
    quint32 frameTxTime;
    quint8 extAddress;
    quint8 txPadContent;
    quint8 rxPadContent;
    quint8 rxExtAddress;
};

struct CanIsoTpFlowControlOptionsPrivate {
    CanIsoTpFlowControlOptionsPrivate();

    inline bool operator ==(const CanIsoTpFlowControlOptionsPrivate &rhs) const;
    inline bool operator !=(const CanIsoTpFlowControlOptionsPrivate &rhs) const { return !operator==(rhs); }

    quint8 blockSize;
    quint8 stMin;
    quint8 wftMax;
};

struct CanIsoTpLinkLayerOptionsPrivate {
    CanIsoTpLinkLayerOptionsPrivate();

    inline bool operator ==(const CanIsoTpLinkLayerOptionsPrivate &rhs) const;
    inline bool operator !=(const CanIsoTpLinkLayerOptionsPrivate &rhs) const { return !operator==(rhs); }

    quint8 mtu;
    quint8 txDlen;
    quint8 txFdFlags;
};
