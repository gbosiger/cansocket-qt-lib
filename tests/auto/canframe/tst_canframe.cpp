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
        QVERIFY(!empty.isFDFrame());
        QVERIFY(!empty.isErrorFrame());
        QVERIFY(!empty.isRTRFrame());
        QVERIFY(!(empty.fullId() & CanFrame::ErrorIdFlag));
        QVERIFY(!(empty.fullId() & CanFrame::EFFIdFlag));
        QVERIFY(!(empty.fullId() & CanFrame::RTRIdFlag));

        CanFrame undefined(CanFrame::UnknownFrame);
        QVERIFY(!undefined.isDataFrame());
        QVERIFY(!undefined.isFDFrame());
        QVERIFY(!undefined.isErrorFrame());
        QVERIFY(!undefined.isRTRFrame());
        QVERIFY(!(undefined.fullId() & CanFrame::ErrorIdFlag));
        QVERIFY(!(undefined.fullId() & CanFrame::EFFIdFlag));
        QVERIFY(!(undefined.fullId() & CanFrame::RTRIdFlag));

        CanFrame dataFrame(CanFrame::DataFrame);
        QVERIFY(dataFrame.isDataFrame());
        QVERIFY(!dataFrame.isFDFrame());
        QVERIFY(!dataFrame.isErrorFrame());
        QVERIFY(!dataFrame.isRTRFrame());
        QVERIFY(!(dataFrame.fullId() & CanFrame::ErrorIdFlag));
        QVERIFY(!(dataFrame.fullId() & CanFrame::EFFIdFlag));
        QVERIFY(!(dataFrame.fullId() & CanFrame::RTRIdFlag));

        CanFrame canFDFrame(CanFrame::FDFrame);
        QVERIFY(!canFDFrame.isDataFrame());
        QVERIFY(canFDFrame.isFDFrame());
        QVERIFY(!canFDFrame.isErrorFrame());
        QVERIFY(!canFDFrame.isRTRFrame());
        QVERIFY(!(canFDFrame.fullId() & CanFrame::ErrorIdFlag));
        QVERIFY(!(canFDFrame.fullId() & CanFrame::EFFIdFlag));
        QVERIFY(!(canFDFrame.fullId() & CanFrame::RTRIdFlag));

        CanFrame canErrorFrame(CanFrame::ErrorFrame);
        QVERIFY(!canErrorFrame.isDataFrame());
        QVERIFY(!canErrorFrame.isFDFrame());
        QVERIFY(canErrorFrame.isErrorFrame());
        QVERIFY(!canErrorFrame.isRTRFrame());
        QVERIFY(canErrorFrame.fullId() & CanFrame::ErrorIdFlag);
        QVERIFY(!(canErrorFrame.fullId() & CanFrame::EFFIdFlag));
        QVERIFY(!(canErrorFrame.fullId() & CanFrame::RTRIdFlag));

        CanFrame canRTRFrame(CanFrame::RTRFrame);
        QVERIFY(!canRTRFrame.isDataFrame());
        QVERIFY(!canRTRFrame.isFDFrame());
        QVERIFY(!canRTRFrame.isErrorFrame());
        QVERIFY(canRTRFrame.isRTRFrame());
        QVERIFY(!(canRTRFrame.fullId() & CanFrame::ErrorIdFlag));
        QVERIFY(!(canRTRFrame.fullId() & CanFrame::EFFIdFlag));
        QVERIFY(canRTRFrame.fullId() & CanFrame::RTRIdFlag);
}

QTEST_MAIN(tst_CanFrame)

#include "tst_canframe.moc"
