#include "isp.h"
#include <QFile>

Isp::Isp(QObject *parent) : QObject(parent)
{
    pSerial = new QSerialPort(this);
    isConnect = false;
    startAddr = 0x08000000;
}

void Isp::set_comName(QString str_)
{
    comName = str_;
}

void Isp::set_fileName(QString str_)
{
    fileName = str_;
}

void Isp::set_startAddr(uint32_t addr_)
{
    startAddr = addr_;
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

int Isp::download()
{
    if(isConnect == false)
        return -1;
    QFile file(fileName);
    if(!file.exists()){
        emit send_isp_msg(tr("File is not exist"));
        return -1;
    }
    emit send_isp_msg(fileName);

    if(erase_chip() != 0)
        return -1;
    if(write_firmware(fileName) != 0)
        return -1;
    if(verify_firmware(fileName) != 0)
        return -1;
    return 0;
}

int Isp::write_firmware(QString fileName_)
{
    uint32_t address = startAddr;
    uint32_t nsend = 0;
    QFile file(fileName_);
    if(!file.open(QFile::ReadOnly)){
        emit send_isp_msg(QString("File open error"));
        return -1;
    }

    emit send_isp_msg(tr("Start Program"));

    while(true){
        QByteArray data = file.read(256);
        if(data.size() == 0)
            break;
        uint8_t errorIndex = 0;
        while(true){
            if(write_block(address, (uint8_t*)data.data(), data.size()) == 0){
                address += data.size();
                nsend += data.size();
                emit send_progress_bar_value((float)nsend / (float)file.size() * 100);
                break;
            }else{
                emit send_isp_msg(QString("* [0X%1] write error, retry").arg(address, 8, 16, QChar('0')));
                errorIndex++;
                if(errorIndex >= 10){
                    emit send_isp_msg(QString("Download failed"));
                    file.close();
                    return -1;
                }
            }
        }
    }
    emit send_isp_msg(QString("Download completed"));
    file.close();
    return 0;
}

int Isp::write_block(uint32_t addr_, uint8_t *pData_, int len_)
{
    uint8_t write_block_cmd[300];
    write_block_cmd[0] = ISP_CMD_WM;
    write_block_cmd[1] = ISP_CMD_WM ^ 0xff;
    pSerial->write((char*)write_block_cmd, 2);
    pSerial->waitForReadyRead(50);
    pSerial->read((char*)write_block_cmd, 1);
    if(write_block_cmd[0] != ISP_ACK)
        return -1;

    write_block_cmd[0] = (addr_ >> 24) & 0xff;
    write_block_cmd[1] = (addr_ >> 16) & 0xff;
    write_block_cmd[2] = (addr_ >> 8) & 0xff;
    write_block_cmd[3] = (addr_ ) & 0xff;
    write_block_cmd[4] = check_sum(write_block_cmd, 4);
    pSerial->write((char*)write_block_cmd, 5);
    pSerial->waitForReadyRead(50);
    pSerial->read((char*)write_block_cmd, 1);
    if(write_block_cmd[0] != ISP_ACK)
        return -1;

    write_block_cmd[0] = len_ - 1;
    memcpy(write_block_cmd + 1, pData_, len_);
    write_block_cmd[len_ + 1] = check_sum(write_block_cmd, len_ + 1);
    pSerial->write((char*)write_block_cmd, len_ + 2);
    pSerial->waitForReadyRead(100);
    pSerial->read((char*)write_block_cmd, 1);
    if(write_block_cmd[0] != ISP_ACK)
        return -1;

    return 0;
}

int Isp::verify_firmware(QString fileName_)
{
    uint32_t address = startAddr;
    uint32_t nsend = 0;
    QFile file(fileName_);
    if(!file.open(QFile::ReadOnly)){
        emit send_isp_msg(QString("File open error"));
        return -1;
    }
    emit send_isp_msg(tr("Start Verify"));

    while(true){
        uint8_t receiveData[256] = {0};
        QByteArray data = file.read(256);

        if(data.size() == 0)
            break;
        uint8_t errorIndex = 0;
        while(true){
            if(read_block(address, receiveData, data.size()) == data.size()){
                int i;
                for(i = 0; i < data.size(); i++)
                {
                    if((uint8_t)data.at(i) != receiveData[i])
                        break;
                }
                if(i == data.size()){
                    address += data.size();
                    nsend += data.size();
                    emit send_progress_bar_value((float)nsend / (float)file.size() * 100);
                    break;
                }else{
                    emit send_isp_msg(QString("Verify failed"));
                    file.close();
                    return -1;
                }
            }else{
                emit send_isp_msg(QString("* [0X%1] verify error，retry").arg(address, 8, 16, QChar('0')));
                errorIndex++;
                if(errorIndex >= 10){
                    emit send_isp_msg(QString("Verify failed"));
                    file.close();
                    return -1;
                }
            }
        }
    }
    emit send_isp_msg(QString("Verify success"));
    file.close();
    return 0;
}

int Isp::read_block(uint32_t addr_, uint8_t *pData_, int len_)
{
    uint8_t read_block_cmd[300];
    read_block_cmd[0] = ISP_CMD_RM;
    read_block_cmd[1] = ISP_CMD_RM ^ 0xff;
    pSerial->write((char*)read_block_cmd, 2);
    pSerial->waitForReadyRead(20);
    pSerial->read((char*)read_block_cmd, 1);
    if(read_block_cmd[0] != ISP_ACK)
        return -1;

    read_block_cmd[0] = (addr_ >> 24) & 0xff;
    read_block_cmd[1] = (addr_ >> 16) & 0xff;
    read_block_cmd[2] = (addr_ >> 8) & 0xff;
    read_block_cmd[3] = (addr_ ) & 0xff;
    read_block_cmd[4] = check_sum(read_block_cmd, 4);
    pSerial->write((char*)read_block_cmd, 5);
    pSerial->waitForReadyRead(20);
    pSerial->read((char*)read_block_cmd, 1);
    if(read_block_cmd[0] != ISP_ACK)
        return -1;

    read_block_cmd[0] = len_ - 1;
    read_block_cmd[1] = read_block_cmd[0] ^ 0xff;
    pSerial->write((char*)read_block_cmd, 2);
    pSerial->waitForReadyRead(50);
    pSerial->read((char*)read_block_cmd, 1);
    if(read_block_cmd[0] != ISP_ACK)
        return -1;
    pSerial->waitForReadyRead(200);
    return pSerial->read((char*)pData_, len_);
}

uint8_t Isp::check_sum(uint8_t *pData_, int len_)
{
    uint8_t checkSum = 0;
    while(len_--)
    {
        checkSum ^= *pData_++;
    }
    return checkSum;
}
