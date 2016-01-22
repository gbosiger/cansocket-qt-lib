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

#include "canrawreader.h"
#include <CanSocket/canframe.h>
#include <QtCore/qdebug.h>

CanRawReader::CanRawReader(CanRawSocket *canRawSocket, QCoreApplication *coreApplication)
    : QObject(coreApplication)
    , m_coreApplication(coreApplication)
    , m_canRawSocket(canRawSocket)
    , m_dataStream(canRawSocket)
    , m_standardOutput(stdout)
{
    connect(m_canRawSocket, SIGNAL(readyRead()), SLOT(handleReadyRead()));
    connect(m_canRawSocket, SIGNAL(stateChanged(CanAbstractSocket::SocketState)), SLOT(handleStateChanged(CanAbstractSocket::SocketState)));
    connect(m_canRawSocket, SIGNAL(error(CanAbstractSocket::SocketError)), SLOT(handleError(CanAbstractSocket::SocketError)));

    m_dataStream.setByteOrder(static_cast<QDataStream::ByteOrder>(QSysInfo::ByteOrder));
}

void CanRawReader::handleReadyRead()
{
    CanFrame readFrame;
    while(!m_dataStream.atEnd()) {
        m_dataStream >> readFrame;
        qDebug() << readFrame;
    }
}

void CanRawReader::handleStateChanged(CanRawSocket::SocketState state)
{
    qDebug() << state;
}

void CanRawReader::handleError(CanAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    m_standardOutput << QObject::tr("An error while operating on interface %1, error: %2").arg(m_canRawSocket->interfaceName()).arg(m_canRawSocket->errorString()) << endl;
    m_coreApplication->quit();
}
