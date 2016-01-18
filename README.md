# cansocket-qt-lib

## Overview

Cansocket-qt-lib is a Qt library for CAN devices using sockets (SocketCAN) under GNU/Linux OS. While C functions, for managing sockets and for reading or writing to them, can easily be used in Qt/C++ applications, it's a lot more convenient to program in a "Qt way" using QIODevice for a base class (and thus mechanisms like signals and slots, event loop, etc.). QIODevice class also guarantees a standardized API.

Goals of this library are:
* supporting all standard CAN protocols that are implemented in the kernel,
* having a robust library that could be used in embedded Linux devices, and
* organization and implementation of the project by following standards of Qt modules.

However, only RAW CAN protocol is currently supported, but the library is designed in a way that makes implementation of other CAN protocols as easy as possible. 

Project is in transition from pre-alpha to alpha stage. All functionalities of RAW CAN protocol are already supported. A main focus is now on unit testing, before any new protocols will be added to the project. 

It must be also said that a QSerialBus module (https://github.com/qtproject/qtserialbus) with QtCanBus classes is released under Qt (as a Technology Preview). The cansocket-qt-lib project was developed independently and its main design was conceived before the public release of QSerialBus module, thus API and the implementation are not the same. In fact credit goes to developers of QtNetwork (https://github.com/qtproject/qtbase/tree/dev/src/network/socket) and QtSerialPort (https://github.com/qtproject/qtserialport) modules, which were used for reference of how to implement a new IO device in Qt properly.

Qt 5.5.1 or higher is required, although with small changes also odler Qt5 versionss work.

More project documentation and information will hopefully be available soon. In case of any questions or desire to participate do not hesitate to contact me.

## Example 

An example for RAW CAN protocol is avaliable in the examples directory. To run it, you need to setup virtual can interface first:
```
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```
Can-utils (https://github.com/linux-can/can-utils) can also be then used for testing. For example dumping (loopback) frames to the terminal:
```
candump -e  any,0:0,#FFFFFFFF
```
or for generating new ones once the example is executed. They will be received in nonblocking mode, by the example application (filtering should be removed from the example first):
```
cangen -v vcan0
```


Code snippet of the example for quick demonstaration:
```
    String interfaceName = "vcan0";
    char dataToSend[] = "\x11\x22\x33\x44\x55\xAA\xBB\xCC";

    CanRawSocket *canRawSocket = new CanRawSocket(&coreApplication);
    new CanRawReader(canRawSocket, &coreApplication);

    CanRawFilter rawFilter;
    rawFilter.setFilterId(0x1ab);
    rawFilter.setFilterMask(CanFrame::EffIdFlag | CanFrame::RtrIdFlag | CanFrame::SffIdMask);

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
```


## Copyright

Copyrgiht © 2016 Georgije Bosiger 

This project may be used under the terms of the GNU Lesser General Public License version 3.0 as published by the Free Software Foundation and appearing in the file LICENSE.md.

