#ifndef BIOMETRICTHREAD_H
#define BIOMETRICTHREAD_H

#include <QThread>
#include <QDir>
#include <QImage>

class QFileSystemWatcher;
class Verilook;

class BiometricThread : public QThread
{
    Q_OBJECT
public:
    explicit BiometricThread(QObject *parent = 0);    

signals:
    void incomingFace(QImage croppedFace);
    void loadDbStarted();
    void loadDbFinished();
    void print(QString message);

public slots:
    void incomingFile();

protected:
    virtual void run();

    void loadDb(const QString& path);

    Verilook * m_verilook;
    QFileSystemWatcher * m_watcher;
    QDir m_root, m_incomingDir;
};

#endif // BIOMETRICTHREAD_H
