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

};

tst_CanFrame::tst_CanFrame()
{
}

QTEST_MAIN(tst_CanFrame)

#include "tst_canframe.moc"
