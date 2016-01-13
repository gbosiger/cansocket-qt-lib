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
        QVERIFY(!empty.isDataFrame());
        QVERIFY(!empty.isFdFrame());
        QVERIFY(!empty.isErrorFrame());
        QVERIFY(!empty.isRtrFrame());
        QVERIFY(!(empty.id() & CanFrame::ErrorIdFlag));
        QVERIFY(!(empty.id() & CanFrame::EffIdFlag));
        QVERIFY(!(empty.id() & CanFrame::RtrIdFlag));

        CanFrame undefined(CanFrame::UnknownFrame);
        QVERIFY(!undefined.isDataFrame());
        QVERIFY(!undefined.isFdFrame());
        QVERIFY(!undefined.isErrorFrame());
        QVERIFY(!undefined.isRtrFrame());
        QVERIFY(!(undefined.id() & CanFrame::ErrorIdFlag));
        QVERIFY(!(undefined.id() & CanFrame::EffIdFlag));
        QVERIFY(!(undefined.id() & CanFrame::RtrIdFlag));

        CanFrame dataFrame(CanFrame::DataFrame);
        QVERIFY(dataFrame.isDataFrame());
        QVERIFY(!dataFrame.isFdFrame());
        QVERIFY(!dataFrame.isErrorFrame());
        QVERIFY(!dataFrame.isRtrFrame());
        QVERIFY(!(dataFrame.id() & CanFrame::ErrorIdFlag));
        QVERIFY(!(dataFrame.id() & CanFrame::EffIdFlag));
        QVERIFY(!(dataFrame.id() & CanFrame::RtrIdFlag));

        CanFrame canFdFrame(CanFrame::FdFrame);
        QVERIFY(!canFdFrame.isDataFrame());
        QVERIFY(canFdFrame.isFdFrame());
        QVERIFY(!canFdFrame.isErrorFrame());
        QVERIFY(!canFdFrame.isRtrFrame());
        QVERIFY(!(canFdFrame.id() & CanFrame::ErrorIdFlag));
        QVERIFY(!(canFdFrame.id() & CanFrame::EffIdFlag));
        QVERIFY(!(canFdFrame.id() & CanFrame::RtrIdFlag));

        CanFrame canErrorFrame(CanFrame::ErrorFrame);
        QVERIFY(!canErrorFrame.isDataFrame());
        QVERIFY(!canErrorFrame.isFdFrame());
        QVERIFY(canErrorFrame.isErrorFrame());
        QVERIFY(!canErrorFrame.isRtrFrame());
        QVERIFY(canErrorFrame.id() & CanFrame::ErrorIdFlag);
        QVERIFY(!(canErrorFrame.id() & CanFrame::EffIdFlag));
        QVERIFY(!(canErrorFrame.id() & CanFrame::RtrIdFlag));

        CanFrame canRtrFrame(CanFrame::RtrFrame);
        QVERIFY(!canRtrFrame.isDataFrame());
        QVERIFY(!canRtrFrame.isFdFrame());
        QVERIFY(!canRtrFrame.isErrorFrame());
        QVERIFY(canRtrFrame.isRtrFrame());
        QVERIFY(!(canRtrFrame.id() & CanFrame::ErrorIdFlag));
        QVERIFY(!(canRtrFrame.id() & CanFrame::EffIdFlag));
        QVERIFY(canRtrFrame.id() & CanFrame::RtrIdFlag);
}

QTEST_MAIN(tst_CanFrame)

#include "tst_canframe.moc"
