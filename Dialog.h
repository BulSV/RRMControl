#ifndef DIALOG_H
#define DIALOG_H

#ifdef DEBUG
#include <QDebug>
#endif

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QStatusBar>
#include <QTimer>
#include <QLCDNumber>
#include <QSpinBox>
#include "ComPort.h"
#include "IProtocol.h"
#include "RRMProtocol.h"
#include "SpinBox.h"

class Dialog : public QDialog
{
    Q_OBJECT

    enum CODE {
        TEMP,
        DP1,
        DP2,
        WRITE
    };

    QLabel *lPort;
    QComboBox *cbPort;
    QLabel *lBaud;
    QComboBox *cbBaud;
    QPushButton *bPortStart;
    QPushButton *bPortStop;
    QLabel *lTx;
    QLabel *lRx;

    SpinBox *sbSetDP1;
    SpinBox *sbSetDP2;
    SpinBox *sbSetTemp;

    QLCDNumber *lcdDP1;
    QLCDNumber *lcdDP2;
    QLCDNumber *lcdSensorTemp;

    QLabel *lDP1;
    QLabel *lDP2;
    QLabel *lSensor;

    QPushButton *bSetDP1;
    QPushButton *bSetDP2;
    QPushButton *bSetTemp;

    QGroupBox *gbSetDP1;
    QGroupBox *gbSetDP2;
    QGroupBox *gbSetTemp;
    QGroupBox *gbConfig;
    QGroupBox *gbInfo;

    QPushButton *bWrite;

    QSerialPort *itsPort;
    ComPort *itsComPort;
    IProtocol *itsProtocol;

    QStringList itsSensorsList;
    QString itsDP1;
    QString itsDP2;

    // цвет индикации температуры >0 & <=0
    void setColorLCD(QLCDNumber *lcd, bool isHeat);
    // добавляет завершающие нули
    QString &addTrailingZeros(QString &str, int prec);

    void write(const CODE &code);

    QStatusBar *itsStatusBar;
    QTimer *itsBlinkTimeTxNone;
    QTimer *itsBlinkTimeRxNone;
    QTimer *itsBlinkTimeTxColor;
    QTimer *itsBlinkTimeRxColor;
    QTimer *itsTimeToDisplay;

private slots:
    void openPort();
    void closePort();
    void received(bool isReceived);
    void writeDP1();
    void writeDP2();
    void writeTemp();
    void writePermanently();
    // мигание надписей "Rx" - при получении и "Tx" - при отправке пакета
    void colorTxNone();
    void colorRxNone();
    void colorIsTx();
    void colorIsRx();
    // display current Rx data
    void displayTemp();
    void displayDP();
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
};

#endif // DIALOG_H
