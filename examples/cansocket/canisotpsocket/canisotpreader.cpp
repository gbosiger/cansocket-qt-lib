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

#include "canisotpreader.h"
#include <QtCore/qdebug.h>

CanIsoTpReader::CanIsoTpReader(CanIsoTpSocket *canIsoTpSocket, QCoreApplication *coreApplication)
    : QObject(coreApplication)
    , m_coreApplication(coreApplication)
    , m_canIsoTpSocket(canIsoTpSocket)
    , m_dataStream(canIsoTpSocket)
    , m_standardOutput(stdout)
{
    connect(m_canIsoTpSocket, SIGNAL(readyRead()), SLOT(handleReadyRead()), Qt::QueuedConnection);
    connect(m_canIsoTpSocket, SIGNAL(stateChanged(CanAbstractSocket::SocketState)), SLOT(handleStateChanged(CanAbstractSocket::SocketState)));
    connect(m_canIsoTpSocket, SIGNAL(error(CanAbstractSocket::SocketError)), SLOT(handleError(CanAbstractSocket::SocketError)));

    m_dataStream.setByteOrder(static_cast<QDataStream::ByteOrder>(QSysInfo::ByteOrder));
}

void CanIsoTpReader::handleReadyRead()
{
    m_standardOutput << m_canIsoTpSocket->readAll().toHex() << endl;
}

void CanIsoTpReader::handleStateChanged(CanIsoTpSocket::SocketState state)
{
    qDebug() << state;
}

void CanIsoTpReader::handleError(CanAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    m_standardOutput << QObject::tr("An error while operating on interface %1, error: %2").arg(m_canIsoTpSocket->interfaceName()).arg(m_canIsoTpSocket->errorString()) << endl;
    m_coreApplication->quit();
}
