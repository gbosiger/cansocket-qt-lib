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

#ifndef CANRAWREADER_H
#define CANRAWREADER_H

#include <QObject>
#include <CanSocket/canrawsocket.h>
#include <QCoreApplication>
#include <QDataStream>
#include <QTextStream>

class CanRawReader : public QObject
{
    Q_OBJECT
public:
    explicit CanRawReader(CanRawSocket *canRawSocket, QCoreApplication *coreApplication);

public slots:
    void handleReadyRead();
    void handleStateChanged(CanAbstractSocket::SocketState);
    void handleError(CanAbstractSocket::SocketError);

signals:
    void error();

private:
    QCoreApplication *m_coreApplication;
    CanRawSocket *m_canRawSocket;
    QDataStream m_dataStream;
    QTextStream m_standardOutput;
};

#endif // CANRAWREADER_H
