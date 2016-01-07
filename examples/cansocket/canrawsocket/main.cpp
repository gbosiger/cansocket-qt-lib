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

#include <CanSocket/canrawsocket.h>
#include "canrawreader.h"

QT_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication coreApplication(argc, argv);

    QString interfaceName = "vcan0";
    char dataToSend[] = "\x11\x22\x33\x44\x55\xAA\xBB\xCC";

    CanRawSocket *canRawSocket = new CanRawSocket(&coreApplication);
    new CanRawReader(canRawSocket, &coreApplication);

    CanRawFilter rawFilter;
    rawFilter.setFullId(0x1ab);
    rawFilter.setFullIdMask(CanFrame::EFFIdFlag | CanFrame::RTRIdFlag | CanFrame::SFFIdMask);

    CanRawFilterArray rawFilterArray;
    rawFilterArray.append(rawFilter);

    canRawSocket->connectToInterface(interfaceName);
    canRawSocket->setCanFilter(rawFilterArray);
    canRawSocket->setFlexibleDataRateFrames(CanRawSocket::EnabledFDFrames);
    canRawSocket->setErrorFilterMask(CanFrame::AllCanFrameErrors);
    canRawSocket->setReceiveOwnMessages(CanRawSocket::EnabledOwnMessages);
    canRawSocket->setLoopback(CanRawSocket::EnabledLoopback);

    QDataStream dataStream;
    dataStream.setByteOrder(static_cast<QDataStream::ByteOrder>(QSysInfo::ByteOrder));
    dataStream.setDevice(canRawSocket);

    CanFrame canFrame(CanFrame::DataFrame);
    canFrame.setCanId(0x1ab);
    canFrame.setDataLength(8);
    canFrame.setData(dataToSend);

    dataStream << canFrame;

    canFrame.setCanId(CanFrame::BusError);
    canFrame.toErrorFrame();

    dataStream << canFrame;

    canFrame.toDataFrame();
    canFrame.setCanId(0x1cc);
    canFrame.setDataLength(8);

    dataStream << canFrame;


    return coreApplication.exec();
}

