#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    on_refreshBtn_clicked();
    pIsp = new Isp(this);
    connect(pIsp, SIGNAL(send_isp_msg(QString)), this, SLOT(msg_display(QString)));
    connect(pIsp, SIGNAL(send_progress_bar_value(int)), this, SLOT(progress_bar_update(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_browseBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择固件"),"./", tr("(*.bin *.BIN)"));
    ui->pathEdit->setText(fileName);
}


void MainWindow::on_downloadBtn_clicked()
{
    bool ok;
    if(!ui->addrEdit->text().contains("0x")){
        ui->textBrowser->append(tr("start address should be hex format"));
        return;
    }
    pIsp->set_startAddr(ui->addrEdit->text().toUInt(&ok,16));
    pIsp->set_fileName(ui->pathEdit->text());
    pIsp->set_verify(ui->verifyCheckBox->isChecked());
    pIsp->set_readout_protect(ui->readoutProtectCheckBox->isChecked());
    pIsp->download();
}


void MainWindow::on_readBtn_clicked()
{

}


void MainWindow::on_eraseBtn_clicked()
{
    pIsp->erase_chip();
}


void MainWindow::on_clearBtn_clicked()
{
    ui->textBrowser->clear();
}


void MainWindow::on_saveBtn_clicked()
{

}


void MainWindow::on_refreshBtn_clicked()
{
    ui->comNameComboBox->clear();
    QList<QSerialPortInfo> serials = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo port, serials) {
        ui->comNameComboBox->addItem(port.portName() + " " + port.description());
    }
}


void MainWindow::on_connectBtn_clicked()
{
    if(ui->connectBtn->text() == "Connect"){
        if(pIsp->com_connect(ui->comNameComboBox->currentText().split(" ")[0]) == 0
                && pIsp->isp_connect() == 0){
            ui->connectBtn->setText("Disconnect");
            pIsp->get_id();
            pIsp->get_version();
        }
    }
    else{
        pIsp->com_disconnect();
        ui->connectBtn->setText("Connect");
    }
}

void MainWindow::msg_display(QString str)
{
    ui->textBrowser->append(str);
}

void MainWindow::progress_bar_update(int value)
{
    ui->progressBar->setValue(value);
}

