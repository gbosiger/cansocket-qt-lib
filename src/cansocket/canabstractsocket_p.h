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

#ifndef CANABSTRACTSOCKET_P_H
#define CANABSTRACTSOCKET_P_H

#include "CanSocket/canabstractsocket.h"

#include "private/qringbuffer_p.h"
#include "private/qiodevice_p.h"

#include "qsocketnotifier.h"

class CanAbstractSocketErrorInfo
{
public:
    explicit CanAbstractSocketErrorInfo(CanAbstractSocket::SocketError newErrorCode = CanAbstractSocket::UnkownSocketError,
                                        const QString &newErrorString = QString());

    CanAbstractSocket::SocketError errorCode;
    QString errorString;
};

class CanAbstractSocketPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(CanAbstractSocket)

public:

    CanAbstractSocketPrivate(quint32 readChunkSize, quint64 initialBufferSize);
    virtual ~CanAbstractSocketPrivate();

    const qint32 readChunkSize;

    static int timeoutValue(int msecs, int elapsed);

    virtual bool connectToInterface(const QString &interfaceName);
    virtual void disconnectFromInterface();

    CanAbstractSocketErrorInfo getSystemError(int systemErrorCode = -1) const;

    void setError(const CanAbstractSocketErrorInfo &errorInfo);

    qint64 writeData(const char *data, qint64 maxSize);

    bool waitForReadyRead(int msecs);
    bool waitForBytesWritten(int msecs);

    qint64 readBufferMaxSize;
    QRingBuffer writeBuffer;

    CanAbstractSocket::SocketType type;
    CanAbstractSocket::SocketError error;
    CanAbstractSocket::SocketState state;

    QString interfaceName;

    bool readNotification();
    bool startAsyncWrite();
    bool completeAsyncWrite();

    bool isReadNotificationEnabled() const;
    void setReadNotificationEnabled(bool enable);
    bool isWriteNotificationEnabled() const;
    void setWriteNotificationEnabled(bool enable);

    bool waitForReadOrWrite(bool *selectForRead, bool *selectForWrite,
                            bool checkRead, bool checkWrite,
                            int msecs);

    virtual qint64 readFromSocket(char *data, qint64 maxSize);
    virtual qint64 writeToSocket(const char *data, qint64 maxSize);

    qintptr descriptor;

    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;

    bool readSocketNotifierCalled;
    bool readSocketNotifierState;
    bool readSocketNotifierStateSet;

    bool emittedReadyRead;
    bool emittedBytesWritten;

    qint64 pendingBytesWritten;
    bool writeSequenceStarted;

};

#endif // CANABSTRACTSOCKET_P_H

