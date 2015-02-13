#include "RRMProtocol.h"

#ifdef DEBUG
#include <QDebug>
#endif

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

RRMProtocol::RRMProtocol(ComPort *comPort, QObject *parent) :
    IProtocol(parent),
    itsComPort(comPort),
    itsPrevCPUTemp(0.0),
    itsPrevSensor1Temp(0.0),
    itsPrevSensor2Temp(0.0),
    itsWasPrevCPUTemp(false),
    itsWasPrevSensor1Temp(false),
    itsWasPrevSensor2Temp(false)
{
    connect(itsComPort, SIGNAL(DataIsReaded(bool)), this, SLOT(readData(bool)));
}

void RRMProtocol::setDataToWrite(const QMultiMap<QString, QString> &data)
{
    itsWriteData = data;
}

QMultiMap<QString, QString> RRMProtocol::getReadedData() const
{
    return itsReadData;
}

void RRMProtocol::readData(bool isReaded)
{
    itsReadData.clear();

    if(isReaded) {
        QByteArray ba;

        ba = itsComPort->getReadData();

        itsReadData.insert(QString("CODE"), QString(ba.at(1)));
        itsReadData.insert(QString("SENS1"),
                           QString::number(tempCorr(tempSensors(wordToInt(ba.mid(2, 2))), SENSOR1), FORMAT, PRECISION));
        itsReadData.insert(QString("SENS2"),
                           QString::number(tempCorr(tempSensors(wordToInt(ba.mid(4, 2))), SENSOR2), FORMAT, PRECISION));

        emit DataIsReaded(true);
    } else {
        emit DataIsReaded(false);
    }
}

void RRMProtocol::writeData()
{
    QByteArray ba;

    ba.append(STARTBYTE);
    ba.append(itsWriteData.value("CODE").toInt());
    ba.append(intToByteArray(itsWriteData.value("TEMP").toInt(), 2).at(0));
    ba.append(intToByteArray(itsWriteData.value("TEMP").toInt(), 2).at(1));
    ba.append('\0');
    ba.append('\0');
    ba.append('\0');
    ba.append(STOPBYTE);

    itsComPort->setWriteData(ba);
#ifdef DEBUG
    for(int i = 0; i < ba.size(); ++i) {
        qDebug() << "ba =" << (int)ba.at(i);
    }
#endif
    itsComPort->writeData();
}

void RRMProtocol::resetProtocol()
{
    itsWasPrevCPUTemp = false;
    itsWasPrevSensor1Temp = false;
    itsWasPrevSensor2Temp = false;
}

// преобразует word в byte
int RRMProtocol::wordToInt(QByteArray ba)
{
    if(ba.size() != 2)
        return -1;

    int temp = ba[0];
    if(temp < 0)
    {
        temp += 0x100; // 256;
        temp *= 0x100;
    }
    else
        temp = ba[0]*0x100; // старший байт

    int i = ba[1];
    if(i < 0)
    {
        i += 0x100; // 256;
        temp += i;
    }
    else
        temp += ba[1]; // младший байт

    return temp;
}

// определяет температуру
float RRMProtocol::tempSensors(int temp)
{
    if(temp & NEGATIVE) {
        return -static_cast<float>(qAbs(temp - OFFSET))/SLOPE;
    } else {
        return static_cast<float>(temp)/SLOPE;
    }
}

// определяет температуру кристалла
float RRMProtocol::tempCPU(int temp)
{
    return (static_cast<float>(temp*CPU_FACTOR - CPU_OFFSET))/CPU_SLOPE;
}

float RRMProtocol::tempCorr(float temp, RRMProtocol::SENSORS sensor)
{
    float prevValue = 0.0;
    bool wasPrev = false;

    switch (sensor) {
    case CPU:
        prevValue = itsPrevCPUTemp;
        wasPrev = itsWasPrevCPUTemp;
        break;
    case SENSOR1:
        prevValue = itsPrevSensor1Temp;
        wasPrev = itsWasPrevSensor1Temp;
        break;
    case SENSOR2:
        prevValue = itsPrevSensor2Temp;
        wasPrev = itsWasPrevSensor2Temp;
        break;
    default:
        prevValue = itsPrevCPUTemp;
        wasPrev = itsWasPrevCPUTemp;
        break;
    }

    if(wasPrev) {
        prevValue = prevValue*(1 - ACCURACY) + temp*ACCURACY;
    } else {
        prevValue = temp;
    }

    switch (sensor) {
    case CPU:
        itsPrevCPUTemp = prevValue;
        itsWasPrevCPUTemp = true;
        break;
    case SENSOR1:
        itsPrevSensor1Temp = prevValue;
        itsWasPrevSensor1Temp = true;
        break;
    case SENSOR2:
        itsPrevSensor2Temp = prevValue;
        itsWasPrevSensor2Temp = true;
        break;
    default:
        itsPrevCPUTemp = prevValue;
        itsWasPrevCPUTemp = true;
        break;
    }

    return prevValue;
}

QString RRMProtocol::sensorToString(RRMProtocol::SENSORS sensor)
{
    switch (sensor) {
    case CPU:
        return "CPU";
        break;
    case SENSOR1:
        return "SENS1";
        break;
    case SENSOR2:
        return "SENS2";
        break;
    default:
        return "CPU";
        break;
    }
}

QByteArray RRMProtocol::intToByteArray(const int &value, const int &numBytes)
{
    QByteArray ba;

    for(int i = numBytes - 1; i > - 1; --i) {
        ba.append((value >> 8*i) & 0xFF);
    }

    return ba;
}
