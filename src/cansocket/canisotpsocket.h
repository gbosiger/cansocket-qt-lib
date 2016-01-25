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

#ifndef CANISOTPSOCKET
#define CANISOTPSOCKET

#include <CanSocket/canabstractsocket.h>
#include <CanSocket/canframe.h>
#include <QtCore/qvariant.h>

class CanIsoTpSocketPrivate;
struct CanIsoTpOptionsPrivate;
struct CanIsoTpFlowControlOptionsPrivate;
struct CanIsoTpLinkLayerOptionsPrivate;

class CANSOCKET_EXPORT CanIsoTpOptions
{
    Q_GADGET

public:
    enum IsoTpFlag {
        ListenModeFlag = 0x001,
        ExtAddressingFlag = 0x002,
        TxFramePaddingFlag = 0x004,
        RxFramePadddingFlag = 0x008,
        CheckPadddingLengthFlag = 0x010,
        CheckPaddingDataFlag = 0x020,
        HalfDuplexFlag = 0x040,
        ForceTxMinSepTimeFlag = 0x080,
        ForceRxMinSepTimeFlag = 0x100,
        RxExtAddressingFlag = 0x200,
    };
    Q_FLAG(IsoTpFlag)
    Q_DECLARE_FLAGS(IsoTpFlags, IsoTpFlag)

    CanIsoTpOptions();
    ~CanIsoTpOptions();

    void setIsoTpFlags(IsoTpFlags flags);
    IsoTpFlags isoTpFlags() const;

    void setFrameTxTime(quint32 nsecs);
    quint32 frameTxTime() const;

    void setExtendedAddress(quint8 address);
    quint8 extendedAddres() const;

    void setTxPaddingByte(quint8 content);
    quint8 txPaddingByte() const;

    void setRxPaddingByte(quint8 content);
    quint8 rxPaddingByte() const;

    bool operator ==(const CanIsoTpOptions &rhs) const;
    bool operator !=(const CanIsoTpOptions &rhs) const { return !operator==(rhs); }

private:
    friend class CanIsoTpSocketPrivate;
    CanIsoTpOptionsPrivate *d;
};
Q_DECLARE_METATYPE(CanIsoTpOptions)


class CANSOCKET_EXPORT CanIsoTpFlowControlOptions
{
    Q_GADGET

public:
    enum MinSepTimeCodeOption {
        MinSepTime0ms = 0,
        MinSepTime127ms = 0x7F,
        MinSepTime100us = 0xF1,
        MinSepTime900us = 0xF9
    };
    Q_ENUM(MinSepTimeCodeOption)

    CanIsoTpFlowControlOptions();
    ~CanIsoTpFlowControlOptions();

    void setBlockSize(quint8 size);
    quint8 blockSize() const;

    void setMinSeparationTime(quint8 sepTimeCode);
    quint8 minSeparationTime() const;

    void setWaitFramesMaxNumber(quint8 waitFramesMaxNumber);
    quint8 waitFramesMaxNumber() const;

    bool operator ==(const CanIsoTpFlowControlOptions &rhs) const;
    bool operator !=(const CanIsoTpFlowControlOptions &rhs) const { return !operator==(rhs); }

private:
    friend class CanIsoTpSocketPrivate;
    CanIsoTpFlowControlOptionsPrivate *d;
};
Q_DECLARE_METATYPE(CanIsoTpFlowControlOptions)


class CANSOCKET_EXPORT CanIsoTpLinkLayerOptions
{
    Q_GADGET

public:
    enum MtuOption {
        DataFrameMtu = 16,
        FdFrameMtu = 72
    };
    Q_ENUM(MtuOption)

    enum TxDataLengthOption {
        TxDlen8 = 8,
        TxDlen12 = 12,
        TxDlen16 = 16,
        TxDlen20 = 20,
        TxDlen24 = 24,
        TxDlen32 = 32,
        TxDlen48 = 48,
        TxDlen64 = 64
    };
    Q_ENUM(TxDataLengthOption)

    CanIsoTpLinkLayerOptions();
    ~CanIsoTpLinkLayerOptions();

    void setMaxDataTransferUnit(MtuOption mtu);
    MtuOption maxDataTransferUnit() const;

    void setTxDataLength(TxDataLengthOption dlen);
    TxDataLengthOption txDataLength() const;

    void setTxFdFrameFlags(CanFrame::CanFdFrameFlags flags);
    CanFrame::CanFdFrameFlags txFdFrameFlags() const;

    bool operator ==(const CanIsoTpLinkLayerOptions &rhs) const;
    bool operator !=(const CanIsoTpLinkLayerOptions &rhs) const { return !operator==(rhs); }

private:
    friend class CanIsoTpSocketPrivate;
    CanIsoTpLinkLayerOptionsPrivate *d;
};
Q_DECLARE_METATYPE(CanIsoTpLinkLayerOptions)


class CANSOCKET_EXPORT CanIsoTpSocket : public CanAbstractSocket
{
    Q_OBJECT

    Q_PROPERTY(uint txId READ txId NOTIFY txIdChanged)
    Q_PROPERTY(uint rxId READ rxId NOTIFY rxIdChanged)
    Q_PROPERTY(CanIsoTpOptions isoTpOptions READ isoTpOptions WRITE setIsoTpOptions NOTIFY isoTpOptionsChanged)
    Q_PROPERTY(CanIsoTpFlowControlOptions flowControlOptions READ flowControlOptions WRITE setFlowControlOptions NOTIFY flowControlOptionsChanged)
    Q_PROPERTY(uint txMinSepTime READ txMinSepTime WRITE setTxMinSepTime NOTIFY txMinSepTimeChanged)
    Q_PROPERTY(uint rxMinSepTime READ rxMinSepTime WRITE setRxMinSepTime NOTIFY rxMinSepTimeChanged)
    Q_PROPERTY(CanIsoTpLinkLayerOptions linkLayerOptions READ linkLayerOptions WRITE setLinkLayerOptions NOTIFY linkLayerOptionsChanged)

public:
    enum CanIsoTpSocketOption {
        TxIdOption,
        RxIdOption,
        IsoTpOptions,
        FlowControlOptions,
        TxMinSepTimeOption,
        RxMinSepTimeOption,
        LinkLayerOptions
    };
    Q_ENUM(CanIsoTpSocketOption)

    explicit CanIsoTpSocket(QObject *parent = Q_NULLPTR);
    virtual ~CanIsoTpSocket();

    bool connectToInterface(const QString &interfaceName,
                       uint txId,
                       uint rxId,
                       OpenMode mode);

    void setSocketOption(CanIsoTpSocketOption option, const QVariant &value);
    QVariant socketOption(CanIsoTpSocketOption option);

    bool setTxId(uint id);
    uint txId();

    bool setRxId(uint id);
    uint rxId();

    void setIsoTpOptions(const CanIsoTpOptions &options);
    CanIsoTpOptions isoTpOptions();

    void setFlowControlOptions(const CanIsoTpFlowControlOptions &options);
    CanIsoTpFlowControlOptions flowControlOptions();

    void setTxMinSepTime(uint nsecs);
    uint txMinSepTime();

    void setRxMinSepTime(uint nsecs);
    uint rxMinSepTime();

    void setLinkLayerOptions(const CanIsoTpLinkLayerOptions &options);
    CanIsoTpLinkLayerOptions linkLayerOptions();

Q_SIGNALS:
    void txIdChanged();
    void rxIdChanged();
    void isoTpOptionsChanged();
    void flowControlOptionsChanged();
    void txMinSepTimeChanged();
    void rxMinSepTimeChanged();
    void linkLayerOptionsChanged();

private:
    Q_DISABLE_COPY(CanIsoTpSocket)
    Q_DECLARE_PRIVATE(CanIsoTpSocket)
};


#endif // CANISOTPSOCKET

