#ifndef SCHEDULERGUI_H
#define SCHEDULERGUI_H

#include <QMainWindow>
#include<QLabel>
#include<QLineEdit>
#include<map>
#include<io_control.h>
#include<QTime>
#include<QTcpServer>
#include<QTcpSocket>
#include<QTimer>
#include<gpio_pi.h>
#include<socket.h>
#include<scheduler.h>
#include<QCloseEvent>
#include<timer.h>
#include<QEvent>

using namespace Scheduler;

namespace Ui {
class SchedulerGUI;
}

class SchedulerGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit SchedulerGUI(QWidget *parent = 0);
    ~SchedulerGUI();
    void closeEvent(QCloseEvent *event);
private:
    void uiIni();
    void dataIni();
    void paramIni();
    bool IoIni();
    void deviceIni();
    bool serverIni();
    void updateUI();
    void updateModuleUI();
    void updateRobotUI();
    void setRobotLabelPosition();
    void iniModuleLabel();
    void connectSlotAndSignal();
    void closeServer();

    void sendCatchOrder(uint robotNumber);
    void sendClearOrder(uint robotNumber);
    void emergencyStop();
    void stopConveyor();
    void activateConveyor();
    void removeRobotSocket(uint robotNumber);
    void addRobotSocket(uint robotNumber,Socket*);
    void processMsgFromRobot(uint robotNumber,QString msg);
    void statisticAdapter(uint robotNumber,uint flag);
signals:
    void signalAddAdapter(uint modelState);
private slots:
    void slotRobotWaiting(uint robotNumber);
    void slotRobotCatching(uint robotNumber);
    void slotRobotCatched(uint robotNumber);
    void slotRobotOnLine(uint robotNumber);
    void slotRobotOffLine(uint robotNumber);
    void slotRobotAckCatchedError(uint roborNumber);
    void slotRobotRecognizeError(uint robotNumber);
    void slotRobotClearing(uint robotNumber);

    void slotAddAdapter(uint modelState);
    void checkCatchable();
    void calculateOptimizeCatchTime(bool &isCatchNow, std::vector<uint> &nowCanCatchRobotArray);
    void updateModuleArrayState();
    void checkIsActivateConveyor();
    void clearError();
    void clearAckCatchedError();
    void clearLastModuleHaveAdapterError();
    void checkResetButtonStatus();
    void slotControlPlate(uint,uint);
    void slotSetConveyorStatus(uint,uint);
    void slotReleaseAllPlate();
    void slotPressAllPlate();
    void slotSetAlarmStatus(bool,uint alarmFlag);
    void slotAlarmTimerTimeout();

    void slotNewConnection();
    void slotReceiveMsg(uint);
    void slotDisconnect(uint);
    void slotSendMsg(uint,QString);
    void delay(uint msec);
    void addRunMessage(QString msg,const QColor&color);
    void robotReplyMsgTimeOut(const uint&robotNumber);
private slots:
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
    void on_radioButton_clicked();
    void on_radioButton_2_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

private:
    uint* robotUnrecognizeNum;

    Ui::SchedulerGUI *ui;
    IO_Control* ioUI;

    QTimer checkCatchableTimer;

    ////////1.ack catched timeout  2.clear last loaded module///////////////
    QTimer checkResetSignalTimer;
    QTimer alarmTimer;

    uint checkCatchTimerInterval;
    uint checkResetButtonInterval;
    uint flashAlarmInterval;
    uint stopConveyorDelayTime=0;

    bool isConveyorRunning=false;
    bool isStartCount=false;
    bool isValidModule=false;
    bool isProductiveTwoModel=false;
    bool _isProductiveTwoModel=false;
    bool alarmStatus=false;

    uint detectModuleTime=0;
    uint maxDetectModuleTime=8;

    ///////detect adapter about/////////
    bool prevR4AdapterSensorStatus=false;
    bool prevR1ModuleSensorStatus=false;
    bool prevR1AdapterSensorStatus=false;
    uint detectAdapterTime=0;
    uint maxDetectAdapterTime=4;
    bool isDetectFallingEdge=false;
    bool isHaveAdapter=false;
    bool isEmergencyStop=false;
    bool isPushResetButton=false;

    QString catchOrder,clearOrder;

    uint moduleNum;
    uint robotNum;
    uint* robotWorkStateArray;
    uint* moduleStateArray;
    uint checkCatchingMsgInterval=500;
    uint checkCatchedMsgInterval=7000;
    uint robotMaxWaitTime=30000;

    int lastRobotNumber=-1;
    int onlineRobotNum=0;

    uint modelGap=20;
    uint modelRadius=40;
    uint labelGap=60;
    uint robotLabelWidth=230;
    uint robotLabelHeight=50;

    QLabel** robotLabelArr;
    QLabel* conveyorLabel;
    QLabel** moduleLabelArray;
    QLineEdit** robotUnrecognizeNumLineEdit;

    uint conveyorStatusIO,conveyorDirectionIO;
    uint alarmStatusIO,resetAlarmButtonIO;

    bool flag=false;

    std::map<uint,QString> robotName;
    std::map<uint,uint> robotModuleNumber;
    std::map<uint,QString>robotIP;
    std::map<uint,uint>robotPlateIO;
    std::map<uint,uint>robotModuleSensorIO;
    std::map<uint,uint>robotAdapterSensorIO;
    std::map<uint,Socket*>robotSocketArray;
    std::map<QString,uint>robotMsgToStatusArray;
    std::map<uint,Timer*>robotMsgTimer;
    std::map<uint,QTime>robotWaitQTimer;

    Timer* timerPtr;
    QTcpServer* server;
    GPIO_PI gpio_pi;
};

#endif // SCHEDULERGUI_H
