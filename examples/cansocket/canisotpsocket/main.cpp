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

#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QByteArray>
#include <QString>

#include <CanSocket/canisotpsocket.h>
#include <CanSocket/canframe.h>
#include "canisotpreader.h"

QT_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication coreApplication(argc, argv);

    QString interfaceName = "vcan0";
    char dataToSend[] = "\x11\x22\x33\x44\x55\xAA\xBB\xCC\x11\x22\x33\x55\xAA\xBB\xCC\xDD";

    CanIsoTpSocket *canIsoTpSocket = new CanIsoTpSocket(&coreApplication);
    new CanIsoTpReader(canIsoTpSocket, &coreApplication);

    canIsoTpSocket->setTxId(0x0A);
    canIsoTpSocket->setRxId(0x01);

    canIsoTpSocket->connectToInterface(interfaceName);

    canIsoTpSocket->write(dataToSend, sizeof(dataToSend) - 1);

    return coreApplication.exec();
}
