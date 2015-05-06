#include "RRMProtocol.h"

#ifdef DEBUG
#include <QDebug>
#endif

#define STARTBYTE 0x55
#define STOPBYTE 0xAA
#define BYTESLENTH 8

#define FORMAT 'f'
#define PRECISION 2

#define CODE_WRITE 0x00
#define CODE_DP1 0x01
#define CODE_DP2 0x02
#define CODE_TEMP 0x03
#define CODE_CALIBR 0x04

#define NULL_DATA '\0'

RRMProtocol::RRMProtocol(ComPort *comPort, QObject *parent) :
    IProtocol(parent),
    itsComPort(comPort),
    m_resend(new QTimer(this)),
    m_numResends(3),
    m_currentResend(0)
{
    m_resend->setInterval(100);

    m_calibrCoeffs.push_back(40781);
    m_calibrCoeffs.push_back(32791);
    m_calibrCoeffs.push_back(36016);
    m_calibrCoeffs.push_back(24926);
    m_calibrCoeffs.push_back(28446);

    connect(itsComPort, SIGNAL(DataIsReaded(bool)), this, SLOT(readData(bool)));
    connect(itsComPort, SIGNAL(DataIsWrited(bool)), this, SIGNAL(DataIsWrited(bool)));
//    connect(m_resend, SIGNAL(timeout()), this, SLOT(writeData()));
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

        itsReadData.insert(QString("CODE"), QString::number(static_cast<int>(ba.at(1))));

        if( static_cast<int>(ba.at(1)) < 0x10 ) {

            itsReadData.insert(QString("DP1"),
                               QString::number(wordToInt(ba.mid(1, 2))));
            itsReadData.insert(QString("DP2"),
                               QString::number(wordToInt(ba.mid(3, 2))));
            itsReadData.insert(QString("TEMP"),
                               QString::number(sensorTemp(wordToInt(ba.mid(5, 2))), FORMAT, PRECISION));

        } else {

            switch ( static_cast<int>(ba.at(1)) ) {
            case 0x10:
//                itsReadData.insert(QString("K0"),
//                                   QString::number(wordToInt(ba.mid(2, 2)), FORMAT, PRECISION));
                m_calibrCoeffs.replace(0, wordToInt(ba.mid(2, 2)));
                break;
            case 0x11:
//                itsReadData.insert(QString("K1"),
//                                   QString::number(wordToInt(ba.mid(2, 2)), FORMAT, PRECISION));
                m_calibrCoeffs.replace(1, wordToInt(ba.mid(2, 2)));
                break;
            case 0x12:
//                itsReadData.insert(QString("K2"),
//                                   QString::number(wordToInt(ba.mid(2, 2)), FORMAT, PRECISION));
                m_calibrCoeffs.replace(2, wordToInt(ba.mid(2, 2)));
                break;
            case 0x13:
//                itsReadData.insert(QString("K3"),
//                                   QString::number(wordToInt(ba.mid(2, 2)), FORMAT, PRECISION));
                m_calibrCoeffs.replace(3, wordToInt(ba.mid(2, 2)));
                break;
            case 0x14:
//                itsReadData.insert(QString("K4"),
//                                   QString::number(wordToInt(ba.mid(2, 2)), FORMAT, PRECISION));
                m_calibrCoeffs.replace(4, wordToInt(ba.mid(2, 2)));
                break;
            }

        }

        emit DataIsReaded(true);

    } else {
        emit DataIsReaded(false);
    }
//            // TODO
//            if( !itsWriteData.isEmpty() && itsReadData.value("TEMP") != itsWriteData.value("TEMP")
//                    && m_currentResend < m_numResends ) {
//                m_resend->start();
//                ++m_currentResend;
//            } else {
//                m_currentResend = 0;
//                m_resend->stop();
//            }
//            // end TODO
}

void RRMProtocol::writeData()
{
    QByteArray ba;

    ba.append(STARTBYTE);
    ba.append(itsWriteData.value("CODE").toInt());

    if( itsWriteData.value("CODE").toInt() != CODE_WRITE
            && itsWriteData.value("CODE").toInt() != CODE_CALIBR ) {

        ba.append(intToByteArray(itsWriteData.value("DATA").toInt(), 2).at(0));
        ba.append(intToByteArray(itsWriteData.value("DATA").toInt(), 2).at(1));

    } else {

        ba.append(NULL_DATA);
        ba.append(NULL_DATA);

    }
    ba.append(NULL_DATA);
    ba.append(NULL_DATA);
    ba.append(NULL_DATA);
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
    // TODO
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
double RRMProtocol::sensorTemp(const int &ADC16)
{
    double temp = 0.01 * ( (-1.5)*m_calibrCoeffs.at(0) + ADC16*0.0001 *
                           (m_calibrCoeffs.at(1) + ADC16*0.00001 *
                            ( (-2)*m_calibrCoeffs.at(2) + ADC16*0.00001 *
                              (4*m_calibrCoeffs.at(3) + (-2)*0.00001*m_calibrCoeffs.at(4)*ADC16 ) ) ) );

    return temp;
}

QByteArray RRMProtocol::intToByteArray(const int &value, const int &numBytes)
{
    QByteArray ba;

    for(int i = numBytes - 1; i > - 1; --i) {
        ba.append((value >> 8*i) & 0xFF);
    }

    return ba;
}
