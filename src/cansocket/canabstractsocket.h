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

#ifndef CANABSTRACTSOCKET_H
#define CANABSTRACTSOCKET_H

#include <QtCore/qiodevice.h>
#include <QtCore/qobject.h>

#include <CanSocket/cansocketglobal.h>

class CanAbstractSocketPrivate;

class CANSOCKET_EXPORT CanAbstractSocket : public QIODevice
{
    Q_OBJECT

    Q_PROPERTY(SocketType socketType READ socketType)
    Q_PROPERTY(SocketState socketState READ socketState NOTIFY stateChanged)
    Q_PROPERTY(SocketError error READ error RESET clearError NOTIFY error)

public:
    enum SocketType {
        RawSocket,
        BcmSocket,
        Tp16Socket,
        Tp20Socket,
        IsoTpSocket,
        UnkownCanSocketType = -1
    };
    Q_ENUM(SocketType)

    enum SocketError {
        UnkownSocketError,
        NoSuchDeviceError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,
        UnsupportedSocketOperationError,
        OperationError,
        WriteError,
        ReadError,

        NoError = -1
    };
    Q_ENUM(SocketError)

    enum SocketState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };
    Q_ENUM(SocketState)

    CanAbstractSocket(SocketType socketType, QObject *parent = Q_NULLPTR);
    virtual ~CanAbstractSocket();

    virtual bool connectToInterface(const QString &interfaceName, OpenMode mode = ReadWrite);
    virtual void disconnectFromInterface();

    void close() Q_DECL_OVERRIDE;
    bool isSequential() const Q_DECL_OVERRIDE;
    bool atEnd() const Q_DECL_OVERRIDE;
    bool canReadLine() const Q_DECL_OVERRIDE;
    bool flush();

    QString interfaceName() const;

    virtual qintptr socketDescriptor() const;

    SocketType socketType() const;
    SocketState socketState() const;
    SocketError error() const;
    QString errorString() const;

    void clearError();

    qint64 readBufferSize() const;
    virtual void setReadBufferSize(qint64 size);

    qint64 bytesAvailable() const Q_DECL_OVERRIDE;
    qint64 bytesToWrite() const Q_DECL_OVERRIDE;

    bool waitForReadyRead(int msecs) Q_DECL_OVERRIDE;
    bool waitForBytesWritten(int msecs) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void stateChanged(CanAbstractSocket::SocketState);
    void error(CanAbstractSocket::SocketError);

protected:
    qint64 readData(char *data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 readLineData(char *data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeData(const char *data, qint64 maxSize) Q_DECL_OVERRIDE;

    inline void setSocketType(SocketType socketType);
    inline void setSocketState(SocketState socketState);
    inline void setSocketError(SocketError socketError, QString error);

    CanAbstractSocket(SocketType socketType, CanAbstractSocketPrivate &dd, QObject *parent = Q_NULLPTR);

private:
    Q_DECLARE_PRIVATE(CanAbstractSocket)
    Q_DISABLE_COPY(CanAbstractSocket)
};

Q_DECLARE_METATYPE(CanAbstractSocket::SocketState)
Q_DECLARE_METATYPE(CanAbstractSocket::SocketError)

#endif // CANABSTRACTSOCKET_H
