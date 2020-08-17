#include "io_control.h"
#include "ui_io_control.h"
#include<QSettings>
#include<QFile>
#include<QMessageBox>
#include<scheduler.h>

using namespace Scheduler;

IO_Control::IO_Control(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IO_Control)
{
    ui->setupUi(this);
    dataIni();
}

IO_Control::~IO_Control()
{
    delete ui;
}

void IO_Control::dataIni()
{
}

void IO_Control::on_checkBox_clicked()
{
    uint status;
    if(ui->checkBox->isChecked())
        status=PLATE_PRESS;
    else
        status=PLATE_RELEASE;
    emit signalControlPlate(0,status);
}

void IO_Control::on_checkBox_2_clicked()
{
    uint status;
    if(ui->checkBox_2->isChecked())
        status=PLATE_PRESS;
    else
        status=PLATE_RELEASE;
    emit signalControlPlate(1,status);
}

void IO_Control::on_checkBox_3_clicked()
{
    uint status;
    if(ui->checkBox_3->isChecked())
        status=PLATE_PRESS;
    else
        status=PLATE_RELEASE;
    emit signalControlPlate(2,status);
}

void IO_Control::on_checkBox_4_clicked()
{
    uint status;
    if(ui->checkBox_4->isChecked())
        status=PLATE_PRESS;
    else
        status=PLATE_RELEASE;
    emit signalControlPlate(3,status);
}

void IO_Control::on_radioButton_clicked()
{
    //forward
    on_pushButton_2_clicked();
    emit signalControlConveyor(CONVEYOR_ACTIVATE,CONVEYOR_FORWARD);

}

void IO_Control::on_radioButton_3_clicked()
{
    //stop
    emit signalControlConveyor(CONVEYOR_STOP,CONVEYOR_STOP);
}

void IO_Control::on_pushButton_clicked()
{
    ///press all///
    emit signalPressAllPlate();
    ui->checkBox->setChecked(true);
    ui->checkBox_2->setChecked(true);
    ui->checkBox_3->setChecked(true);
    ui->checkBox_4->setChecked(true);
}

void IO_Control::on_pushButton_2_clicked()
{
    ///release all///
    emit signalReleaseAllPlate();
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(false);
    ui->checkBox_3->setChecked(false);
    ui->checkBox_4->setChecked(false);
}
