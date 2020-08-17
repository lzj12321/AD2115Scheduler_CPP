#ifndef IO_CONTROL_H
#define IO_CONTROL_H

#include <QDialog>
#include<QCheckBox>

namespace Ui {
class IO_Control;
}

class IO_Control : public QDialog
{
    Q_OBJECT

public:
    explicit IO_Control(QWidget *parent = 0);
    ~IO_Control();

private:
    void dataIni();

private slots:
    void on_checkBox_clicked();
    void on_checkBox_2_clicked();
    void on_checkBox_3_clicked();
    void on_checkBox_4_clicked();
    void on_radioButton_clicked();
    void on_radioButton_3_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
private:
    Ui::IO_Control *ui;
signals:
    void signalControlPlate(uint,uint);
    void signalControlConveyor(uint,uint);
    void signalPressAllPlate();
    void signalReleaseAllPlate();
};

#endif // IO_CONTROL_H
