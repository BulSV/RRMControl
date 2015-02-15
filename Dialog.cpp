#include "Dialog.h"
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

#define NEGATIVE 32768 // 2^15
#define OFFSET 65536 // 2^16
#define SLOPE 128

#define CPU_FACTOR 0.537
#define CPU_OFFSET 900
#define CPU_SLOPE 2.95

#define ACCURACY 0.02

#define FORMAT 'f'
#define PRECISION 2

#define BLINKTIMETX 200 // ms
#define BLINKTIMERX 500 // ms
#define DISPLAYTIME 100 // ms

#define REWINDTIME 50 // ms

#define TEMPRANGE_MIN -50 // degrees Celsius
#define TEMPRANGE_MAX 50 // degrees Celsius
#define NORMAL_TEMP 35 // degrees Celsius

#define DPRANGE_MIN 0
#define DPRANGE_MAX 1024

#define CODE_TEMP "0"
#define CODE_DP1 "1"
#define CODE_DP2 "2"
#define CODE_WRITE "3"

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
        sbSetDP1(new SpinBox(QIcon(":/Resources/left.png"), QIcon(":/Resources/right.png"),
                              QString::fromUtf8(""), QString::fromUtf8(""), REWINDTIME, this)),
        sbSetDP2(new SpinBox(QIcon(":/Resources/left.png"), QIcon(":/Resources/right.png"),
                              QString::fromUtf8(""), QString::fromUtf8(""), REWINDTIME, this)),
        sbSetTemp(new SpinBox(QIcon(":/Resources/left.png"), QIcon(":/Resources/right.png"),
                              QString::fromUtf8(""), QString::fromUtf8(""), REWINDTIME, this)),
        lcdDP1(new QLCDNumber(this)),
        lcdDP2(new QLCDNumber(this)),
        lcdSensorTemp(new QLCDNumber(this)),
        lDP1(new QLabel(QString::fromUtf8("DP 1:"), this)),
        lDP2(new QLabel(QString::fromUtf8("DP 2:"), this)),
        lSensor(new QLabel(QString::fromUtf8("Sensor, °C:"), this)),
        bSetDP1(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetDP2(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetTemp(new QPushButton(QString::fromUtf8("Set"), this)),
        gbSetDP1(new QGroupBox(QString::fromUtf8("DP 1"), this)),
        gbSetDP2(new QGroupBox(QString::fromUtf8("DP 2"), this)),
        gbSetTemp(new QGroupBox(QString::fromUtf8("Temperature, °C"), this)),
        gbConfig(new QGroupBox(QString::fromUtf8("Configure"), this)),
        gbInfo(new QGroupBox(QString::fromUtf8("Information"), this)),
        bWrite(new QPushButton(QString::fromUtf8("Write"), this)),
        itsPort(new QSerialPort(this)),
        itsComPort(new ComPort(itsPort, STARTBYTE, STOPBYTE, BYTESLENTH, this)),
        itsProtocol(new RRMProtocol(itsComPort, this)),
        itsStatusBar (new QStatusBar(this)),
        itsBlinkTimeTxNone(new QTimer(this)),
        itsBlinkTimeRxNone(new QTimer(this)),
        itsBlinkTimeTxColor(new QTimer(this)),
        itsBlinkTimeRxColor(new QTimer(this)),
        itsTimeToDisplay(new QTimer(this))
{
    setLayout(new QVBoxLayout(this));

    lTx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    lTx->setFrameStyle(QFrame::Box);
    lTx->setAlignment(Qt::AlignCenter);
    lTx->setMargin(2);

    lRx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    lRx->setFrameStyle(QFrame::Box);
    lRx->setAlignment(Qt::AlignCenter);
    lRx->setMargin(2);

    QGridLayout *gridDP1 = new QGridLayout;
    gridDP1->addWidget(sbSetDP1, 0, 0, 1, 2);
    gridDP1->addWidget(bSetDP1, 1, 0, 1, 2);

    QGridLayout *gridDP2 = new QGridLayout;
    gridDP2->addWidget(sbSetDP2, 0, 0, 1, 2);
    gridDP2->addWidget(bSetDP2, 1, 0, 1, 2);

    QGridLayout *gridTemp = new QGridLayout;
    gridTemp->addWidget(sbSetTemp, 0, 0, 1, 2);
    gridTemp->addWidget(bSetTemp, 1, 0, 1, 2);

    gbSetDP1->setLayout(gridDP1);
    gbSetDP2->setLayout(gridDP2);
    gbSetTemp->setLayout(gridTemp);

    QGridLayout *gridConfig = new QGridLayout;
    gridConfig->addWidget(gbSetDP1, 0, 0, 3, 2);
    gridConfig->addWidget(gbSetDP2, 0, 3, 3, 2);
    gridConfig->addWidget(gbSetTemp, 0, 5, 3, 2);
    gridConfig->addWidget(bWrite, 3, 5, 1, 2);

    gbConfig->setLayout(gridConfig);

    QFrame *frame1 = new QFrame(this);
    frame1->setFrameStyle(QFrame::Box | QFrame::Raised);
    lDP1->setMargin(5);

    QFrame *frame2 = new QFrame(this);
    frame2->setFrameStyle(QFrame::Box | QFrame::Raised);
    lDP2->setMargin(5);

    QFrame *frame3 = new QFrame(this);
    frame3->setFrameStyle(QFrame::Box | QFrame::Raised);
    lSensor->setMargin(5);

    QGridLayout *gridInfo = new QGridLayout;
    gridInfo->addWidget(lDP1, 0, 0);
    gridInfo->addWidget(lcdDP1, 0, 1);
    gridInfo->addWidget(frame1, 0, 0, 1, 2);
    gridInfo->addWidget(lDP2, 0, 3);
    gridInfo->addWidget(lcdDP2, 0, 4);
    gridInfo->addWidget(frame2, 0, 3, 1, 2);
    gridInfo->addWidget(lSensor, 0, 6);
    gridInfo->addWidget(lcdSensorTemp, 0, 7);
    gridInfo->addWidget(frame3, 0, 6, 1, 2);
    gridInfo->setSpacing(5);

    gbInfo->setLayout(gridInfo);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(lBaud, 1, 0);
    grid->addWidget(cbBaud, 1, 1);
    // пещаю логотип фирмы
    grid->addWidget(new QLabel("<img src=':/Resources/elisat.png' height='40' width='150'/>", this), 0, 3, 2, 5, Qt::AlignRight);
    grid->addWidget(bPortStart, 2, 1);
    grid->addWidget(bPortStop, 2, 2);
    grid->addWidget(lTx, 2, 6, Qt::AlignRight);
    grid->addWidget(lRx, 2, 7, Qt::AlignRight);
    grid->setSpacing(5);

    layout()->addItem(grid);
    layout()->addWidget(gbConfig);
    layout()->addWidget(gbInfo);
    layout()->addWidget(itsStatusBar);
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

    itsStatusBar->show();

    itsBlinkTimeTxNone->setInterval(BLINKTIMETX);
    itsBlinkTimeRxNone->setInterval(BLINKTIMERX);
    itsBlinkTimeTxColor->setInterval(BLINKTIMETX);
    itsBlinkTimeRxColor->setInterval(BLINKTIMERX);
    itsTimeToDisplay->setInterval(DISPLAYTIME);

    sbSetDP1->setRange(DPRANGE_MIN, DPRANGE_MAX);
    sbSetTemp->setRange(TEMPRANGE_MIN, TEMPRANGE_MAX);
    sbSetTemp->setValue(NORMAL_TEMP);

    QList<QLCDNumber*> list;
    list << lcdDP1 << lcdDP2 << lcdSensorTemp;
    foreach(QLCDNumber *lcd, list) {
        lcd->setMinimumSize(80, 25);
        lcd->setMaximumSize(80, 25);
        lcd->setSegmentStyle(QLCDNumber::Flat);
        lcd->setFrameStyle(QFrame::NoFrame);
    }

    lcdDP1->setDigitCount(3);
    lcdDP2->setDigitCount(3);
    lcdSensorTemp->setDigitCount(6);

    connect(bPortStart, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(bPortStop, SIGNAL(clicked()), this, SLOT(closePort()));
    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));
    connect(cbBaud, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));
    connect(itsProtocol, SIGNAL(DataIsReaded(bool)), this, SLOT(received(bool)));
    connect(bSetTemp, SIGNAL(clicked()), this, SLOT(writeTemp()));
    connect(itsBlinkTimeTxColor, SIGNAL(timeout()), this, SLOT(colorIsTx()));
    connect(itsBlinkTimeRxColor, SIGNAL(timeout()), this, SLOT(colorIsRx()));
    connect(itsBlinkTimeTxNone, SIGNAL(timeout()), this, SLOT(colorTxNone()));
    connect(itsBlinkTimeRxNone, SIGNAL(timeout()), this, SLOT(colorRxNone()));
    connect(itsTimeToDisplay, SIGNAL(timeout()), this, SLOT(displayTemp()));
    connect(itsTimeToDisplay, SIGNAL(timeout()), this, SLOT(displayDP()));

    connect(sbSetTemp, SIGNAL(downButtonReleased()), this, SLOT(writeTemp()));
    connect(sbSetTemp, SIGNAL(upButtonReleased()), this, SLOT(writeTemp()));

    connect(sbSetDP1, SIGNAL(downButtonReleased()), this, SLOT(writeDP1()));
    connect(sbSetDP1, SIGNAL(upButtonReleased()), this, SLOT(writeDP2()));

    connect(sbSetDP2, SIGNAL(downButtonReleased()), this, SLOT(writeDP2()));
    connect(sbSetDP2, SIGNAL(upButtonReleased()), this, SLOT(writeDP2()));

    connect(bWrite, SIGNAL(clicked()), this, SLOT(writePermanently()));

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

        itsStatusBar->showMessage(QString::fromUtf8("Port: ") +
                             QString(itsPort->portName()) +
                             QString::fromUtf8(" | Baud: ") +
                             QString(QString::number(itsPort->baudRate())) +
                             QString::fromUtf8(" | Data bits: ") +
                             QString(QString::number(itsPort->dataBits())));
        bPortStart->setEnabled(false);
        bPortStop->setEnabled(true);
        lTx->setStyleSheet("background: none; font: bold; font-size: 10pt");
        lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    }
    else
    {
        itsStatusBar->showMessage(QString::fromUtf8("Error opening port: ") +
                             QString(itsPort->portName()));
        lTx->setStyleSheet("background: red; font: bold; font-size: 10pt");
        lRx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    }
}

void Dialog::closePort()
{
    itsPort->close();
    itsBlinkTimeTxNone->stop();
    itsBlinkTimeTxColor->stop();
    itsBlinkTimeRxNone->stop();
    itsBlinkTimeRxColor->stop();
    lTx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    lRx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    bPortStop->setEnabled(false);
    bPortStart->setEnabled(true);
    itsProtocol->resetProtocol();
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
            if(strKeysList.at(i) == "CODE"
                    && itsProtocol->getReadedData().value(strKeysList.at(i)) == CODE_TEMP) {
                itsSensorsList.append(itsProtocol->getReadedData().value(strKeysList.at(i)));
            } else if(strKeysList.at(i) == "CODE"
                      && ((itsProtocol->getReadedData().value(strKeysList.at(i)) == CODE_DP1)
                          || (itsProtocol->getReadedData().value(strKeysList.at(i)) == CODE_DP2))) {
                itsDPList.append(itsProtocol->getReadedData().value(strKeysList.at(i)));
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
            lTx->setStyleSheet("background: green; font: bold; font-size: 10pt");
        }

        QString codeStr;

        switch (code) {
        case 0:
            codeStr = CODE_TEMP;
            break;
        case 1:
            codeStr = CODE_DP1;
            break;
        case 2:
            codeStr = CODE_DP2;
            break;
        case 3:
            codeStr = CODE_WRITE;
            break;
        default:
            codeStr = CODE_TEMP;
            break;
        }
        dataTemp.insert("CODE", codeStr);
        dataTemp.insert("DATA", QString::number(sbSetTemp->value()));
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
        for(int i = 0; i < prec - (str.size() - 1 - pointIndex); ++i) {
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
    itsTimeToDisplay->stop();

    QList<QLCDNumber*> list;
    list << lcdSensorTemp;
    QString tempStr;

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
    itsTimeToDisplay->stop();

    QList<QLCDNumber*> list;
    list << lcdDP1 << lcdDP2;
    QString tempStr;

    for(int k = 0; k < list.size() && k < itsDPList.size(); ++k) {
        tempStr = itsDPList.at(itsDPList.size() - 1 - k);

        if(list.at(k)->digitCount() < tempStr.size())
        {
            list[k]->display("ERR"); // Overflow
        } else {
            list[k]->display(tempStr);
        }
#ifdef DEBUG
        qDebug() << "itsDPList.size() =" << itsDPList.size();
        qDebug() << "DP[" << k << "] =" << list.at(k)->value();
#endif
    }

    itsDPList.clear();
}
