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

#ifndef CANISOTPSOCKET_P_H
#define CANISOTPSOCKET_P_H

#include <CanSocket/canisotpsocket.h>
#include <private/canabstractsocket_p.h>

class CanIsoTpSocketPrivate : CanAbstractSocketPrivate
{
    Q_DECLARE_PUBLIC(CanIsoTpSocket)

public:
    CanIsoTpSocketPrivate(quint32 readChunkSize, quint64 initialBufferSize);
    virtual ~CanIsoTpSocketPrivate();

    bool connectToInterface(const QString &interfaceName) Q_DECL_OVERRIDE;

    bool setSocketOption(CanIsoTpSocket::CanIsoTpSocketOption option, const QVariant &value);
    QVariant socketOption(CanIsoTpSocket::CanIsoTpSocketOption option);

    qint64 readFromSocket(char *data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeToSocket(const char *data, qint64 maxSize) Q_DECL_OVERRIDE;

   int msgSize() const Q_DECL_OVERRIDE;

   quint32 txId;
   quint32 rxId;
   CanIsoTpOptions tpOpt;
   CanIsoTpFlowControlOptions flowCtrlOpt;
   CanIsoTpLinkLayerOptions llOpt;
};

#endif // CANISOTPSOCKET_P_H
