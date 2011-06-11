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
    void newImagePath(QString path);

public slots:
    void incomingFile();
    void onFaceDetected(QImage image, QRect face);

protected:
    virtual void run();

    QImage cropAroundFace(const QImage& orig, const QRect& face);
    void loadDb();

    Verilook * m_verilook;
    QFileSystemWatcher * m_watcher;
    QDir m_incomingDir;
};

#endif // BIOMETRICTHREAD_H
