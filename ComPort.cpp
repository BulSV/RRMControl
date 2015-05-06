#include "ComPort.h"
#include <QApplication>
#include <QTime>
#include <QDebug>

ComPort::ComPort(QSerialPort *port,
                 int startByte,
                 int stopByte,
                 int packetLenght,
                 bool isMaster,
                 QObject *parent)
    : QObject(parent),
      itsPort(port),
      itsStartByte(startByte),
      itsStopByte(stopByte),
      itsPacketLenght(packetLenght),
      m_counter(0),
      m_isDataWritten(true),
      m_isMaster(isMaster)
{
    itsReadData.clear();
    itsPort->setReadBufferSize(1); // for reading 1 byte at a time

    connect(itsPort, SIGNAL(readyRead()), this, SLOT(readData()));
}

void ComPort::readData()
{
    if(itsPort->openMode() != QSerialPort::WriteOnly) {
        QByteArray buffer;

        if(itsPort->bytesAvailable() > 0) {
            buffer.append(itsPort->read(1));

            if( !m_counter && (buffer.at(0) == static_cast<char>(itsStartByte)) ) {
                itsReadData.append(buffer);
                ++m_counter;
            } else if( m_counter && (m_counter < itsPacketLenght) ) {
                itsReadData.append(buffer);
                ++m_counter;

                if((m_counter == itsPacketLenght)
                        && itsReadData.at(itsPacketLenght - 1) == static_cast<char>(itsStopByte)) {
                    emit DataIsReaded(true);
                    emit ReadedData(itsReadData);

                    if(!m_isMaster && !m_isDataWritten) {
                        privateWriteData();
                    }

                    itsReadData.clear();
                    m_counter = 0;
                }
            } else {
                emit DataIsReaded(false);

                itsReadData.clear();
                m_counter = 0;
            }
        }
    }
}

QByteArray ComPort::getReadData() const
{
    return itsReadData;
}

void ComPort::setWriteData(const QByteArray &data)
{
    itsWriteData = data;
    m_isDataWritten = false;
}

QByteArray ComPort::getWriteData() const
{
    return itsWriteData;
}

void ComPort::privateWriteData()
{
    if( (itsPort->openMode() != QSerialPort::ReadOnly) && itsPort->isOpen() ) {
        itsPort->write(itsWriteData);
        emit DataIsWrited(true);
        emit WritedData(itsWriteData);
        m_isDataWritten = true;
    }
}

void ComPort::writeData()
{
    if(m_isMaster) {
        privateWriteData();
    }
}
