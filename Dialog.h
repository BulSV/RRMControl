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
#include <QTimer>
#include <QLCDNumber>
#include <QSpinBox>
#include "ComPort.h"
#include "IProtocol.h"
#include "RRMProtocol.h"

class Dialog : public QDialog
{
    Q_OBJECT

    enum CODE {
        WRITE,
        OFFSET,
        GAIN,
        TEMP,
        CALIBR
    };

    QLabel *lPort;
    QComboBox *cbPort;
    QLabel *lBaud;
    QComboBox *cbBaud;
    QPushButton *bPortStart;
    QPushButton *bPortStop;
    QLabel *lTx;
    QLabel *lRx;

    QSpinBox *sbSetOffset;
    QSpinBox *sbSetGain;
    QSpinBox *sbSetTemp;

    QLCDNumber *m_lcdOffset;
    QLCDNumber *m_lcdGain;
    QLCDNumber *m_lcdTemp;

    QLabel *lOffset;
    QLabel *lGain;
    QLabel *lTemp;

    QPushButton *bSetOffset;
    QPushButton *bSetGain;
    QPushButton *bSetTemp;

    QGroupBox *gbSetOffset;
    QGroupBox *gbSetGain;
    QGroupBox *gbSetTemp;
    QGroupBox *gbConfig;
    QGroupBox *gbInfo;

    QPushButton *bWrite;
    QPushButton *bCalibr;

    QSerialPort *m_Port;
    ComPort *m_ComPort;
    IProtocol *m_Protocol;

    QMap<QString, QString> m_DisplayList;

    // Color of indication of temperature >0 & <=0
    void setColorLCD(QLCDNumber *lcd, bool isHeat);
    // Reset QLCDNumber color to default
    void defaultColorLCD(QLCDNumber *lcd);
    // Add trailing zeros
    QString &addTrailingZeros(QString &str, int prec);

    void write(const CODE &code);

    QTimer *m_BlinkTimeTxNone;
    QTimer *m_BlinkTimeRxNone;
    QTimer *m_BlinkTimeTxColor;
    QTimer *m_BlinkTimeRxColor;
    QTimer *m_TimeToDisplay;

    QMap<QString, bool> m_isDataSet;
    void initIsDataSet();

private slots:
    void openPort();
    void closePort();
    void received(bool isReceived);
    void writeOffset();
    void writeGain();
    void writeTemp();
    void calibrate();
    void writePermanently();
    // Blinking labels "Rx" - at receiving and "Tx" - at sending packet
    void colorTxNone();
    void colorRxNone();
    void colorIsTx();
    void colorIsRx();
    // display current Rx data
    void displayData();
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
};

#endif // DIALOG_H
