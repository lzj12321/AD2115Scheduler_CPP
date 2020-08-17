#include "schedulergui.h"
#include "ui_schedulergui.h"
#include<QTime>
#include<QLineEdit>
#include<QPixmap>
#include<QSettings>
#include<QMessageBox>
#include<QInputDialog>
#include<QProcess>

SchedulerGUI::SchedulerGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SchedulerGUI)
{
    ui->setupUi(this);
    paramIni();
    dataIni();
    serverIni();
    IoIni();
    deviceIni();
    uiIni();
    connectSlotAndSignal();
    checkCatchableTimer.setInterval(checkCatchTimerInterval);
    checkCatchableTimer.start();
}

SchedulerGUI::~SchedulerGUI()
{
    slotReleaseAllPlate();
    stopConveyor();

    delete[] robotWorkStateArray;
    delete[] moduleStateArray;
    delete[] robotLabelArr;
    delete[] timerPtr;
    delete[] robotUnrecognizeNum;

    for(uint i=0;i<moduleNum;++i)
        delete moduleLabelArray[i];
    delete moduleLabelArray;

    delete[] robotUnrecognizeNumLineEdit;

    delete server;
    delete ui;
}

void SchedulerGUI::closeEvent(QCloseEvent *event)
{
    int choose;
    choose=QMessageBox::question(this,"Quit","Confirm quit the Scheduler?", QMessageBox::Yes | QMessageBox::No);
    if (choose==QMessageBox::No)
    {
        event->ignore();
    }
    else if(choose==QMessageBox::Yes)
    {
        for(uint i=0;i<robotNum;++i)
        {
            if(robotWorkStateArray[i]==ROBOT_ACK_CATCHED)
            {
                int isClose=QMessageBox::question(this,"Warning","There's a robot in catching status,are you sure quit the Scheduler?",QMessageBox::Yes|QMessageBox::No);
                if(isClose==QMessageBox::Yes)
                {
                    slotReleaseAllPlate();
                    stopConveyor();
                    event->accept();
                    break;
                }
                else
                    event->ignore();
            }
        }
    }
}

void SchedulerGUI::uiIni()
{
    addRunMessage("UI initialize!",Qt::green);

    //    ui->label_6->setFont(QFont::Bold);

    ui->lineEdit_6->setText(QString::number(robotUnrecognizeNum[0],10));
    ui->lineEdit_7->setText(QString::number(robotUnrecognizeNum[1],10));
    ui->lineEdit_8->setText(QString::number(robotUnrecognizeNum[2],10));
    ui->lineEdit_9->setText(QString::number(robotUnrecognizeNum[3],10));


    uint unrecognizeAmount=0;
    for(uint i=0;i<robotNum;++i){
        unrecognizeAmount+=robotUnrecognizeNum[i];
    }

    if(isProductiveTwoModel)
    {
        ui->radioButton->setChecked(true);
    }
    else
    {
        ui->radioButton_2->setChecked(true);
    }
    setRobotLabelPosition();
    iniModuleLabel();
    updateUI();
}

void SchedulerGUI::dataIni()
{
    robotUnrecognizeNumLineEdit=new QLineEdit*[robotNum];
    robotUnrecognizeNumLineEdit[0]=ui->lineEdit_6;
    robotUnrecognizeNumLineEdit[1]=ui->lineEdit_7;
    robotUnrecognizeNumLineEdit[2]=ui->lineEdit_8;
    robotUnrecognizeNumLineEdit[3]=ui->lineEdit_9;
    for(uint i=0;i<robotNum;++i){
        robotUnrecognizeNum[i]=0;
    }
}

void SchedulerGUI::paramIni()
{
    //////////data and param initialize////////////
    addRunMessage("param initialize!",Qt::green);
    QString configFilePath="settings.ini";
    QFile file(configFilePath);
    if(!file.exists())
    {
        QMessageBox::warning(NULL,"Warning","配置文件丢失");
        exit(0);
    }

    QSettings settings(configFilePath,QSettings::IniFormat);
    isProductiveTwoModel=settings.value("runData/isProductiveTwoModel").toBool();
    _isProductiveTwoModel=isProductiveTwoModel;
    conveyorStatusIO=settings.value("io/conveyorStatusIO").toUInt();
    conveyorDirectionIO=settings.value("io/conveyorDirectionIO").toUInt();
    alarmStatusIO=settings.value("io/alarmStatusIO").toUInt();
    resetAlarmButtonIO=settings.value("io/resetAlarmButtonIO").toUInt();
    stopConveyorDelayTime=settings.value("io/stopConveyorDelayTime").toUInt();

    catchOrder=settings.value("order/catchOrder").toString();
    clearOrder=settings.value("order/clearOrder").toString();
    moduleNum=settings.value("deviceNumber/moduleNumber").toUInt();
    robotNum=settings.value("deviceNumber/robotNumber").toUInt();


    robotUnrecognizeNum=new uint[robotNum];


    checkCatchTimerInterval=settings.value("detectParam/checkCatchTimerInterval").toUInt();
    maxDetectAdapterTime=settings.value("detectParam/maxDetectAdapterTime").toUInt();
    checkCatchingMsgInterval=settings.value("detectParam/checkCatchingMsgInterval").toUInt();
    checkCatchedMsgInterval=settings.value("detectParam/checkCatchedMsgInterval").toUInt();
    maxDetectModuleTime=settings.value("detectParam/maxDetectModuleTime").toUInt();
    checkResetButtonInterval=settings.value("detectParam/checkResetButtonInterval").toUInt();
    flashAlarmInterval=settings.value("detectParam/flashAlarmInterval").toUInt();

    modelGap=settings.value("uiParam/modelGap").toUInt();
    modelRadius=settings.value("uiParam/modelRadius").toUInt();
    labelGap=settings.value("uiParam/labelGap").toUInt();
    robotLabelWidth=settings.value("uiParam/robotLabelWidth").toUInt();
    robotLabelHeight=settings.value("uiParam/robotLabelHeight").toUInt();

    for(uint i=0;i<robotNum;++i){
        QString _robotName=settings.value("robotName/robot"+QString::number(i,10)).toString();
        robotName.insert(std::pair<uint,QString>(i,_robotName));

        uint _robotModuleIO=settings.value("io/moduleSensor"+QString::number(i,10)).toUInt();
        robotModuleSensorIO.insert(std::pair<uint,uint>(i,_robotModuleIO));

        uint _robotPlateIO=settings.value("io/plate"+QString::number(i,10)).toUInt();
        robotPlateIO.insert(std::pair<uint,uint>(i,_robotPlateIO));

        uint _robotAdapterIO=settings.value("io/adapterSensor"+QString::number(i,10)).toUInt();
        robotAdapterSensorIO.insert(std::pair<uint,uint>(i,_robotAdapterIO));

        uint _robotModuleNumber=settings.value("robotModuleNumber/robot"+QString::number(i,10)).toUInt();
        robotModuleNumber.insert(std::pair<uint,uint>(i,_robotModuleNumber));

        QString _robotIP=settings.value("robotIP/robot"+QString::number(i,10)).toString();
        robotIP.insert(std::pair<uint,QString>(i,_robotIP));
    }

    robotMsgToStatusArray.insert(std::pair<QString,uint>("wait",ROBOT_WAITING));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("catched",ROBOT_CATCHED));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("unrecognize",ROBOT_RECOGNIZE_ERROR));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("droppass",ROBOT_DROP_PASS));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("clear",ROBOT_CLEARING));
    ////////////ini robot status////////////
    ioUI=new IO_Control(this);

    moduleLabelArray=new QLabel*[moduleNum];
    for(uint i=0;i<moduleNum;++i)
    {
        moduleLabelArray[i]=new QLabel(this);
    }

    moduleStateArray=new uint[moduleNum];
    robotWorkStateArray=new uint[robotNum];

    robotLabelArr=new QLabel*[robotNum];
    robotLabelArr[0]=ui->label;
    robotLabelArr[1]=ui->label_2;
    robotLabelArr[2]=ui->label_3;
    robotLabelArr[3]=ui->label_4;
    conveyorLabel=ui->label_5;


    for(uint i=0;i<robotNum;++i)
        robotWorkStateArray[i]=ROBOT_OFFLINE;
    onlineRobotNum=0;

    for(uint i=0;i<moduleNum;++i){
        moduleStateArray[i]=MODULE_INISTATUS;
    }

    timerPtr=new Timer[robotNum];
    for(uint i=0;i<robotNum;++i){
        timerPtr[i].setTimerData(i,0);
        timerPtr[i].setInterval(checkCatchingMsgInterval);
        robotMsgTimer.insert(std::pair<uint,Timer*>(i,&timerPtr[i]));
        QTime _robotTimer;
        robotWaitQTimer.insert(std::pair<uint,QTime>(i,_robotTimer));
    }
}

bool SchedulerGUI::IoIni()
{
    ///////////initialize the wiringpi io///////////
    if(gpio_pi.gpioIni())
    {
        addRunMessage("io initialize success!",Qt::green);
        return true;
    }
    else
    {
        addRunMessage("io initialize fail!",Qt::red);
        return false;
    }
}

void SchedulerGUI::deviceIni()
{
    ////////release all plate and stop the conveyor//////////
    for(uint i=0;i<robotNum;++i){
        slotControlPlate(i,PLATE_RELEASE);
    }
    stopConveyor();
}

void SchedulerGUI::updateUI()
{
    QTime timer;
    timer.start();
    updateRobotUI();
    updateModuleUI();
}

void SchedulerGUI::updateModuleUI()
{
    for(uint i=0;i<moduleNum;++i)
    {
        switch (moduleStateArray[i])
        {
        case MODULE_INISTATUS:
        {
            moduleLabelArray[i]->setStyleSheet("background:blue");
        }
            break;
        case MODULE_EMPTY:
        {
            moduleLabelArray[i]->setStyleSheet("background:gray");
        }
            break;
        case MODULE_LOADED:
        {
            moduleLabelArray[i]->setStyleSheet("background:green");
        }
            break;
        case MODULE_CATCHING:
        {
            moduleLabelArray[i]->setStyleSheet("background:yellow");
        }
            break;
        case MODULE_UNRECOGNIZE_ONCETIME:
        {
            moduleLabelArray[i]->setStyleSheet("background:magenta");
        }
            break;
        case MODULE_UNRECOGNIZE_TWICETIME:
        {
            moduleLabelArray[i]->setStyleSheet("background:darkMagenta");
        }
            break;
        case MODULE_ERROR:
        {
            moduleLabelArray[i]->setStyleSheet("background:red");
        }
            break;
        default:
            break;
        }
    }
}

void SchedulerGUI::updateRobotUI()
{
    for(uint i=0;i<robotNum;++i){
        switch (robotWorkStateArray[i]){
        case ROBOT_CONNECTED:
        {
            robotLabelArr[i]->setText(robotName[i]+": CONNECTED");
            robotLabelArr[i]->setStyleSheet("background:blue");
        }
            break;
        case ROBOT_WAITING:
        {
            robotLabelArr[i]->setText(robotName[i]+": WAITTING");
            robotLabelArr[i]->setStyleSheet("background:yellow");
        }
            break;
        case ROBOT_CATCHED:
        {
            robotLabelArr[i]->setText(robotName[i]+": WORKING");
            robotLabelArr[i]->setStyleSheet("background:green");
        }
            break;
        case ROBOT_ACK_CATCHED:
        {
            robotLabelArr[i]->setText(robotName[i]+":WAIT CAUGHT");
            robotLabelArr[i]->setStyleSheet("background:green");
        }
            break;
        case ROBOT_OFFLINE:
        {
            robotLabelArr[i]->setText(robotName[i]+": OFFLINE");
            robotLabelArr[i]->setStyleSheet("background:red");
        }
            break;
        case ROBOT_ACK_CATCHED_ERROR:
        {
            robotLabelArr[i]->setText(robotName[i]+":WAIT CAUGHT ERROR");
            robotLabelArr[i]->setStyleSheet("background:red");
        }
            break;
        case ROBOT_RECOGNIZE_ERROR:
        {
            robotLabelArr[i]->setText(robotName[i]+": RECOGNIZE ERROR");
            robotLabelArr[i]->setStyleSheet("background-color:rgb(143,89,2)");
        }
            break;
        case ROBOT_CLEARING:
        {
            robotLabelArr[i]->setText(robotName[i]+": CLEARING");
            robotLabelArr[i]->setStyleSheet("background:blue");
        }
            break;
        default:
            break;
        }
    }
}

void SchedulerGUI::setRobotLabelPosition()
{

    //    uint startX=conveyorLabel->x();
    //    uint startY=conveyorLabel->y();

    //    for(uint i=0;i<moduleNum;++i)
    //    {
    //        uint x=startX+(i*modelRadius*2);
    //        uint y=startY;
    //        moduleLabelArray[i]->setGeometry(x,y,modelRadius+15,modelRadius);
    //    }

    uint robotNumber=0;
    uint startDrawX=conveyorLabel->x();
    uint startDrawY=conveyorLabel->y()+conveyorLabel->height()/2;

    for(uint i=0;i<moduleNum;++i){
        uint drawX=startDrawX+i*(modelRadius*2);
        uint drawY=startDrawY;

        bool isRobotModulePosition=false;
        for(uint j=0;j<robotNum;++j){
            isRobotModulePosition=isRobotModulePosition||(i==robotModuleNumber[j]);
        }
        if(isRobotModulePosition)
        {
            switch (robotNumber)
            {
            case 0:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY+labelGap-modelRadius,robotLabelWidth,robotLabelHeight);
            }
                break;
            case 1:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY-labelGap-modelRadius*2,robotLabelWidth,robotLabelHeight);
            }
                break;
            case 2:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY+labelGap-modelRadius,robotLabelWidth,robotLabelHeight);
            }
                break;
            case 3:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY-labelGap-modelRadius*2,robotLabelWidth,robotLabelHeight);
            }
                break;
            default:
                break;
            }
            ++robotNumber;
        }
    }
}

void SchedulerGUI::iniModuleLabel()
{
    uint startX=conveyorLabel->x();
    uint startY=conveyorLabel->y();

    for(uint i=0;i<moduleNum;++i)
    {
        bool isRobotPosition=false;
        for(uint j=0;j<robotNum;++j){
            isRobotPosition=isRobotPosition||(i==robotModuleNumber[j]);
        }
        uint x=startX+(i*modelRadius*2);
        uint y=startY;
        moduleLabelArray[i]->setGeometry(x,y,modelRadius+15,modelRadius);
        moduleLabelArray[i]->setFrameShape(QFrame::Box);
        moduleLabelArray[i]->setFrameShadow(QFrame::Sunken);
        if(isRobotPosition)
        {
            moduleLabelArray[i]->setLineWidth(8);
        }
    }
}

void SchedulerGUI::connectSlotAndSignal()
{
    connect(ioUI,SIGNAL(signalControlConveyor(uint,uint)),this,SLOT(slotSetConveyorStatus(uint,uint)));
    connect(ioUI,SIGNAL(signalControlPlate(uint,uint)),this,SLOT(slotControlPlate(uint,uint)));
    connect(ioUI,SIGNAL(signalPressAllPlate()),this,SLOT(slotPressAllPlate()));
    connect(ioUI,SIGNAL(signalReleaseAllPlate()),this,SLOT(slotReleaseAllPlate()));


    connect(&checkCatchableTimer,SIGNAL(timeout()),this,SLOT(updateModuleArrayState()));
    connect(this,SIGNAL(signalAddAdapter(uint)),this,SLOT(slotAddAdapter(uint)));
    connect(&checkResetSignalTimer,SIGNAL(timeout()),this,SLOT(checkResetButtonStatus()));
    connect(&alarmTimer,SIGNAL(timeout()),this,SLOT(slotAlarmTimerTimeout()));
    //////////////connect robot timer msg////////////////////////////
    for(uint i=0;i<robotNum;++i){
        connect(robotMsgTimer[i],SIGNAL(timerOutTime(uint)),this,SLOT(robotReplyMsgTimeOut(uint)));
    }
    //    addRunMessage("connect the slot and signal!",Qt::green);
}

void SchedulerGUI::closeServer()
{
    for(uint i=0;i<robotNum;++i)
    {
        slotRobotOffLine(i);
    }
    server->close();
}

bool SchedulerGUI::serverIni()
{
    server=new QTcpServer(this);
    if(server->listen(QHostAddress::Any, 8888))
    {
        connect(server,SIGNAL(newConnection()),this,SLOT(slotNewConnection()));
        addRunMessage("server initialize sucess",Qt::green);
    }
    else
    {
        QMessageBox::warning(NULL,"Warning","server initialize fail,please check the network then restart this program!");
        exit(0);
    }
}

void SchedulerGUI::sendCatchOrder(uint robotNumber)
{
    if(robotSocketArray[robotNumber]->sendMsg(catchOrder))
    {
        robotMsgTimer[robotNumber]->setTimerData(robotNumber,ROBOT_ACK_CATCHED);
        robotMsgTimer[robotNumber]->setInterval(checkCatchedMsgInterval);
        robotMsgTimer[robotNumber]->start();
        slotRobotCatching(robotNumber);
    }
    else
    {
        addRunMessage("robot "+QString::number(robotNumber,10)+" send catch order error!",Qt::red);
        slotRobotOffLine(robotNumber);
    }
}

void SchedulerGUI::sendClearOrder(uint robotNumber)
{
    if(!robotSocketArray[robotNumber]->sendMsg(clearOrder))
    {
        addRunMessage("robot "+QString::number(robotNumber,10)+" send clear order error!",Qt::red);
        slotRobotOffLine(robotNumber);
    }
}

void SchedulerGUI::emergencyStop()
{
    isEmergencyStop=true;
    stopConveyor();
    moduleStateArray[moduleNum-1]=MODULE_ERROR;
    updateModuleUI();
    slotSetAlarmStatus(START_ALARM,ALARM_CONTINUOUS);
    addRunMessage("Emergency Stop!",Qt::red);
}

void SchedulerGUI::stopConveyor()
{
    isConveyorRunning=false;
    slotSetConveyorStatus(CONVEYOR_STOP,CONVEYOR_STOP);
}

void SchedulerGUI::activateConveyor()
{
    isConveyorRunning=true;
    slotSetConveyorStatus(CONVEYOR_ACTIVATE,CONVEYOR_FORWARD);
}

void SchedulerGUI::removeRobotSocket(uint robotNumber)
{
    if(robotSocketArray.find(robotNumber)!=robotSocketArray.end())
    {
        delete robotSocketArray[robotNumber];
        robotSocketArray.erase(robotSocketArray.find(robotNumber));
        --onlineRobotNum;
    }
}

void SchedulerGUI::addRobotSocket(uint robotNumber, Socket *_socket)
{
    removeRobotSocket(robotNumber);
    robotSocketArray.insert(std::pair<uint,Socket*>(robotNumber,_socket));
    ++onlineRobotNum;
}

void SchedulerGUI::updateModuleArrayState()
{
    if(!isConveyorRunning)
    {
        checkCatchable();
    }

    bool currR1ModuleSensorStatus=gpio_pi.getIOStatus(robotModuleSensorIO[0]);
    if(
            (!isDetectFallingEdge)&&
            ((currR1ModuleSensorStatus&&prevR1ModuleSensorStatus&&isConveyorRunning)
             ||isEmergencyStop
             ))
    {
        if(!isEmergencyStop&&detectModuleTime<999)
        {
            ++detectModuleTime;
            ++detectAdapterTime;
        }
        if(detectAdapterTime<maxDetectAdapterTime&&!isHaveAdapter)
        {
            bool currR1AdapterSensorStatus=gpio_pi.getIOStatus(robotAdapterSensorIO[0]);
            isHaveAdapter=currR1AdapterSensorStatus&&prevR1AdapterSensorStatus;
            if(isHaveAdapter)
            {
                if(isProductiveTwoModel)
                {
                    if(!isStartCount){
                        isStartCount=true;
                        isValidModule=true;
                    }
                }
            }
            prevR1AdapterSensorStatus=currR1AdapterSensorStatus;
        }
        else if(detectAdapterTime>=maxDetectAdapterTime)
        {
            if(isProductiveTwoModel)
            {
                if(isValidModule&&isStartCount&&isHaveAdapter)
                    emit signalAddAdapter(MODULE_LOADED);
                else if(isValidModule&&isStartCount&&!isHaveAdapter)
                {
                    emit signalAddAdapter(MODULE_EMPTY);
                }
            }
            else
            {
                if(isHaveAdapter)
                    emit signalAddAdapter(MODULE_LOADED);
                else
                   emit signalAddAdapter(MODULE_EMPTY);
            }
            isDetectFallingEdge=true;
        }
    }

    ///////begin to detect is the module leave after detected a module come//////
    if((isDetectFallingEdge&&isConveyorRunning)||isEmergencyStop)
    {
        if(!isEmergencyStop&&detectModuleTime<999)
        {
            ++detectModuleTime;
        }

        if(!(prevR1ModuleSensorStatus||currR1ModuleSensorStatus))
        {
            if(isProductiveTwoModel)
            {
                isValidModule=!isValidModule;
            }
            detectModuleTime=0;
            isDetectFallingEdge=false;
            isHaveAdapter=false;
            prevR1AdapterSensorStatus=false;
            detectAdapterTime=0;
        }
    }
    prevR1ModuleSensorStatus=currR1ModuleSensorStatus;

    /////////use the last sensor to detect the last position///////////////
    if(isConveyorRunning)
    {
        bool currR4AdapterSensorStatus=gpio_pi.getIOStatus((robotAdapterSensorIO[3]));
        bool isLastModuleHaveAdapter=prevR4AdapterSensorStatus&&currR4AdapterSensorStatus;
        if(isLastModuleHaveAdapter)
            emergencyStop();
        prevR4AdapterSensorStatus=currR4AdapterSensorStatus;
    }
    checkIsActivateConveyor();
}

void SchedulerGUI::checkIsActivateConveyor()
{
    /////////////////////confirm is stop conveyor////////////////////////
    if(onlineRobotNum==0)
    {
        stopConveyor();
        return;
    }

    if(isEmergencyStop)
        return;
    if((
                detectModuleTime<maxDetectModuleTime
                &&detectModuleTime!=0
                &&isValidModule
                )
            &&(moduleStateArray[robotModuleNumber[lastRobotNumber]]==MODULE_LOADED
               ||(robotWorkStateArray[lastRobotNumber]!=ROBOT_RECOGNIZE_ERROR
                  &&moduleStateArray[robotModuleNumber[lastRobotNumber]]==MODULE_UNRECOGNIZE_ONCETIME)))
    {
        stopConveyor();
        return;
    }

    for(uint i=0;i<robotNum;++i)
    {
        if(
                robotWorkStateArray[i]==ROBOT_ACK_CATCHED_ERROR||
                robotWorkStateArray[i]==ROBOT_ACK_CATCHED
                ){
            stopConveyor();
            return;
        }
    }
    activateConveyor();
}

void SchedulerGUI::clearError()
{
    clearAckCatchedError();
    if(isEmergencyStop)
        clearLastModuleHaveAdapterError();
}

void SchedulerGUI::clearAckCatchedError()
{
    /////////////////confirm the robot is in a safe status then you can activate the conveyor//////
    for(uint i=0;i<robotNum;++i)
    {
        if(robotWorkStateArray[i]==ROBOT_ACK_CATCHED_ERROR)
        {
            robotWorkStateArray[i]=ROBOT_OFFLINE;
        }
    }

    slotSetAlarmStatus(STOP_ALARM,STOP_ALARM);
    isEmergencyStop=false;
    addRunMessage("Clear Ack Catched Error!",Qt::blue);
}

void SchedulerGUI::clearLastModuleHaveAdapterError()
{
    bool isEmergencyAdapterExists=gpio_pi.getIOStatus(robotAdapterSensorIO[3]);
    if(isEmergencyAdapterExists==true)
    {
        addRunMessage("Please clear the adapter first!",Qt::red);
        return;
    }

    if(moduleStateArray[moduleNum-2]==MODULE_LOADED||moduleStateArray[moduleNum-2]==MODULE_UNRECOGNIZE_ONCETIME)
        moduleStateArray[moduleNum-2]=MODULE_EMPTY;

    slotSetAlarmStatus(STOP_ALARM,STOP_ALARM);
    isEmergencyStop=false;
    addRunMessage("Clear Last Module Have Adapter Error!",Qt::blue);
}

void SchedulerGUI::checkResetButtonStatus()
{
    bool _isPush=gpio_pi.getIOStatus(resetAlarmButtonIO);
    isPushResetButton=isPushResetButton&&_isPush;
    if(isPushResetButton)
    {
        clearLastModuleHaveAdapterError();
        addRunMessage("Detected push the reset button!",Qt::blue);
        isPushResetButton=false;
        return;
    }
    isPushResetButton=_isPush;
}
void SchedulerGUI::slotRobotWaiting(uint robotNumber)
{
    robotWorkStateArray[robotNumber]=ROBOT_WAITING;
    updateRobotUI();
    robotWaitQTimer[robotNumber].start();
}

void SchedulerGUI::slotRobotCatching(uint robotNumber)
{
    if(moduleStateArray[robotModuleNumber[robotNumber]]!=MODULE_UNRECOGNIZE_ONCETIME)
        moduleStateArray[robotModuleNumber[robotNumber]]=MODULE_CATCHING;
    robotWorkStateArray[robotNumber]=ROBOT_ACK_CATCHED;
    updateUI();
    ////////start the ack catched timer//////////////////
    robotMsgTimer[robotNumber]->setTimerData(robotNumber,ROBOT_ACK_CATCHED);
    robotMsgTimer[robotNumber]->start(checkCatchedMsgInterval);
}

void SchedulerGUI::slotRobotCatched(uint robotNumber)
{
    slotControlPlate(robotNumber,PLATE_RELEASE);
    moduleStateArray[robotModuleNumber[robotNumber]]=MODULE_EMPTY;
    robotWorkStateArray[robotNumber]=ROBOT_CATCHED;
    updateUI();
    //////////stop the ack catched timer/////////////////
    robotMsgTimer[robotNumber]->setTimerData(robotNumber,ROBOT_CATCHED);
    robotMsgTimer[robotNumber]->stop();
}

void SchedulerGUI::slotAddAdapter(uint moduleState)
{
    for(uint i=0;i<robotNum;++i){
        if(robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
            robotWorkStateArray[i]=ROBOT_WAITING;
    }
    for(uint i=moduleNum-1;i>0;--i)
    {
        moduleStateArray[i]=moduleStateArray[i-1];
    }
    moduleStateArray[0]=moduleState;
    checkIsActivateConveyor();
    checkCatchable();
    updateModuleUI();
}

void SchedulerGUI::slotRobotOnLine(uint robotNumber)
{
    if(robotNumber>robotNum-1)
    {
        QMessageBox::critical(NULL,"WARNING","input robotnumber error!");
        exit(0);
    }
    robotWorkStateArray[robotNumber]=ROBOT_CONNECTED;
    lastRobotNumber=((int)robotNumber)>lastRobotNumber?robotNumber:lastRobotNumber;
    updateRobotUI();
}

void SchedulerGUI::slotRobotOffLine(uint robotNumber)
{
    if(robotWorkStateArray[robotNumber]==ROBOT_ACK_CATCHED)
    {
        slotRobotAckCatchedError(robotNumber);
        return;
    }
    robotWorkStateArray[robotNumber]=ROBOT_OFFLINE;
    removeRobotSocket(robotNumber);
    slotControlPlate(robotNumber,PLATE_RELEASE);
    if(robotNumber>robotNum-1)
    {
        QMessageBox::critical(NULL,"WARNING","input robotnumber error!");
        exit(0);
    }

    lastRobotNumber=-1;
    for(uint i=0;i<robotNum;++i)
    {
        if(robotWorkStateArray[i]!=ROBOT_OFFLINE&&robotWorkStateArray[i]!=ROBOT_ACK_CATCHED_ERROR)
            lastRobotNumber=i;
    }

    //    addRunMessage(QString::number(robotNumber,10)+" offline",Qt::red);
    //    addRunMessage("online robot amount:"+QString::number(onlineRobotNum,10),Qt::red);
    updateRobotUI();
}

void SchedulerGUI::slotRobotAckCatchedError(uint robotNumber)
{
    if(robotNumber>robotNum-1)
    {
        QMessageBox::critical(NULL,"WARNING","input robotnumber error!");
        exit(0);
    }

    slotControlPlate(robotNumber,PLATE_RELEASE);
    moduleStateArray[robotModuleNumber[robotNumber]]=MODULE_ERROR;
    robotWorkStateArray[robotNumber]=ROBOT_ACK_CATCHED_ERROR;
    robotMsgTimer[robotNumber]->stop();
    stopConveyor();
    slotSetAlarmStatus(START_ALARM,ALARM_FLASH);
    removeRobotSocket(robotNumber);

    lastRobotNumber=-1;
    for(uint i=0;i<robotNum;++i)
    {
        if(robotWorkStateArray[i]!=ROBOT_OFFLINE&&robotWorkStateArray[i]!=ROBOT_ACK_CATCHED_ERROR)
            lastRobotNumber=i;
    }
    addRunMessage(QString::number(robotNumber,10)+" ack catched error!",Qt::red);
    updateUI();
}

void SchedulerGUI::slotRobotRecognizeError(uint robotNumber)
{
    slotControlPlate(robotNumber,PLATE_RELEASE);
    if(moduleStateArray[robotModuleNumber[robotNumber]]==MODULE_UNRECOGNIZE_ONCETIME
            ||moduleStateArray[robotModuleNumber[robotNumber]]==MODULE_CATCHING)
    {
        robotWorkStateArray[robotNumber]=ROBOT_RECOGNIZE_ERROR;
        if(moduleStateArray[robotModuleNumber[robotNumber]]==MODULE_UNRECOGNIZE_ONCETIME)
            moduleStateArray[robotModuleNumber[robotNumber]]=MODULE_UNRECOGNIZE_TWICETIME;
        else
            moduleStateArray[robotModuleNumber[robotNumber]]=MODULE_UNRECOGNIZE_ONCETIME;
        addRunMessage("robot "+QString::number(robotNumber,10)+" unrecognize",Qt::red);
        statisticAdapter(robotNumber,ROBOT_RECOGNIZE_ERROR);
        updateUI();
    }
    robotMsgTimer[robotNumber]->stop();
}

void SchedulerGUI::slotRobotClearing(uint robotNumber)
{
    if(robotWorkStateArray[robotNumber]==ROBOT_WAITING)
    {
        robotWorkStateArray[robotNumber]=ROBOT_CLEARING;
        updateRobotUI();
    }
    else
        addRunMessage(QString::number(robotNumber,10)+" Robot receive a clear order when it isn't in the waitting status!",Qt::red);
}

void SchedulerGUI::slotControlPlate(uint robotNumber, uint status)
{
    gpio_pi.setIOStatus(robotPlateIO[robotNumber],status);
}

void SchedulerGUI::slotSetConveyorStatus(uint status,uint direction)
{
    if(status==CONVEYOR_ACTIVATE)
        slotReleaseAllPlate();
    gpio_pi.setIOStatus(conveyorStatusIO,status);
}

void SchedulerGUI::slotReleaseAllPlate()
{
    for(uint i=0;i<robotNum;++i)
    {
        gpio_pi.setIOStatus(robotPlateIO[i],PLATE_RELEASE);
    }
}

void SchedulerGUI::slotPressAllPlate()
{
    for(uint i=0;i<robotNum;++i)
    {
        gpio_pi.setIOStatus(robotPlateIO[i],PLATE_PRESS);
    }
}

void SchedulerGUI::slotSetAlarmStatus(bool status,uint alarmFlag)
{
    alarmStatus=status;
    if(status==START_ALARM)
    {
        if(alarmFlag==ALARM_CONTINUOUS){
            if(!checkResetSignalTimer.isActive())
            {
                checkResetSignalTimer.setInterval(checkResetButtonInterval);
                checkResetSignalTimer.start();
            }
        }
        gpio_pi.setIOStatus(alarmStatusIO,START_ALARM);
        addRunMessage("start alarm",Qt::red);
        if(alarmFlag==ALARM_FLASH)
        {
            if(!alarmTimer.isActive())
            {
                alarmTimer.setInterval(flashAlarmInterval);
                alarmTimer.start();
            }
        }
    }
    else
    {
        addRunMessage("stop alarm",Qt::blue);
        checkResetSignalTimer.stop();
        alarmTimer.stop();
        gpio_pi.setIOStatus(alarmStatusIO,STOP_ALARM);
    }
}

void SchedulerGUI::slotAlarmTimerTimeout()
{
    if(alarmStatus==START_ALARM)
    {
        gpio_pi.setIOStatus(alarmStatusIO,STOP_ALARM);
        alarmStatus=STOP_ALARM;
    }
    else
    {
        gpio_pi.setIOStatus(alarmStatusIO,START_ALARM);
        alarmStatus=START_ALARM;
    }
}

void SchedulerGUI::slotNewConnection()
{
    QTcpSocket* clientSocket=server->nextPendingConnection();
    Socket* robotSocket=new Socket;
    robotSocket->setSocket(clientSocket);

    QString socketIP="";
    socketIP=clientSocket->peerAddress().toString();
    socketIP=socketIP.split(":").last();
    int newRobotNumber=-1;
    for(uint i=0;i<robotNum;++i)
    {
        if(socketIP==robotIP[i])
        {
            newRobotNumber=i;
            robotSocket->setSocketFlag(newRobotNumber);
            addRobotSocket(newRobotNumber,robotSocket);
            slotRobotOnLine(newRobotNumber);
            connect(robotSocket,SIGNAL(readyToRead(uint)),this,SLOT(slotReceiveMsg(uint)));
            connect(robotSocket,SIGNAL(disconnectServer(uint)),this,SLOT(slotDisconnect(uint)));
            break;
        }
    }

    ///////////if there is a old robot connect ,remove it//////////
    if(newRobotNumber==-1)
    {
        addRunMessage("A new connection from a uncertain ip:"+socketIP,Qt::red);
        clientSocket->close();
    }
}

void SchedulerGUI::slotReceiveMsg(uint robotNumber)
{
    QString msg;
    if(!robotSocketArray[robotNumber]->readMsg(msg))
    {
        slotRobotOffLine(robotNumber);
        addRunMessage("error occured when read msg from robot!",Qt::red);
    }
    processMsgFromRobot(robotNumber,msg);
}

void SchedulerGUI::slotDisconnect(uint robotNumber)
{
    addRunMessage("robot "+QString::number(robotNumber,10)+" disconnect from server!",Qt::red);
    slotRobotOffLine(robotNumber);
}

void SchedulerGUI::slotSendMsg(uint robotNumber,QString msg)
{
    if(!robotSocketArray[robotNumber]->sendMsg(msg))
    {
        slotRobotOffLine(robotNumber);
        addRunMessage(QString::number(robotNumber,10)+" error occuered when send msg to robot!",Qt::red);
    }
}

void SchedulerGUI::checkCatchable()
{
    ///////////confirm which robot can catch adapter///////////////////////////////////////
    if(
            (!isValidModule)
            ||detectModuleTime>maxDetectModuleTime
            ||detectModuleTime==0
            ||isEmergencyStop
            )
        return;


    bool isSendCatchOrderNow=true;
    std::vector<uint>nowCanCatchRobotArray;
    ///calculate how much robot can catch now///////
    for(uint i=0;i<robotNum;++i)
    {
        if(
                (
                    moduleStateArray[robotModuleNumber[i]]==MODULE_LOADED||moduleStateArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME
                    )
                &&robotWorkStateArray[i]==ROBOT_WAITING
                )
        {
            nowCanCatchRobotArray.push_back(i);
        }
    }

    if(!(
                moduleStateArray[robotModuleNumber[lastRobotNumber]]==MODULE_LOADED
                ||(!isConveyorRunning)
                ||(
                    moduleStateArray[robotModuleNumber[lastRobotNumber]]==MODULE_UNRECOGNIZE_ONCETIME
                    &&robotWorkStateArray[lastRobotNumber]!=ROBOT_RECOGNIZE_ERROR
                    )
                ))
    {
        calculateOptimizeCatchTime(isSendCatchOrderNow,nowCanCatchRobotArray);
    }


    if(isSendCatchOrderNow)
    {
        if(isConveyorRunning){
            QTime delayTimer;
            delayTimer.start();
            while(delayTimer.elapsed()<stopConveyorDelayTime);
            stopConveyor();
        }
        for(uint i=0;i<nowCanCatchRobotArray.size();++i){
            slotControlPlate(nowCanCatchRobotArray[i],PLATE_PRESS);
            sendCatchOrder(nowCanCatchRobotArray[i]);
        }
    }
}

void SchedulerGUI::calculateOptimizeCatchTime(bool &isCatchNow, std::vector<uint> &nowCanCatchRobotArray)
{
    std::vector<uint>nextCanCatchRobotArray;
    std::vector<uint>nNextCanCatchRobotArray;
    std::vector<uint>nnNextCanCatchRobotArray;

    /////calculate the next module status array/////
    uint* nextModuleStatusArray=new uint[moduleNum];
    for(uint i=moduleNum-1;i>0;--i)
    {
        nextModuleStatusArray[i]=moduleStateArray[i-1];
    }
    nextModuleStatusArray[0]=MODULE_EMPTY;
    ////calculate the next time how many robot can catch////
    for(uint i=0;i<robotNum;++i)
    {
        if(
                (nextModuleStatusArray[robotModuleNumber[i]]==MODULE_LOADED
                 ||nextModuleStatusArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                &&(robotWorkStateArray[i]==ROBOT_WAITING||robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
                )
        {
            nextCanCatchRobotArray.push_back(i);
        }
    }
    delete[] nextModuleStatusArray;


    //////////////////////calculate the next next module status array////////////////////////////////
    if(!(
                moduleStateArray[robotModuleNumber[lastRobotNumber]-1]==MODULE_LOADED
                ||!isConveyorRunning
                ||moduleStateArray[robotModuleNumber[lastRobotNumber]-1]==MODULE_UNRECOGNIZE_ONCETIME
                )){

        uint* nNextModuleStatusArray=new uint[moduleNum];
        for(uint i=moduleNum-1;i>1;--i)
        {
            nNextModuleStatusArray[i]=moduleStateArray[i-2];
        }
        nNextModuleStatusArray[0]=MODULE_EMPTY;
        nNextModuleStatusArray[1]=MODULE_EMPTY;
        ////calculate the next next time how many robot can catch////
        for(uint i=0;i<robotNum;++i)
        {
            if(
                    (nNextModuleStatusArray[robotModuleNumber[i]]==MODULE_LOADED
                     ||nNextModuleStatusArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                    &&(robotWorkStateArray[i]==ROBOT_WAITING||robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
                    )
            {
                nNextCanCatchRobotArray.push_back(i);
            }
        }
        delete[] nNextModuleStatusArray;
    }


    //////////////////////calculate the next next next module status array////////////////////////////////
    if(!(
                moduleStateArray[robotModuleNumber[lastRobotNumber]-2]==MODULE_LOADED
                ||!isConveyorRunning
                ||moduleStateArray[robotModuleNumber[lastRobotNumber]-2]==MODULE_UNRECOGNIZE_ONCETIME
                )){

        uint* nnNextModuleStatusArray=new uint[moduleNum];
        for(uint i=moduleNum-1;i>2;--i)
        {
            nnNextModuleStatusArray[i]=moduleStateArray[i-3];
        }
        nnNextModuleStatusArray[0]=MODULE_EMPTY;
        nnNextModuleStatusArray[1]=MODULE_EMPTY;
        nnNextModuleStatusArray[2]=MODULE_EMPTY;
        ////calculate the next next next time how many robot can catch////
        for(uint i=0;i<robotNum;++i)
        {
            if(
                    (nnNextModuleStatusArray[robotModuleNumber[i]]==MODULE_LOADED
                     ||nnNextModuleStatusArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                    &&(robotWorkStateArray[i]==ROBOT_WAITING||robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
                    )
            {
                nnNextCanCatchRobotArray.push_back(i);
            }
        }
        delete[] nnNextModuleStatusArray;
    }


    if(
            (nextCanCatchRobotArray.size()>nowCanCatchRobotArray.size())
            ||(nNextCanCatchRobotArray.size()>nowCanCatchRobotArray.size())
            ||(nnNextCanCatchRobotArray.size()>nowCanCatchRobotArray.size())
            )
    {
        isCatchNow=false;
        //        addRunMessage("now can catch robot amount:"+QString::number(nowCanCatchRobotArray.size(),10),Qt::blue);
        //        addRunMessage("next can catch robot amount:"+QString::number(nextCanCatchRobotArray.size(),10),Qt::blue);
        //        addRunMessage("next next can catch robot amount:"+QString::number(nNextCanCatchRobotArray.size(),10),Qt::blue);
        //        addRunMessage("next next next can catch robot amount:"+QString::number(nnNextCanCatchRobotArray.size(),10),Qt::blue);
        //        addRunMessage("--------<^.^>---<^_^>---------",Qt::magenta);
    }

    //    for(uint i=0;i<robotNum;++i)
    //    {
    //        if(robotWorkStateArray[i]==ROBOT_WAITING&&(robotWaitQTimer[i].elapsed()>robotMaxWaitTime))
    //        {
    //            isCatchNow=false;
    //            break;
    //        }
    //    }
}

void SchedulerGUI::processMsgFromRobot(uint robotNumber,QString msg)
{
    //  "wait" ROBOT_WAITING
    //  "catched" ROBOT_CATCHED
    // "unrecognize" ROBOT_UNRECOGNIZE
    // "droppass" ROBOT_DROP_PASS
    // "clear" ROBOT_CLEARNING

    if(robotMsgToStatusArray.find(msg)==robotMsgToStatusArray.end())
        addRunMessage("robot "+QString::number(robotNumber,10)+" receive a error message:"+msg,Qt::red);
    switch (robotMsgToStatusArray[msg]){
    case ROBOT_WAITING:
    {
        slotRobotWaiting(robotNumber);
    }
        break;
    case ROBOT_CATCHED:
    {
        slotRobotCatched(robotNumber);
    }
        break;
    case ROBOT_RECOGNIZE_ERROR:
    {
        slotRobotRecognizeError(robotNumber);
    }
        break;
    case ROBOT_DROP_PASS:
    {
        //        statisticAdapter(robotNumber,ROBOT_DROP_PASS);
    }
        break;
    case ROBOT_CLEARING:
    {
        slotRobotClearing(robotNumber);
    }
        break;
    default:
    {
        addRunMessage("robot "+QString::number(robotNumber,10)+" receive a error message:"+msg,Qt::red);
        //        qDebug()<<"robot "+QString::number(robotNumber,10)+" receive a error message:"+msg;
    }
        break;
    }
}

void SchedulerGUI::statisticAdapter(uint robotNumber,uint flag)
{
    if(flag==ROBOT_RECOGNIZE_ERROR)
    {
        robotUnrecognizeNum[robotNumber]++;
        robotUnrecognizeNumLineEdit[robotNumber]->setText(QString::number(robotUnrecognizeNum[robotNumber],10));
    }
}

void SchedulerGUI::on_pushButton_clicked()
{
    if(checkCatchableTimer.isActive()){
        if(QMessageBox::Yes==QMessageBox::question(NULL,"Warning","Scheduler is running,open this will restore the scheduler's status?",QMessageBox::Yes,QMessageBox::No))
        {
            QString str=QInputDialog::getText(NULL,"Warning","Please input password to stop the Scheduler",QLineEdit::Password);
            if(str=="123456"){
                checkCatchableTimer.stop();
                checkResetSignalTimer.stop();
                stopConveyor();

                ///////////restore the module status array///////////
                for(uint i=0;i<moduleNum;++i){
                    moduleStateArray[i]=MODULE_INISTATUS;
                }
                ///////////restore the robot work status array///////
                closeServer();
                ioUI->show();
                addRunMessage("Open the io control interface!",Qt::red);
                ui->label_7->setStyleSheet("background:red");
            }
            else
            {
                QMessageBox::warning(NULL,"Warning","password error!");
            }
        }
    }
    else
        ioUI->show();
}

void SchedulerGUI::delay(uint msec)
{
    QTime pass=QTime::currentTime();
    QTime now;
    do
    {
        now=QTime::currentTime();
    }
    while(pass.msecsTo(now)<=msec);
}

void SchedulerGUI::addRunMessage(QString msg,const QColor&color)
{
    ui->textEdit->setTextColor(color);
    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"   "+ msg);
}

void SchedulerGUI::robotReplyMsgTimeOut(const uint &robotNumber)
{
    uint timerDescriptor;
    robotMsgTimer[robotNumber]->getTimerDescriptor(timerDescriptor);
    switch(timerDescriptor){
    case ROBOT_ACK_CATCHED:
        if(robotWorkStateArray[robotNumber]!=ROBOT_CATCHED){
            slotRobotAckCatchedError(robotNumber);
        }
        break;
    default:{

    }
        break;
    }
}

void SchedulerGUI::on_pushButton_3_clicked()
{
    clearError();
}

void SchedulerGUI::on_radioButton_clicked()
{
    if(!_isProductiveTwoModel)
    {
        int choose;
        choose=QMessageBox::question(NULL,"Warning","Are you sure to switch the model of product?",QMessageBox::Yes,QMessageBox::No);
        if(choose==QMessageBox::Yes)
        {
            QString str=QInputDialog::getText(NULL,"Warning","Please input password to stop the Scheduler",QLineEdit::Password);
            if(str=="123456"){
                QMessageBox::warning(NULL,"Warning","You need to restart the program in order to activate the change!");
                QString configFilePath="settings.ini";
                QSettings settings(configFilePath,QSettings::IniFormat);
                settings.setValue("runData/isProductiveTwoModel",true);
                _isProductiveTwoModel=true;
                //                addRunMessage("Is product two model:"+QString::number(isProductiveTwoModel),Qt::red);
                //                addRunMessage("Current choose model:"+QString::number(_isProductiveTwoModel),Qt::red);
            }
            else{
                QMessageBox::warning(NULL,"Warning","Password error!");
                ui->radioButton_2->setChecked(true);
            }
        }
        else
            ui->radioButton_2->setChecked(true);
    }
}

void SchedulerGUI::on_radioButton_2_clicked()
{
    if(_isProductiveTwoModel)
    {
        int choose;
        choose=QMessageBox::question(NULL,"Warning","Are you sure to switch the model of product?",QMessageBox::Yes,QMessageBox::No);
        if(choose==QMessageBox::Yes)
        {
            QString str=QInputDialog::getText(NULL,"Warning","Please input password to stop the Scheduler",QLineEdit::Password);
            if(str=="123456"){
                QMessageBox::warning(NULL,"Warning","You need to restart the program in order to activate the change!");
                QString configFilePath="settings.ini";
                QSettings settings(configFilePath,QSettings::IniFormat);
                settings.setValue("runData/isProductiveTwoModel",false);
                _isProductiveTwoModel=false;
                //                addRunMessage("Is product two model:"+QString::number(isProductiveTwoModel),Qt::red);
                //                addRunMessage("Current choose model:"+QString::number(_isProductiveTwoModel),Qt::red);
            }
            else{
                QMessageBox::warning(NULL,"Warning","Password error!");
                ui->radioButton->setChecked(true);
            }
        }
        else
            ui->radioButton->setChecked(true);
    }
}

void SchedulerGUI::on_pushButton_4_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Reset the conveyor?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        QString str=QInputDialog::getText(NULL,"Warning","Please input password to reset the conveyor(PASSWORD:qwer1234)",QLineEdit::Password);
        if(str=="qwer1234")
        {
            addRunMessage("Reset conveyor!",Qt::red);
            isStartCount=false;
            isValidModule=false;
            for(uint i=0;i<moduleNum;++i)
            {
                moduleStateArray[i]=MODULE_INISTATUS;
                updateModuleUI();
            }
        }
        else
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->radioButton->setChecked(true);
        }
    }
    else
        ui->radioButton->setChecked(true);
}

void SchedulerGUI::on_pushButton_2_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with robot A?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        QString str=QInputDialog::getText(NULL,"Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",QLineEdit::Password);
        if(str=="qwer1234")
        {
            slotRobotOffLine(0);
        }
        else
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_pushButton_5_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with robot B?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        QString str=QInputDialog::getText(NULL,"Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",QLineEdit::Password);
        if(str=="qwer1234")
        {
            slotRobotOffLine(1);
        }
        else
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_pushButton_6_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with robot C?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        QString str=QInputDialog::getText(NULL,"Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",QLineEdit::Password);
        if(str=="qwer1234")
        {
            slotRobotOffLine(2);
        }
        else
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_pushButton_7_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with robot D?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        QString str=QInputDialog::getText(NULL,"Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",QLineEdit::Password);
        if(str=="qwer1234")
        {
            slotRobotOffLine(3);
        }
        else
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_pushButton_8_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Clear Adapter ?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        QString str=QInputDialog::getText(NULL,"Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",QLineEdit::Password);
        if(str=="qwer1234")
        {
            for(uint i=0;i<robotNum;++i)
            {
                if(robotWorkStateArray[i]==ROBOT_WAITING)
                {
                    sendClearOrder(i);
                }
            }
        }
        else
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->radioButton->setChecked(true);
        }
    }
}
