#ifndef LOGHUB_H
#define LOGHUB_H

/*
 * Logging hub.
 *
 * 1. redirects qt debug / error output to glog.
 * 2. shows the fatal message in a QMessageBox before dying.
 */

#include <QString>
class QWidget;

class LogHub
{
public:
    LogHub(const QString& appName);

private:
    static void msgHandler(QtMsgType severity, const char * message);
    void message(QtMsgType severity, const char * message);
    void onFatal(const QString& message);

    static LogHub * s_loghub;
    QString m_appName;
};

#endif // LOGHUB_H
