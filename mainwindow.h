#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "isp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browseBtn_clicked();

    void on_downloadBtn_clicked();

    void on_readBtn_clicked();

    void on_eraseBtn_clicked();

    void on_clearBtn_clicked();

    void on_saveBtn_clicked();

    void on_refreshBtn_clicked();

    void on_connectBtn_clicked();

    void msg_display(QString str);

    void progress_bar_update(int value);

private:
    Ui::MainWindow *ui;
    Isp *pIsp;
};
#endif // MAINWINDOW_H
