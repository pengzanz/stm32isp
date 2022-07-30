#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    on_refreshBtn_clicked();
    pIsp = new Isp(this);
    connect(pIsp, SIGNAL(send_isp_msg(QString)), this, SLOT(msg_display(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_browseBtn_clicked()
{

}


void MainWindow::on_downloadBtn_clicked()
{
    pIsp->set_comName(ui->comNameComboBox->currentText().split(" ")[0]);
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
        if(pIsp->connect(ui->comNameComboBox->currentText().split(" ")[0]) == 0)
            ui->connectBtn->setText("Disconnect");
    }
    else{
        pIsp->disconnect();
        ui->connectBtn->setText("Connect");
    }
}

void MainWindow::msg_display(QString str)
{
    ui->textBrowser->append(str);
}

