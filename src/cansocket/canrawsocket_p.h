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

#ifndef CANRAWSOCKET_P_H
#define CANRAWSOCKET_P_H

#include <CanSocket/canrawsocket.h>
#include <private/canabstractsocket_p.h>

class CanRawSocketPrivate : CanAbstractSocketPrivate
{
    Q_DECLARE_PUBLIC(CanRawSocket)

public:
    CanRawSocketPrivate();
    virtual ~CanRawSocketPrivate();

    bool connectToInterface(const QString &interfaceName) Q_DECL_OVERRIDE;

    bool setSocketOption(CanRawSocket::CanRawSocketOption option, const QVariant &value);
    QVariant socketOption(CanRawSocket::CanRawSocketOption option);

    qint64 readFromSocket(char *data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeToSocket(const char *data, qint64 maxSize) Q_DECL_OVERRIDE;

   int msgSize() const Q_DECL_OVERRIDE;

   CanRawFilterArray canFilter;
   CanFrame::CanFrameErrors errorFilterMask;
   CanRawSocket::Loopback loopback;
   CanRawSocket::ReceiveOwnMessages receiveOwnMessages;
   CanRawSocket::FlexibleDataRateFrames flexibleDataRateFrames;
};

#endif // CANRAWSOCKET_P_H
