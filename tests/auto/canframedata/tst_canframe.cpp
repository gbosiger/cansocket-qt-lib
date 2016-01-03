#include <QObject>
#include <QString>
#include <QtTest>

#include <CanSocket/canframe.h>
#include <linux/can.h>

class tst_CanFrame : public QObject
{
    Q_OBJECT

public:
    tst_CanFrame();

private Q_SLOTS:
    void constructors();
};

tst_CanFrame::tst_CanFrame()
{
}

void tst_CanFrame::constructors()
{
        CanFrame empty;
        QVERIFY(!empty.isCanFrame());
        QVERIFY(!empty.isCanFDFrame());
        QVERIFY(!empty.isCanErrorFrame());
        QVERIFY(!empty.isCanRTRFrame());
        QVERIFY(!empty.errFlag());
        QVERIFY(!empty.effFlag());
        QVERIFY(!empty.rtrFlag());

        CanFrame undefined(CanFrame::UnknownFrameType);
        QVERIFY(!undefined.isCanFrame());
        QVERIFY(!undefined.isCanFDFrame());
        QVERIFY(!undefined.isCanErrorFrame());
        QVERIFY(!undefined.isCanRTRFrame());
        QVERIFY(!undefined.errFlag());
        QVERIFY(!undefined.effFlag());
        QVERIFY(!undefined.rtrFlag());

        CanFrame canFrame;
        QVERIFY(canFrame.isCanFrame());
        QVERIFY(!canFrame.isCanFDFrame());
        QVERIFY(!canFrame.isCanErrorFrame());
        QVERIFY(!canFrame.isCanRTRFrame());
        QVERIFY(!canFrame.errFlag());
        QVERIFY(!canFrame.effFlag());
        QVERIFY(!canFrame.rtrFlag());

        CanFDFrame canFDFrame;
        QVERIFY(!canFDFrame.isCanFrame());
        QVERIFY(canFDFrame.isCanFDFrame());
        QVERIFY(!canFDFrame.isCanErrorFrame());
        QVERIFY(!canFDFrame.isCanRTRFrame());
        QVERIFY(!canFDFrame.errFlag());
        QVERIFY(!canFDFrame.effFlag());
        QVERIFY(!canFDFrame.rtrFlag());

        CanErrorFrame canErrorFrame;
        QVERIFY(canErrorFrame.isCanFrame());
        QVERIFY(!canErrorFrame.isCanFDFrame());
        QVERIFY(canErrorFrame.isCanErrorFrame());
        QVERIFY(!canErrorFrame.isCanRTRFrame());
        QVERIFY(canErrorFrame.errFlag());
        QVERIFY(!canErrorFrame.effFlag());
        QVERIFY(!canErrorFrame.rtrFlag());

        CanRTRFrame canRTRFrame;
        QVERIFY(canRTRFrame.isCanFrame());
        QVERIFY(!canRTRFrame.isCanFDFrame());
        QVERIFY(!canRTRFrame.isCanErrorFrame());
        QVERIFY(canRTRFrame.isCanRTRFrame());
        QVERIFY(!canRTRFrame.errFlag());
        QVERIFY(!canRTRFrame.effFlag());
        QVERIFY(canRTRFrame.rtrFlag());
}

QTEST_MAIN(tst_CanFrame)

#include "tst_canframe.moc"
