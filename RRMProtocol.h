#ifndef READSENSORPROTOCOL_H
#define READSENSORPROTOCOL_H

#include "IProtocol.h"
#include "ComPort.h"

#include <QTimer>
#include <QVector>

class RRMProtocol : public IProtocol
{
    Q_OBJECT
public:
    explicit RRMProtocol(ComPort *comPort, QObject *parent = 0);
    virtual void setDataToWrite(const QMultiMap<QString, QString> &data);
    virtual QMultiMap<QString, QString> getReadedData() const;
signals:

public slots:
    virtual void writeData();
    virtual void resetProtocol();
private slots:
    void readData(bool isReaded);
private:
    ComPort *itsComPort;

    QMultiMap<QString, QString> itsWriteData;
    QMultiMap<QString, QString> itsReadData;

    QTimer *m_resend;
    int m_numResends;
    int m_currentResend;

    QVector<int> m_calibrCoeffs;

    // преобразует word в byte
    int wordToInt(QByteArray ba);
    // определяет температуру
    double sensorTemp(const int &ADC16);
    // disintegrating number of byte by byte
    QByteArray intToByteArray(const int &value, const int &numBytes);
};

#endif // READSENSORPROTOCOL_H
