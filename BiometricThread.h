#ifndef BIOMETRICTHREAD_H
#define BIOMETRICTHREAD_H

#include <QThread>
#include <QDir>
#include <QImage>

class QFileSystemWatcher;
class FaceTracker;

class BiometricThread : public QThread
{
    Q_OBJECT
public:
    explicit BiometricThread(QObject *parent = 0);

    virtual void run();

signals:
    void incomingFace(QImage croppedFace);

public slots:
    void incomingFile();

protected:
    QImage cropAroundFace(const QImage& orig, const QRect& face);

    FaceTracker * m_faceTracker;
    QFileSystemWatcher * m_watcher;
    QDir m_incomingDir;
};

#endif // BIOMETRICTHREAD_H
