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
#include <CanSocket>
#include <QDataStream>

class CanRawReader : public QObject
{
    Q_OBJECT
public:
    explicit CanRawReader(CanRawSocket *canRawSocket, QObject *parent = 0);

public slots:
    void handleReadyRead();
    void handleStateChanged(CanAbstractSocket::SocketState);
    void handleError(CanAbstractSocket::SocketError);

private:
    QDataStream m_dataStream;
    CanRawSocket *m_canRawSocket;
    QTextStream m_standardOutput;
};

#endif // CANRAWREADER_H
