#ifndef LOGHUB_H
#define LOGHUB_H

#include <QObject>

class LogHub : public QObject
{
    Q_OBJECT
public:
    explicit LogHub(QObject *parent = 0);

signals:

public slots:
    void message(QtMsgType severity, const char * message);
    void onFatal(const QString& message);

};

#endif // LOGHUB_H
