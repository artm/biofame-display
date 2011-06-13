#include "BiometricThread.h"
#include "Verilook.h"
#include "CommonTypes.h"

#include <QFileSystemWatcher>
#include <QApplication>
#include <QDebug>

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
    Q_ASSERT( connect(m_verilook, SIGNAL(identified(QStringList)), parent, SLOT(showMatch(QStringList))) );
    Q_ASSERT( connect( m_verilook, SIGNAL(faceAdded(QString)), parent, SLOT(addImagePath(QString)) ) );


    QString incomingPath = m_root.filePath("incoming");
    m_incomingDir = QDir(incomingPath);
    foreach(QString p, m_incomingDir.entryList(QDir::Files)) {
        QFile(m_incomingDir.filePath(p)).remove();
    }

    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(incomingPath);
    Q_ASSERT( connect(m_watcher, SIGNAL(directoryChanged(QString)), SLOT(incomingFile())) );
}


void BiometricThread::incomingFile()
{
    // maybe load
    QStringList allJpegs = m_incomingDir.entryList(QStringList()<<"*.jpg",QDir::Files,QDir::Time|QDir::Reversed);
    if (allJpegs.size()>0) {

        QImage incoming(m_incomingDir.filePath(allJpegs[0]));
        qDebug() << "Scrutinizing:" << allJpegs[0];
        m_verilook->scrutinize( incoming );
        foreach(QString jpg, allJpegs) {
            m_incomingDir.remove(jpg);
        }
    }
}

void BiometricThread::run()
{
    emit loadDbStarted();
    loadDb(m_root.filePath("orig"));
    loadDb(m_root.filePath("new"));
    emit loadDbFinished();
    exec();
}

void BiometricThread::loadDb(const QString& path)
{
    QDir dbdir(path);
    Q_ASSERT(dbdir.exists());
    foreach(QString path, dbdir.entryList(QStringList() << "*.jpg",QDir::Files)) {
        QString fullPath = dbdir.filePath(path);
        m_verilook->addDbFace(fullPath);
        QApplication::processEvents();
    }
}
