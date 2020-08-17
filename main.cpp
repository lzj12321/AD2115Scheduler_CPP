#include "schedulergui.h"
#include <QApplication>
#include<QMessageBox>
#include<QSharedMemory>
#include<QIODevice>
#include<QTextStream>
#include<QFile>
#include<QDir>
#include<QDateTime>
#include<QTime>

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString current_date_time=QTime::currentTime().toString("hh.mm.ss");
    QString message = QString("%1 %2 %3 %4").arg(current_date_time).arg(text).arg(context_info).arg(msg);

    QDir dir;
    if(!dir.exists("/home/pi/Documents/Scheduler/log")){
        dir.mkdir("/home/pi/Documents/Scheduler/log");
    }
    QString runlog="/home/pi/Documents/Scheduler/log/"+current_date+".log";
    QFile file(runlog);
    file.open(QIODevice::WriteOnly | QIODevice::Append|QIODevice::Text);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /////////////register the qdebug output log to file////////////
//    qInstallMessageHandler(outputMessage);

    /////////////confirm programming not running///////////////////
//    QSharedMemory singleon(a.applicationName());
//    if(!singleon.create(1)){
//        QMessageBox::warning(NULL,"Warning","program already runing!");
//        return false;
//    }

    SchedulerGUI w;
    w.show();
    return a.exec();
}
