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

#define CODE_TEMP 0x00
#define CODE_DP1 0x01
#define CODE_DP2 0x02

#define NONE_DATA '\0'

RRMProtocol::RRMProtocol(ComPort *comPort, QObject *parent) :
    IProtocol(parent),
    itsComPort(comPort),
    itsPrevCPUTemp(0.0),
    itsPrevSensorTemp(0.0),
    itsWasPrevCPUTemp(false),
    itsWasPrevSensorTemp(false)
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
#ifdef DEBUG
        qDebug() << "read: " << QString::number(static_cast<int>(ba.at(1)));
    for(int i = 0; i < ba.size(); ++i) {
        qDebug() << "ba =" << (int)ba.at(i);
    }
#endif
        itsReadData.insert(QString("CODE"), QString::number(static_cast<int>(ba.at(1))));
        if(static_cast<int>(ba.at(1)) == CODE_TEMP) {
            itsReadData.insert(QString("DATA"),
                               QString::number(tempCorr(tempSensors(wordToInt(ba.mid(2, 2))), SENSOR), FORMAT, PRECISION));

            emit DataIsReaded(true);
#ifdef DEBUG
            qDebug() << "Reading temperature";
#endif
        } else if(static_cast<int>(ba.at(1)) == CODE_DP1 || static_cast<int>(ba.at(1)) == CODE_DP2){
#ifdef DEBUG
            qDebug() << "Reading DPs";
#endif
            itsReadData.insert(QString("DATA"), QString::number(wordToInt(ba.mid(2, 2))));

            emit DataIsReaded(true);
        } else {
            emit DataIsReaded(false);
        }
    } else {
        emit DataIsReaded(false);
    }
}

void RRMProtocol::writeData()
{
    QByteArray ba;

    ba.append(STARTBYTE);
    ba.append(itsWriteData.value("CODE").toInt());
    ba.append(intToByteArray(itsWriteData.value("DATA").toInt(), 2).at(0));
    ba.append(intToByteArray(itsWriteData.value("DATA").toInt(), 2).at(1));
    ba.append(NONE_DATA);
    ba.append(NONE_DATA);
    ba.append(NONE_DATA);
    ba.append(STOPBYTE);

    itsComPort->setWriteData(ba);
#ifdef DEBUG
    qDebug() << "write";
    for(int i = 0; i < ba.size(); ++i) {
        qDebug() << "ba =" << (int)ba.at(i);
    }
#endif
    itsComPort->writeData();
}

void RRMProtocol::resetProtocol()
{
    itsWasPrevCPUTemp = false;
    itsWasPrevSensorTemp = false;
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
    case SENSOR:
        prevValue = itsPrevSensorTemp;
        wasPrev = itsWasPrevSensorTemp;
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
    case SENSOR:
        itsPrevSensorTemp = prevValue;
        itsWasPrevSensorTemp = true;
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
    case SENSOR:
        return "SENS";
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
