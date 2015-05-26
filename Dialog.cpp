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

#define FORMAT 'f'
#define PRECISION 2

#define BLINKTIMETX 200 // ms
#define BLINKTIMERX 500 // ms
#define DISPLAYTIME 100 // ms

#define TEMPRANGE_MIN -50 // degrees Celsius
#define TEMPRANGE_MAX 50 // degrees Celsius
#define NORMAL_TEMP 35 // degrees Celsius

#define DPRANGE_MIN 0
#define DPRANGE_MAX 1024

#define OFFSET_DIGITS 4
#define GAIN_DIGITS 4
#define TEMP_DIGITS 6

#define CODE_WRITE 0x00
#define CODE_OFFSET 0x01
#define CODE_GAIN 0x02
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
        sbSetOffset(new QSpinBox(this)),
        sbSetGain(new QSpinBox(this)),
        sbSetTemp(new QSpinBox(this)),
        m_lcdOffset(new QLCDNumber(this)),
        m_lcdGain(new QLCDNumber(this)),
        m_lcdTemp(new QLCDNumber(this)),
        lOffset(new QLabel(QString::fromUtf8("Offset:"), this)),
        lGain(new QLabel(QString::fromUtf8("Sensor1, °C:"), this)),
        lTemp(new QLabel(QString::fromUtf8("Sensor2, °C:"), this)),
        bSetOffset(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetGain(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetTemp(new QPushButton(QString::fromUtf8("Set"), this)),
        gbSetOffset(new QGroupBox(QString::fromUtf8("Offset"), this)),
        gbSetGain(new QGroupBox(QString::fromUtf8("Gain"), this)),
        gbSetTemp(new QGroupBox(QString::fromUtf8("Temperature, °C"), this)),
        gbConfig(new QGroupBox(QString::fromUtf8("Configure"), this)),
        gbInfo(new QGroupBox(QString::fromUtf8("Information"), this)),
        bWrite(new QPushButton(QString::fromUtf8("Write"), this)),
        bCalibr(new QPushButton(QString::fromUtf8("Calibrate"), this)),
        m_Port(new QSerialPort(this)),
        m_ComPort(new ComPort(m_Port, STARTBYTE, STOPBYTE, BYTESLENTH, true, this)),
        m_Protocol(new RRMProtocol(m_ComPort, this)),
        m_BlinkTimeTxNone(new QTimer(this)),
        m_BlinkTimeRxNone(new QTimer(this)),
        m_BlinkTimeTxColor(new QTimer(this)),
        m_BlinkTimeRxColor(new QTimer(this)),
        m_TimeToDisplay(new QTimer(this))
{
    initIsDataSet();

    bSetOffset->setMaximumSize(60, 30);
    bSetOffset->setMinimumSize(60, 30);

    bSetGain->setMaximumSize(60, 30);
    bSetGain->setMinimumSize(60, 30);

    bSetTemp->setMaximumSize(60, 30);
    bSetTemp->setMinimumSize(60, 30);

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

    QGridLayout *gridOffset = new QGridLayout;
    gridOffset->addWidget(sbSetOffset, 0, 0);
    gridOffset->addWidget(bSetOffset, 1, 0);

    QGridLayout *gridGain = new QGridLayout;
    gridGain->addWidget(sbSetGain, 0, 0);
    gridGain->addWidget(bSetGain, 1, 0);

    QGridLayout *gridTemp = new QGridLayout;
    gridTemp->addWidget(sbSetTemp, 0, 0);
    gridTemp->addWidget(bSetTemp, 1, 0);

    gbSetOffset->setLayout(gridOffset);
    gbSetGain->setLayout(gridGain);
    gbSetTemp->setLayout(gridTemp);

    QGridLayout *gridConfig = new QGridLayout;
    gridConfig->addWidget(gbSetOffset, 0, 0, 1, 2);
    gridConfig->addWidget(gbSetGain, 0, 2, 1, 2);
    gridConfig->addWidget(gbSetTemp, 0, 4, 1, 2);
    gridConfig->addWidget(bCalibr, 1, 2, 1, 2);
    gridConfig->addWidget(bWrite, 1, 4, 1, 2);

    gbConfig->setLayout(gridConfig);

    QFrame *frame1 = new QFrame(this);
    frame1->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lOffset->setMargin(5);

    QFrame *frame2 = new QFrame(this);
    frame2->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lGain->setMargin(5);

    QFrame *frame3 = new QFrame(this);
    frame3->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lTemp->setMargin(5);

    QGridLayout *gridInfo = new QGridLayout;
    gridInfo->addWidget(lOffset, 0, 0);
    gridInfo->addWidget(m_lcdOffset, 0, 1);
    gridInfo->addWidget(frame1, 0, 0, 1, 2);
    gridInfo->addWidget(lGain, 0, 2);
    gridInfo->addWidget(m_lcdGain, 0, 3);
    gridInfo->addWidget(frame2, 0, 2, 1, 2);
    gridInfo->addWidget(lTemp, 0, 4);
    gridInfo->addWidget(m_lcdTemp, 0, 5);
    gridInfo->addWidget(frame3, 0, 4, 1, 2);
    gridInfo->setSpacing(5);

    gbInfo->setLayout(gridInfo);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(lBaud, 1, 0);
    grid->addWidget(cbBaud, 1, 1);
    // Inserting company logo
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

    // made window of app fixed size
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    QStringList portsNames;

    foreach(QSerialPortInfo portsAvailable, QSerialPortInfo::availablePorts())
    {
        portsNames << portsAvailable.portName();
    }

    cbPort->addItems(portsNames);
#if defined (Q_OS_LINUX)
    cbPort->setEditable(true); // TODO Make correct viewing available virtual ports in Linux
#else
    cbPort->setEditable(false);
#endif

    QStringList portsBauds;
    portsBauds << "115200" << "57600" << "38400";
    cbBaud->addItems(portsBauds);
    cbBaud->setEditable(false);
    bPortStop->setEnabled(false);

    m_BlinkTimeTxNone->setInterval(BLINKTIMETX);
    m_BlinkTimeRxNone->setInterval(BLINKTIMERX);
    m_BlinkTimeTxColor->setInterval(BLINKTIMETX);
    m_BlinkTimeRxColor->setInterval(BLINKTIMERX);
    m_TimeToDisplay->setInterval(DISPLAYTIME);

    sbSetOffset->setRange(DPRANGE_MIN, DPRANGE_MAX);
    sbSetOffset->setAlignment(Qt::AlignCenter);
    sbSetGain->setRange(DPRANGE_MIN, DPRANGE_MAX);
    sbSetGain->setAlignment(Qt::AlignCenter);
    sbSetTemp->setRange(TEMPRANGE_MIN, TEMPRANGE_MAX);
    sbSetTemp->setAlignment(Qt::AlignCenter);
    sbSetTemp->setValue(NORMAL_TEMP);

    QList<QLCDNumber*> list;
    list << m_lcdOffset << m_lcdGain << m_lcdTemp;
    foreach(QLCDNumber *lcd, list) {
        lcd->setMinimumSize(80, 25);
        lcd->setMaximumSize(80, 25);
        lcd->setSegmentStyle(QLCDNumber::Flat);
        lcd->setFrameStyle(QFrame::NoFrame);
        lcd->display("----");
    }

    m_lcdOffset->setDigitCount(OFFSET_DIGITS);
    m_lcdGain->setDigitCount(TEMP_DIGITS);
    m_lcdGain->display("---.--");
    m_lcdTemp->setDigitCount(TEMP_DIGITS);
    m_lcdTemp->display("---.--");

    connect(bPortStart, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(bPortStop, SIGNAL(clicked()), this, SLOT(closePort()));

    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));
    connect(cbBaud, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));

    connect(m_Protocol, SIGNAL(DataIsReaded(bool)), this, SLOT(received(bool)));

    connect(m_BlinkTimeTxColor, SIGNAL(timeout()), this, SLOT(colorIsTx()));
    connect(m_BlinkTimeRxColor, SIGNAL(timeout()), this, SLOT(colorIsRx()));
    connect(m_BlinkTimeTxNone, SIGNAL(timeout()), this, SLOT(colorTxNone()));
    connect(m_BlinkTimeRxNone, SIGNAL(timeout()), this, SLOT(colorRxNone()));

    connect(m_TimeToDisplay, SIGNAL(timeout()), this, SLOT(displayData()));

    connect(bSetTemp, SIGNAL(clicked()), this, SLOT(writeTemp()));
    connect(bSetOffset, SIGNAL(clicked()), this, SLOT(writeOffset()));
    connect(bSetGain, SIGNAL(clicked()), this, SLOT(writeGain()));

    connect(bWrite, SIGNAL(clicked()), this, SLOT(writePermanently()));
    connect(bCalibr, SIGNAL(clicked()), this, SLOT(calibrate()));

    QShortcut *aboutShortcut = new QShortcut(QKeySequence("F1"), this);
    connect(aboutShortcut, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

Dialog::~Dialog()
{
    m_Port->close();
}

void Dialog::openPort()
{
    m_Port->close();
    m_Port->setPortName(cbPort->currentText());

    if(m_Port->open(QSerialPort::ReadWrite))
    {
        switch (cbBaud->currentIndex()) {
        case 0:
            m_Port->setBaudRate(QSerialPort::Baud115200);
            break;
        case 1:
            m_Port->setBaudRate(QSerialPort::Baud57600);
            break;
        case 2:
            m_Port->setBaudRate(QSerialPort::Baud38400);
            break;
        default:
            m_Port->setBaudRate(QSerialPort::Baud115200);
            break;
        }

        m_Port->setDataBits(QSerialPort::Data8);
        m_Port->setParity(QSerialPort::NoParity);
        m_Port->setFlowControl(QSerialPort::NoFlowControl);

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

void Dialog::defaultColorLCD(QLCDNumber *lcd)
{
    QPalette palette;
    // get the palette
    palette = lcd->palette();

    // foreground color
    palette.setColor(palette.WindowText, QColor(0, 0, 0));
    // "light" border
    palette.setColor(palette.Light, QColor(0, 0, 0));
    // "dark" border
    palette.setColor(palette.Dark, QColor(0, 0, 0));

    // set the palette
    lcd->setPalette(palette);
}

void Dialog::closePort()
{
    m_Port->close();
    m_BlinkTimeTxNone->stop();
    m_BlinkTimeTxColor->stop();
    m_BlinkTimeRxNone->stop();
    m_BlinkTimeRxColor->stop();
    lTx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    lRx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    bPortStop->setEnabled(false);
    bPortStart->setEnabled(true);
    m_Protocol->resetProtocol();

    initIsDataSet();
    m_lcdOffset->display("----");
    m_lcdGain->display("---.--");
    m_lcdTemp->display("---.--");

    QList<QLCDNumber*> lcdList = QList<QLCDNumber*>() << m_lcdOffset << m_lcdGain << m_lcdTemp;
    foreach (QLCDNumber *lcd, lcdList) {
        defaultColorLCD(lcd);
    }
}

void Dialog::received(bool isReceived)
{
    if(isReceived) {
        if(!m_BlinkTimeRxColor->isActive() && !m_BlinkTimeRxNone->isActive()) {
            m_BlinkTimeRxColor->start();
            lRx->setStyleSheet("background: green; font: bold; font-size: 10pt");
        }

        if(!m_TimeToDisplay->isActive()) {
            m_TimeToDisplay->start();
        }

        m_DisplayList = m_Protocol->getReadedData();
    }
}

void Dialog::write(const Dialog::CODE &code)
{
    if(m_Port->isOpen()) {
        QMultiMap<QString, QString> dataTemp;

        if(!m_BlinkTimeTxColor->isActive() && !m_BlinkTimeTxNone->isActive()) {
            m_BlinkTimeTxColor->start();
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
            codeStr = QString::number(CODE_OFFSET);
            data = QString::number(sbSetOffset->value());
            m_isDataSet.insert("OFFSET", true);
            break;
        case 2:
            codeStr = QString::number(CODE_GAIN);
            data = QString::number(sbSetGain->value());
            m_isDataSet.insert("GAIN", true);
            break;
        case 3:
            codeStr = QString::number(CODE_TEMP);
            data = QString::number(sbSetTemp->value());
            m_isDataSet.insert("SENS1", true);
            m_isDataSet.insert("SENS2", true);
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
        m_Protocol->setDataToWrite(dataTemp);
        m_Protocol->writeData();
    }
}

void Dialog::writeOffset()
{
    write(OFFSET);
}

void Dialog::writeGain()
{
    write(GAIN);
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
    m_BlinkTimeTxNone->stop();
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
    m_BlinkTimeRxColor->stop();
    m_BlinkTimeRxNone->start();
}

void Dialog::colorRxNone()
{
    m_BlinkTimeRxNone->stop();
}

void Dialog::colorIsTx()
{
    lTx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    m_BlinkTimeTxColor->stop();
    m_BlinkTimeTxNone->start();
}

void Dialog::displayData()
{
    m_TimeToDisplay->stop();

    QMap<QString, QLCDNumber*> list;
    QString tempStr;

    if(!m_isDataSet.value("OFFSET")
            && !m_isDataSet.value("SENS1")
            && !m_isDataSet.value("SENS2")) {
        return;
    }
    if(m_isDataSet.value("OFFSET")) {
        list.insert("OFFSET", m_lcdOffset);
    }
    if(m_isDataSet.value("SENS1")) {
        list.insert("SENS1", m_lcdGain);
    }
    if(m_isDataSet.value("SENS2")) {
        list.insert("SENS2", m_lcdTemp);
    }

    foreach (QString key, m_DisplayList.keys()) {
        if(list.contains(key)) {
            tempStr = m_DisplayList.value(key);

            if(key != "SENS1" && key != "SENS2") {
#undef PRECISION
#define PRECISION 0
            } else {
                setColorLCD(list.value(key), tempStr.toDouble() > 0.0);
            }

            if(list.value(key)->digitCount() < addTrailingZeros(tempStr, PRECISION).size())
            {
                list[key]->display("ERR"); // Overflow
            } else {
                list[key]->display(addTrailingZeros(tempStr, PRECISION));
            }
        }
    }

    m_DisplayList.clear();
}

void Dialog::initIsDataSet()
{
    m_isDataSet.insert("OFFSET", false);
    m_isDataSet.insert("SENS1", false);
    m_isDataSet.insert("SENS2", false);
}
