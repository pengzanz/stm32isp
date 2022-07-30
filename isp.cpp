#include "isp.h"

Isp::Isp(QObject *parent) : QObject(parent)
{
    pSerial = new QSerialPort(this);
    isConnect = false;
}

void Isp::set_comName(QString str)
{
    comName = str;
}

void Isp::set_fileName(QString str)
{
    fileName = str;
}

int Isp::connect(QString comName_)
{
    uint8_t tmp = ISP_CMD_INIT;

    pSerial->setPortName(comName_);
    pSerial->setBaudRate(115200);
    pSerial->setDataBits(QSerialPort::Data8);
    pSerial->setStopBits(QSerialPort::OneStop);
    pSerial->setParity(QSerialPort::EvenParity);
    pSerial->setFlowControl(QSerialPort::NoFlowControl);

    if(!pSerial->open(QSerialPort::ReadWrite)){
        emit send_isp_msg(tr("Com Open failed!"));
        return -1;
    }
    else
        emit send_isp_msg(tr("Com Open success!"));

    pSerial->write((const char*)&tmp, 1);
    pSerial->waitForReadyRead(300);

    if(pSerial->read((char*)&tmp, 1) > 0 && tmp == ISP_ACK){
        emit send_isp_msg(tr("Connect success!"));
    }
    else{
        emit send_isp_msg(tr("Connect failed!"));
        pSerial->close();
        return -1;
    }
    isConnect = true;
    get_version();
    get_id();
    return 0;
}

int Isp::disconnect()
{
    if(pSerial->isOpen())
        pSerial->close();
    isConnect = false;
    return 0;
}

int Isp::get_version()
{
    uint8_t get_version_cmd[5];
    int result = 0;
    if(isConnect == false)
        return -1;
    get_version_cmd[0] = ISP_CMD_GVR;
    get_version_cmd[1] = ISP_CMD_GVR ^ 0xff;
    pSerial->write((const char*)get_version_cmd, 2);
    pSerial->waitForReadyRead(200);
    if(pSerial->read((char*)get_version_cmd, 5) > 0){
        if(get_version_cmd[0] == ISP_ACK && get_version_cmd[4] == ISP_ACK){
            emit send_isp_msg(tr("Isp Ver : %1.%2").arg(get_version_cmd[1] >> 4).arg(get_version_cmd[1] & 0x0f));
        }
        else
            result = -1;
    }
    else
        result = -1;
    return result;
}

int Isp::get_id()
{
    uint8_t get_id_cmd[10];
    if(isConnect == false)
        return -1;

    get_id_cmd[0] = ISP_CMD_GID;
    get_id_cmd[1] = ISP_CMD_GID ^ 0xff;
    pSerial->write((char*)get_id_cmd, 2);
    pSerial->waitForReadyRead(200);
    if(pSerial->read((char*)get_id_cmd, 5) > 0){
        if(get_id_cmd[0] == ISP_ACK && get_id_cmd[4] == ISP_ACK)
            emit send_isp_msg(tr("PID : 0x%1%2").arg(get_id_cmd[2], 2, 16, QLatin1Char('0')).arg(get_id_cmd[3], 2, 16, QLatin1Char('0')));
    }

    return 0;
}

int Isp::erase_chip()
{
    uint8_t erase_chip_cmd[2];
    if(isConnect == false)
        return -1;
    erase_chip_cmd[0] = ISP_CMD_ER;
    erase_chip_cmd[1] = ISP_CMD_ER ^ 0xff;
    pSerial->write((char*)erase_chip_cmd, 2);
    pSerial->waitForReadyRead(50);
    pSerial->read((char*)erase_chip_cmd, 1);
    if(erase_chip_cmd[0] != ISP_ACK)
        return -1;
    erase_chip_cmd[0] = 0xff;
    erase_chip_cmd[1] = 0x00;
    pSerial->write((char*)erase_chip_cmd, 2);
    pSerial->waitForReadyRead(1000);
    pSerial->read((char*)erase_chip_cmd, 1);
    if(erase_chip_cmd[0] == ISP_ACK)
        emit send_isp_msg(tr("Full chip erase"));
    return 0;
}
