#ifndef READSENSORPROTOCOL_H
#define READSENSORPROTOCOL_H

#include "IProtocol.h"
#include "ComPort.h"

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

    float itsPrevCPUTemp;
    float itsPrevSensorTemp;

    bool itsWasPrevCPUTemp;
    bool itsWasPrevSensorTemp;

    // датчики температуры
    enum SENSORS {
        CPU, SENSOR
    };

    // преобразует word в byte
    int wordToInt(QByteArray ba);
    // определяет температуру
    float tempSensors(int temp);
    // определяет температуру кристалла
    float tempCPU(int temp);
    // коррекция скачков температуры
    float tempCorr(float temp, SENSORS sensor);
    // преобразует enum SENSORS в строку
    QString sensorToString(SENSORS sensor);
    // disintegrating number of byte by byte
    QByteArray intToByteArray(const int &value, const int &numBytes);
};

#endif // READSENSORPROTOCOL_H
