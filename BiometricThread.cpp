#include "BiometricThread.h"
#include "Verilook.h"
#include "CommonTypes.h"

#include <QFileSystemWatcher>
#include <QApplication>
#include <QDebug>
#include <QTime>

QStringList BiometricThread::s_supportedImageTypes( QStringList() << "*.jpg" << "*.png" << "*.gif" );

BiometricThread::BiometricThread(QObject *parent)
    : QThread(parent)
{
    m_root = QDir(QDir::homePath() + "/Pictures/Faces");
    Q_ASSERT( m_root.exists() );

    m_verilook = new Verilook(this);
    QString newFacesPath = m_root.filePath("new");
    if (!QDir(newFacesPath).exists())
        Q_ASSERT(m_root.mkdir("new"));
    m_verilook->setNewFacesDir(newFacesPath);
    // retransmit incoming face signal
    Q_ASSERT( connect(m_verilook, SIGNAL(incomingFace(QImage)), parent, SLOT(incomingFace(QImage))) );
    Q_ASSERT( connect(m_verilook, SIGNAL(noMatchFound()), parent, SLOT(showNoMatch())) );
    Q_ASSERT( connect(m_verilook, SIGNAL(identified(QString, QList<Bio::Portrait>,double)), parent,
                      SLOT(showMatch(QString, QList<Bio::Portrait>,double))) );
    Q_ASSERT( connect( m_verilook, SIGNAL(faceAdded(QString)), parent, SLOT(addImagePath(QString)) ) );

    Q_ASSERT( connect(parent, SIGNAL(requestFakeMatch()), m_verilook, SLOT(fakeMatch())) );
    Q_ASSERT( connect(parent, SIGNAL(requestFakeNoMatch()), m_verilook, SLOT(fakeNoMatch())) );

    QString incomingPath = m_root.filePath("incoming");
    m_incomingDir = QDir(incomingPath);
    foreach(QString p, m_incomingDir.entryList(QDir::Files)) {
        QFile(m_incomingDir.filePath(p)).remove();
    }

    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(incomingPath);
    Q_ASSERT( connect(m_watcher, SIGNAL(directoryChanged(QString)), SLOT(onDirChanges())) );
}


void BiometricThread::onDirChanges()
{
    // maybe load
    // we only look for jpegs here, since the robot will send a jpeg
    QStringList allJpegs = m_incomingDir.entryList(QStringList() << "*.jpg",QDir::Files,QDir::Time|QDir::Reversed);
    if (allJpegs.size()>0) {
        QImage incoming(m_incomingDir.filePath(allJpegs[0]));

        // deinterlace
        incoming = incoming.scaledToHeight( incoming.height()/2 );

        // somehow this breaks it
        // if (incoming.isNull()) return;

        m_verilook->scrutinize( incoming );
        foreach(QString jpg, allJpegs) {
            m_incomingDir.remove(jpg);
        }
    }
}

void BiometricThread::run()
{
    emit loadDbStarted();
    loadDb(m_root.filePath("new"));
    loadDb(m_root.filePath("orig"));
    emit loadDbFinished();
    exec();
}

void BiometricThread::loadDb(const QString& path)
{
    QDir dbdir(path);
    Q_ASSERT(dbdir.exists());

    emit dirReadStarted();
    QFileInfoList lst = dbdir.entryInfoList(s_supportedImageTypes,QDir::Files);
    emit dirReadDone();
    int total = lst.size(), i = 0;
    QTime stopwatch;
    stopwatch.start();
    foreach(QFileInfo fi, lst) {
        m_verilook->addDbFace(fi.filePath());

        ++i;

        if (stopwatch.elapsed() > 50) {
            stopwatch.restart();
            double progress = 100.0 * i / total;
            emit print( QString("Loading database... %1%").arg(progress,5,'f',1) );
            QApplication::processEvents();
        }
    }
}

