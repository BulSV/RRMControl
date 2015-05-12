#include "Dialog.h"
#include "LCDSpinBox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QDesktopWidget>
#include <QShortcut>
#include <QSerialPortInfo>
#include <QPalette>
#include <QIcon>

#define STARTBYTE 0x55
#define STOPBYTE 0xAA
#define BYTESLENTH 8

#define FORMAT 'f'
#define PRECISION 2

#define BLINKTIMETX 200 // ms
#define BLINKTIMERX 500 // ms
#define DISPLAYTIME 100 // ms

#define REWINDTIME 50 // ms

#define TEMPRANGE_MIN -50 // degrees Celsius
#define TEMPRANGE_MAX 50 // degrees Celsius
#define TEMPRANGE_STEP 1 // degrees Celsius
#define NORMAL_TEMP 35 // degrees Celsius

#define DPRANGE_MIN 0
#define DPRANGE_MAX 1024
#define DPRANGE_STEP 1

#define DP1_DIGITS 4
#define DP2_DIGITS 4
#define TEMP_DIGITS 6

#define CODE_WRITE 0x00
#define CODE_DP1 0x01
#define CODE_DP2 0x02
#define CODE_TEMP 0x03
#define CODE_CALIBR 0x04

#define NONE_DATA 0x00

Dialog::Dialog(QWidget *parent) :
        QDialog(parent),
        lPort(new QLabel(QString::fromUtf8("Port"), this)),
        cbPort(new QComboBox(this)),
        lBaud(new QLabel(QString::fromUtf8("Baud"), this)),
        cbBaud(new QComboBox(this)),
        bPortStart(new QPushButton(QString::fromUtf8("Start"), this)),
        bPortStop(new QPushButton(QString::fromUtf8("Stop"), this)),
        lTx(new QLabel("  Tx  ", this)),
        lRx(new QLabel("  Rx  ", this)),
//        sbSetDP1(new LCDSpinBox(QIcon(":/Resources/down.png"), QIcon(":/Resources/up.png"),
//                              QString::fromUtf8(""), QString::fromUtf8(""), QLCDNumber::Dec, LCDSpinBox::RIGHT, this)),
//        sbSetDP2(new LCDSpinBox(QIcon(":/Resources/down.png"), QIcon(":/Resources/up.png"),
//                              QString::fromUtf8(""), QString::fromUtf8(""), QLCDNumber::Dec, LCDSpinBox::RIGHT, this)),
//        sbSetTemp(new LCDSpinBox(QIcon(":/Resources/down.png"), QIcon(":/Resources/up.png"),
//                              QString::fromUtf8(""), QString::fromUtf8(""), QLCDNumber::Dec, LCDSpinBox::RIGHT, this)),
        sbSetDP1(new QSpinBox(this)),
        sbSetDP2(new QSpinBox(this)),
        sbSetTemp(new QSpinBox(this)),
        lcdDP1(new QLCDNumber(this)),
        lcdDP2(new QLCDNumber(this)),
        lcdSensorTemp(new QLCDNumber(this)),
        lDP1(new QLabel(QString::fromUtf8("Offset:"), this)),
        lDP2(new QLabel(QString::fromUtf8("Gain:"), this)),
        lSensor(new QLabel(QString::fromUtf8("Sensor, °C:"), this)),
        bSetDP1(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetDP2(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetTemp(new QPushButton(QString::fromUtf8("Set"), this)),
        gbSetDP1(new QGroupBox(QString::fromUtf8("Offset"), this)),
        gbSetDP2(new QGroupBox(QString::fromUtf8("Gain"), this)),
        gbSetTemp(new QGroupBox(QString::fromUtf8("Temperature, °C"), this)),
        gbConfig(new QGroupBox(QString::fromUtf8("Configure"), this)),
        gbInfo(new QGroupBox(QString::fromUtf8("Information"), this)),
        bWrite(new QPushButton(QString::fromUtf8("Write"), this)),
        bCalibr(new QPushButton(QString::fromUtf8("Calibrate"), this)),
        itsPort(new QSerialPort(this)),
        itsComPort(new ComPort(itsPort, STARTBYTE, STOPBYTE, BYTESLENTH, true, this)),
        itsProtocol(new RRMProtocol(itsComPort, this)),
        itsBlinkTimeTxNone(new QTimer(this)),
        itsBlinkTimeRxNone(new QTimer(this)),
        itsBlinkTimeTxColor(new QTimer(this)),
        itsBlinkTimeRxColor(new QTimer(this)),
        itsTimeToDisplay(new QTimer(this)),
        m_isDP1Set(false),
        m_isDp2Set(false),
        m_isTempSet(false)
{
//    dynamic_cast<QPushButton *>( sbSetDP1->buttonUpWidget() )->setMaximumSize(20, 20);
//    dynamic_cast<QPushButton *>( sbSetDP1->buttonDownWidget() )->setMaximumSize(20, 20);

//    dynamic_cast<QPushButton *>( sbSetDP2->buttonUpWidget() )->setMaximumSize(20, 20);
//    dynamic_cast<QPushButton *>( sbSetDP2->buttonDownWidget() )->setMaximumSize(20, 20);

//    dynamic_cast<QPushButton *>( sbSetTemp->buttonUpWidget() )->setMaximumSize(20, 20);
//    dynamic_cast<QPushButton *>( sbSetTemp->buttonDownWidget() )->setMaximumSize(20, 20);

    bSetDP1->setMaximumSize(60, 30);
    bSetDP1->setMinimumSize(60, 30);

    bSetDP2->setMaximumSize(60, 30);
    bSetDP2->setMinimumSize(60, 30);

    bSetTemp->setMaximumSize(60, 30);
    bSetTemp->setMinimumSize(60, 30);

//    bCalibr->setMaximumSize(80, 30);
//    bCalibr->setMinimumSize(80, 30);

//    bWrite->setMaximumSize(80, 30);
//    bWrite->setMinimumSize(80, 30);

    bCalibr->setMinimumHeight(30);
    bCalibr->setMaximumHeight(30);

    bWrite->setMinimumHeight(30);
    bWrite->setMaximumHeight(30);

    lTx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    lTx->setFrameStyle(QFrame::Box);
    lTx->setAlignment(Qt::AlignCenter);
    lTx->setMargin(2);

    lRx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    lRx->setFrameStyle(QFrame::Box);
    lRx->setAlignment(Qt::AlignCenter);
    lRx->setMargin(2);

    QGridLayout *gridDP1 = new QGridLayout;
    gridDP1->addWidget(sbSetDP1, 0, 0);
    gridDP1->addWidget(bSetDP1, 1, 0);

    QGridLayout *gridDP2 = new QGridLayout;
    gridDP2->addWidget(sbSetDP2, 0, 0);
    gridDP2->addWidget(bSetDP2, 1, 0);

    QGridLayout *gridTemp = new QGridLayout;
    gridTemp->addWidget(sbSetTemp, 0, 0);
    gridTemp->addWidget(bSetTemp, 1, 0);

    gbSetDP1->setLayout(gridDP1);
    gbSetDP2->setLayout(gridDP2);
    gbSetTemp->setLayout(gridTemp);

    QGridLayout *gridConfig = new QGridLayout;
    gridConfig->addWidget(gbSetDP1, 0, 0, 1, 2);
    gridConfig->addWidget(gbSetDP2, 0, 2, 1, 2);
    gridConfig->addWidget(gbSetTemp, 0, 4, 1, 2);
    gridConfig->addWidget(bCalibr, 1, 2, 1, 2);
    gridConfig->addWidget(bWrite, 1, 4, 1, 2);

    gbConfig->setLayout(gridConfig);

    QFrame *frame1 = new QFrame(this);
    frame1->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lDP1->setMargin(5);

    QFrame *frame2 = new QFrame(this);
    frame2->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lDP2->setMargin(5);

    QFrame *frame3 = new QFrame(this);
    frame3->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lSensor->setMargin(5);

    QGridLayout *gridInfo = new QGridLayout;
    gridInfo->addWidget(lDP1, 0, 0);
    gridInfo->addWidget(lcdDP1, 0, 1);
    gridInfo->addWidget(frame1, 0, 0, 1, 2);
    gridInfo->addWidget(lDP2, 0, 2);
    gridInfo->addWidget(lcdDP2, 0, 3);
    gridInfo->addWidget(frame2, 0, 2, 1, 2);
    gridInfo->addWidget(lSensor, 0, 4);
    gridInfo->addWidget(lcdSensorTemp, 0, 5);
    gridInfo->addWidget(frame3, 0, 4, 1, 2);
    gridInfo->setSpacing(5);

    gbInfo->setLayout(gridInfo);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(lBaud, 1, 0);
    grid->addWidget(cbBaud, 1, 1);
    // пещаю логотип фирмы
    grid->addWidget(new QLabel("<img src=':/Resources/elisat.png' height='40' width='150'/>", this), 0, 3, 2, 4, Qt::AlignRight);
    grid->addWidget(bPortStart, 2, 1);
    grid->addWidget(bPortStop, 2, 2);
    grid->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding), 2, 3);
    grid->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding), 2, 4);
    grid->addWidget(lTx, 2, 5, 1, 1, Qt::AlignRight);
    grid->addWidget(lRx, 2, 6, 1, 1, Qt::AlignRight);
    grid->setSpacing(5);

    setLayout(new QGridLayout(this));
    layout()->addItem(grid);
    layout()->addWidget(gbConfig);
    layout()->addWidget(gbInfo);
    layout()->setSpacing(5);

    // делает окно фиксированного размера
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    QStringList portsNames;

    foreach(QSerialPortInfo portsAvailable, QSerialPortInfo::availablePorts())
    {
        portsNames << portsAvailable.portName();
    }

    cbPort->addItems(portsNames);
#if defined (Q_OS_LINUX)
    cbPort->setEditable(true); // TODO Make correct viewing available ports in Linux
#else
    cbPort->setEditable(false);
#endif

    QStringList portsBauds;
    portsBauds << "115200" << "57600" << "38400";
    cbBaud->addItems(portsBauds);
    cbBaud->setEditable(false);
    bPortStop->setEnabled(false);

    itsBlinkTimeTxNone->setInterval(BLINKTIMETX);
    itsBlinkTimeRxNone->setInterval(BLINKTIMERX);
    itsBlinkTimeTxColor->setInterval(BLINKTIMETX);
    itsBlinkTimeRxColor->setInterval(BLINKTIMERX);
    itsTimeToDisplay->setInterval(DISPLAYTIME);

    sbSetDP1->setRange(DPRANGE_MIN, DPRANGE_MAX/*, DPRANGE_STEP*/);
    sbSetDP1->setAlignment(Qt::AlignCenter);
    sbSetDP2->setRange(DPRANGE_MIN, DPRANGE_MAX/*, DPRANGE_STEP*/);
    sbSetDP2->setAlignment(Qt::AlignCenter);
    sbSetTemp->setRange(TEMPRANGE_MIN, TEMPRANGE_MAX/*, TEMPRANGE_STEP*/);
    sbSetTemp->setAlignment(Qt::AlignCenter);
    sbSetTemp->setValue(NORMAL_TEMP);

    QList<QLCDNumber*> list;
    list << lcdDP1 << lcdDP2 << lcdSensorTemp;
//         << dynamic_cast<QLCDNumber*>(sbSetDP1->spinWidget())
//         << dynamic_cast<QLCDNumber*>(sbSetDP2->spinWidget())
//         << dynamic_cast<QLCDNumber*>(sbSetTemp->spinWidget());
    foreach(QLCDNumber *lcd, list) {
        lcd->setMinimumSize(80, 25);
        lcd->setMaximumSize(80, 25);
        lcd->setSegmentStyle(QLCDNumber::Flat);
        lcd->setFrameStyle(QFrame::NoFrame);
        lcd->display("----");
    }

    colorSetTempLCD();

    lcdDP1->setDigitCount(DP1_DIGITS);
    lcdDP2->setDigitCount(DP2_DIGITS);
    lcdSensorTemp->setDigitCount(TEMP_DIGITS);
    lcdSensorTemp->display("---.--");

    connect(bPortStart, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(bPortStop, SIGNAL(clicked()), this, SLOT(closePort()));

    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));
    connect(cbBaud, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));

    connect(itsProtocol, SIGNAL(DataIsReaded(bool)), this, SLOT(received(bool)));

    connect(itsBlinkTimeTxColor, SIGNAL(timeout()), this, SLOT(colorIsTx()));
    connect(itsBlinkTimeRxColor, SIGNAL(timeout()), this, SLOT(colorIsRx()));
    connect(itsBlinkTimeTxNone, SIGNAL(timeout()), this, SLOT(colorTxNone()));
    connect(itsBlinkTimeRxNone, SIGNAL(timeout()), this, SLOT(colorRxNone()));

    connect(itsTimeToDisplay, SIGNAL(timeout()), this, SLOT(displayTemp()));
    connect(itsTimeToDisplay, SIGNAL(timeout()), this, SLOT(displayDP()));

    connect(bSetTemp, SIGNAL(clicked()), this, SLOT(writeTemp()));
    connect(bSetDP1, SIGNAL(clicked()), this, SLOT(writeDP1()));
    connect(bSetDP2, SIGNAL(clicked()), this, SLOT(writeDP2()));

//    connect(sbSetTemp, SIGNAL(valueChanged()), this, SLOT(colorSetTempLCD()));

    connect(bWrite, SIGNAL(clicked()), this, SLOT(writePermanently()));
    connect(bCalibr, SIGNAL(clicked()), this, SLOT(calibrate()));

    QShortcut *aboutShortcut = new QShortcut(QKeySequence("F1"), this);
    connect(aboutShortcut, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

Dialog::~Dialog()
{
    itsPort->close();
}

void Dialog::openPort()
{
    itsPort->close();
    itsPort->setPortName(cbPort->currentText());

    if(itsPort->open(QSerialPort::ReadWrite))
    {
        switch (cbBaud->currentIndex()) {
        case 0:
            itsPort->setBaudRate(QSerialPort::Baud115200);
            break;
        case 1:
            itsPort->setBaudRate(QSerialPort::Baud57600);
            break;
        case 2:
            itsPort->setBaudRate(QSerialPort::Baud38400);
            break;
        default:
            itsPort->setBaudRate(QSerialPort::Baud115200);
            break;
        }

        itsPort->setDataBits(QSerialPort::Data8);
        itsPort->setParity(QSerialPort::NoParity);
        itsPort->setFlowControl(QSerialPort::NoFlowControl);

        bPortStart->setEnabled(false);
        bPortStop->setEnabled(true);
        lTx->setStyleSheet("background: none; font: bold; font-size: 10pt");
        lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    }
    else
    {
        lTx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
        lRx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    }
}

void Dialog::closePort()
{
    itsPort->close();
    itsBlinkTimeTxNone->stop();
    itsBlinkTimeTxColor->stop();
    itsBlinkTimeRxNone->stop();
    itsBlinkTimeRxColor->stop();
    lTx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    lRx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    bPortStop->setEnabled(false);
    bPortStart->setEnabled(true);
    itsProtocol->resetProtocol();

    m_isDP1Set = false;
    m_isDp2Set = false;
    m_isTempSet = false;
    lcdDP1->display("----");
    lcdDP2->display("----");
    lcdSensorTemp->display("---.--");

    QPalette palette;
    // get the palette
    palette = lcdSensorTemp->palette();

    // foreground color
    palette.setColor(palette.WindowText, QColor(0, 0, 0));
    // "light" border
    palette.setColor(palette.Light, QColor(0, 0, 0));
    // "dark" border
    palette.setColor(palette.Dark, QColor(0, 0, 0));

    // set the palette
    lcdSensorTemp->setPalette(palette);
}

void Dialog::received(bool isReceived)
{
    if(isReceived) {
        if(!itsBlinkTimeRxColor->isActive() && !itsBlinkTimeRxNone->isActive()) {
            itsBlinkTimeRxColor->start();
            lRx->setStyleSheet("background: green; font: bold; font-size: 10pt");
        }

        if(!itsTimeToDisplay->isActive()) {
            itsTimeToDisplay->start();
        }

        QList<QString> strKeysList = itsProtocol->getReadedData().keys();
        for(int i = 0; i < itsProtocol->getReadedData().size(); ++i) {
            if(strKeysList.at(i) == QString("TEMP")) {
                itsSensorsList.append(itsProtocol->getReadedData().value(strKeysList.at(i)));

            } else if(strKeysList.at(i) == QString("DP1")) {
                itsDP1 = itsProtocol->getReadedData().value(strKeysList.at(i));

            } else if(strKeysList.at(i) == QString("DP2")) {
                itsDP2 = itsProtocol->getReadedData().value(strKeysList.at(i));

            }
        }
    }
}

void Dialog::write(const Dialog::CODE &code)
{
    if(itsPort->isOpen()) {
        QMultiMap<QString, QString> dataTemp;

        if(!itsBlinkTimeTxColor->isActive() && !itsBlinkTimeTxNone->isActive()) {
            itsBlinkTimeTxColor->start();
            lTx->setStyleSheet("background: red; font: bold; font-size: 10pt");
        }

        QString codeStr;
        QString data;

        switch ( static_cast<int>(code) ) {
        case 0:
            codeStr = QString::number(CODE_WRITE);
            data = QString::number(NONE_DATA);
            break;
        case 1:
            codeStr = QString::number(CODE_DP1);
            data = QString::number(sbSetDP1->value());
            m_isDP1Set = true;
            break;
        case 2:
            codeStr = QString::number(CODE_DP2);
            data = QString::number(sbSetDP2->value());
            m_isDp2Set = true;
            break;
        case 3:
            codeStr = QString::number(CODE_TEMP);
            data = QString::number(sbSetTemp->value());
            m_isTempSet = true;
            break;
        case 4:
            codeStr = QString::number(CODE_CALIBR);
            data = QString::number(NONE_DATA);
            break;
        default:
            codeStr = QString::number(CODE_CALIBR);
            break;
        }
        dataTemp.insert("CODE", codeStr);
        dataTemp.insert("DATA", data);
        itsProtocol->setDataToWrite(dataTemp);
        itsProtocol->writeData();
    }
}

void Dialog::writeDP1()
{
    write(DP1);
}

void Dialog::writeDP2()
{
    write(DP2);
}

void Dialog::writeTemp()
{
    write(TEMP);
}

void Dialog::calibrate()
{
    write(CALIBR);
}

void Dialog::writePermanently()
{
    write(WRITE);
}

void Dialog::colorTxNone()
{
    itsBlinkTimeTxNone->stop();
}

void Dialog::setColorLCD(QLCDNumber *lcd, bool isHeat)
{
    QPalette palette;
    // get the palette
    palette = lcd->palette();
    if(isHeat) {
        // foreground color
        palette.setColor(palette.WindowText, QColor(100, 0, 0));
        // "light" border
        palette.setColor(palette.Light, QColor(100, 0, 0));
        // "dark" border
        palette.setColor(palette.Dark, QColor(100, 0, 0));
    } else {
        // foreground color
        palette.setColor(palette.WindowText, QColor(0, 0, 100));
        // "light" border
        palette.setColor(palette.Light, QColor(0, 0, 100));
        // "dark" border
        palette.setColor(palette.Dark, QColor(0, 0, 100));
    }
    // set the palette
    lcd->setPalette(palette);
}

QString &Dialog::addTrailingZeros(QString &str, int prec)
{
    if(str.isEmpty() || prec < 1) { // if prec == 0 then it's no sense
        return str;
    }

    int pointIndex = str.indexOf(".");
    if(pointIndex == -1) {
        str.append(".");
        pointIndex = str.size() - 1;
    }

    if(str.size() - 1 - pointIndex < prec) {
        int size = str.size();
        for(int i = 0; i < prec - (size - 1 - pointIndex); ++i) {
            str.append("0");
        }
    }

    return str;
}

void Dialog::colorIsRx()
{
    lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    itsBlinkTimeRxColor->stop();
    itsBlinkTimeRxNone->start();
}

void Dialog::colorRxNone()
{
    itsBlinkTimeRxNone->stop();
}

void Dialog::colorIsTx()
{
    lTx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    itsBlinkTimeTxColor->stop();
    itsBlinkTimeTxNone->start();
}

void Dialog::displayTemp()
{
#ifdef DEBUG
    qDebug() << "void Dialog::displayTemp()";
#endif
    itsTimeToDisplay->stop();

    if(!m_isTempSet) {
        return;
    }

    QList<QLCDNumber*> list;
    list << lcdSensorTemp;
    QString tempStr;
#ifdef DEBUG
        qDebug() << "itsSensorsList.size() =" << itsSensorsList.size();
#endif
    for(int k = 0; k < list.size() && k < itsSensorsList.size(); ++k) {
        tempStr = itsSensorsList.at(itsSensorsList.size() - 1 - k);

        if(list.at(k)->digitCount() < addTrailingZeros(tempStr, PRECISION).size())
        {
            list[k]->display("ERR"); // Overflow
        } else {
            list[k]->display(addTrailingZeros(tempStr, PRECISION));
        }

        setColorLCD(list[k], tempStr.toDouble() > 0.0);
#ifdef DEBUG
        qDebug() << "itsSensorsList.size() =" << itsSensorsList.size();
        qDebug() << "Temperature[" << k << "] =" << list.at(k)->value();
#endif
    }

    itsSensorsList.clear();
}

void Dialog::displayDP()
{
#ifdef DEBUG
    qDebug() << "void Dialog::displayDP()";
#endif
    itsTimeToDisplay->stop();

    QString tempStr;
    QStringList DPList;

    QList<QLCDNumber*> list;

    if(m_isDP1Set) {
        list << lcdDP1;
        DPList << itsDP1;
    }
    if(m_isDp2Set) {
        list << lcdDP2;
        DPList  << itsDP2;
    }
    if(!m_isDP1Set && !m_isDp2Set) {
        return;
    }

#ifdef DEBUG
    qDebug() << "itsDPList.size() =" << DPList.size();
#endif
    for(int k = 0; k < list.size() && k < DPList.size(); ++k) {
        tempStr = DPList.at(k);

        if(list.at(k)->digitCount() < tempStr.size())
        {
            list[k]->display("ERR"); // Overflow
        } else {
            list[k]->display(tempStr);
        }
#ifdef DEBUG
        qDebug() << "DPList.size() =" << DPList.size();
        qDebug() << "DP[" << k << "] =" << list.at(k)->value();
#endif
    }
//    for(int k = 0; k < list.size() && k < DPList.size(); ++k) {
//        tempStr = DPList.at(DPList.size() - 1 - k);

//        if(list.at(k)->digitCount() < tempStr.size())
//        {
//            list[k]->display("ERR"); // Overflow
//        } else {
//            list[k]->display(tempStr);
//        }
//#ifdef DEBUG
//        qDebug() << "DPList.size() =" << DPList.size();
//        qDebug() << "DP[" << k << "] =" << list.at(k)->value();
//#endif
//    }
}

void Dialog::colorSetTempLCD()
{
//    setColorLCD(dynamic_cast<QLCDNumber*>(sbSetTemp->spinWidget()), sbSetTemp->value() > 0);
}
