#include "LogHub.h"

#include <QMessageBox>

static QtMsgHandler origMessageHandler = 0;
LogHub loghub;

void logHubMsgHandler(QtMsgType severity, const char * message)
{
    loghub.message(severity, message);
}

LogHub::LogHub(QObject *parent) : QObject(parent)
{
    origMessageHandler = qInstallMsgHandler(logHubMsgHandler);
}

void LogHub::onFatal(const QString &message)
{
    QMessageBox::critical( 0, "Insert your app name here", message, QMessageBox::Ok, QMessageBox::Ok );
    abort();
}

void LogHub::message(QtMsgType severity, const char *message)
{
    if (origMessageHandler)
        origMessageHandler(severity, message);

    switch(severity) {
    case QtDebugMsg:
        break;
    case QtWarningMsg:
        break;
    case QtCriticalMsg:
        break;
    case QtFatalMsg:
        onFatal(message);
        break;
    }
}
