#include "BiometricThread.h"
#include "Verilook.h"
#include "CommonTypes.h"

#include <QFileSystemWatcher>
#include <QApplication>


BiometricThread::BiometricThread(QObject *parent)
    : QThread(parent)
    , m_verilook(new Verilook(this))
{
    QString incomingPath = QDir::homePath() + "/Pictures/Faces/incoming";
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
        QList<QRect> faces;
        m_verilook->findFaces(incoming, faces);
        emit incomingFace(cropAroundFace(incoming,faces[0]));

        // now we can extract the template...

    }
}

void BiometricThread::run()
{
    loadDb();
    exec();
}

QImage BiometricThread::cropAroundFace(const QImage &orig, const QRect &face)
{
    int w = face.width() * 4 / 3, h = w * Bio::CROP_RATIO;
    int dw = w - face.width(), dh = h - face.height();

    return orig.copy( face.x()-dw/2, face.y()-dh/2, w, h );
}

void BiometricThread::loadDb()
{
    QDir dbdir(QDir::homePath() +  "/Pictures/Faces/orig");
    Q_ASSERT(dbdir.exists());
    emit loadDbStarted();
    foreach(QString path, dbdir.entryList(QStringList() << "*.jpg",QDir::Files)) {
        QString fullPath = dbdir.filePath(path);
        emit newImagePath(fullPath);
        m_verilook->addDbFace(fullPath);
        QApplication::processEvents();
    }
    emit loadDbFinished();
}

