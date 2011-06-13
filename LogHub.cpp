#include "LogHub.h"
#include <QMessageBox>

LogHub * LogHub::s_loghub = 0;

LogHub::LogHub(const QString& appName)
    : m_appName(appName)
{
    if (s_loghub) {
        qCritical("Only one instance of LogHub is allowed");
        // should have aborted already, but just in case
        abort();
    } else {
        s_loghub = this;
        qInstallMsgHandler(LogHub::msgHandler);
    }
}

void LogHub::msgHandler(QtMsgType severity, const char * message)
{
    if (s_loghub)
        s_loghub->message(severity, message);
    else {
        fprintf(stderr, "LogHub inconsistensy: handler installed but singleton is NULL");
        abort();
    }
}

void LogHub::onFatal(const QString &message)
{
    QMessageBox::critical( 0, "", m_appName + " critical error:\n" + message);
    exit(1);
}

void LogHub::message(QtMsgType severity, const char *message)
{
    switch(severity) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", message);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", message);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", message);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", message);
        onFatal(message);
        break;
    }
}
