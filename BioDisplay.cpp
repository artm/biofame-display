#include "BioDisplay.h"
#include "FaceTracker.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QFileSystemWatcher>

template<class X>
X random(QList<X> lst)
{
    return lst[ random() % lst.size() ];
}

BioDisplay::BioDisplay(QWidget *parent)
    : QMainWindow(parent)
    , m_faceTracker(new FaceTracker(this))
{
    setupTimers();
    createUI();
    loadDB();
}

BioDisplay::~BioDisplay()
{

}

void BioDisplay::createUI()
{
    m_view = new QGraphicsView(this);
    setCentralWidget(m_view);

    m_scene = new QGraphicsScene(this);
    Q_ASSERT(m_scene);
    m_scene->setBackgroundBrush( QBrush(Qt::black) );
    m_view->setScene(m_scene);

    showFullScreen();
    m_scene->setSceneRect(0,0,width(),height());
    m_view->fitInView( m_scene->sceneRect() );

    int w = width(), h = height();
    m_faceH = h/2, m_faceW = m_faceH*2/3;
    m_vertSpace = h/2 - m_faceH/2;
    int hgap = (w - 2*m_faceW)/3;

    // big portraits
    QGraphicsRectItem * rectItem = m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(QColor(50,50,50)) );
    rectItem->setPos( hgap+m_faceW/2 ,h/2);
    m_currentPortrait = new QGraphicsPixmapItem(rectItem);
    m_currentPortrait->setOffset( -m_faceW/2, -m_faceH/2 );
    rectItem =  m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(QColor(50,50,50)) );
    rectItem->setPos(w - (hgap+m_faceW/2),h/2);
    m_matchPortrait = new QGraphicsPixmapItem(rectItem);
    m_matchPortrait->setOffset( -m_faceW/2, -m_faceH/2 );

    // text
    QFont fnt;
    fnt.setPixelSize( h/12 );
    m_caption = m_scene->addText( "", fnt );
    m_caption->setDefaultTextColor(QColor(100,100,100));
    m_caption->setTextWidth(w*2/3);
    setCaption("Fifteen Minutes of Biometric Fame");

    // small portraits
    int smallFaceH = m_vertSpace*3/4, smallFaceW = smallFaceH*2/3;
    int smallFaceMargin = (m_vertSpace-smallFaceH)/2;
    int nSmallFaces = (w - smallFaceMargin) / (smallFaceW+smallFaceMargin);
    int offs = smallFaceMargin+smallFaceW/2;
    for(int i = 0; i<nSmallFaces; i++) {
        QGraphicsRectItem * smallPortrait = m_scene->addRect( -smallFaceW/2, -smallFaceH/2, smallFaceW, smallFaceH,
                                                          QPen(QColor(50,50,50)) );
        smallPortrait->setPos(offs+i*(smallFaceMargin+smallFaceW),m_vertSpace/2);
        m_smallPortraits << new QGraphicsPixmapItem(smallPortrait);
    }
}

void BioDisplay::loadDB()
{
    QDir dbdir(QDir::homePath() +  "/Pictures/Faces/orig");
    Q_ASSERT(dbdir.exists());
    setCaption("Loading database...");
    QApplication::processEvents();
    foreach(QString path, dbdir.entryList(QStringList() << "*.jpg",QDir::Files)) {
        m_pics << dbdir.filePath(path);
    }
    setCaption("Database loaded");

    QString incomingPath = QDir::homePath() + "/Pictures/Faces/incoming";
    m_incomingDir = QDir(incomingPath);
    foreach(QString p, m_incomingDir.entryList(QDir::Files)) {
        QFile(m_incomingDir.filePath(p)).remove();
    }

    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(incomingPath);
    Q_ASSERT( connect(m_watcher, SIGNAL(directoryChanged(QString)), SLOT(incomingFile())) );
}

void BioDisplay::setCaption(const QString &text)
{
    m_caption->setHtml(QString("<center>%1</center>").arg(text));
    QSizeF tsz = m_caption->boundingRect().size();
    // vertically center under the portrait...
    m_caption->setPos(m_scene->width()/2 - tsz.width()/2, m_scene->height() - m_vertSpace/2 - tsz.height()/2);
}

void BioDisplay::searchAnimation()
{
    setCaption("searching...");
    m_rndPicTimer.start();
}

void BioDisplay::setupTimers()
{
    m_rndPicTimer.setInterval(50);
    Q_ASSERT( connect(&m_rndPicTimer, SIGNAL(timeout()), this, SLOT(showRndPic())) );
}

void BioDisplay::showRndPic()
{
    QString path = random( m_pics );
    QPixmap pic( path );
    m_matchPortrait->setPixmap(pic.scaled(m_faceW,m_faceH));
}

void BioDisplay::incomingFile()
{
    // maybe load
    QStringList allJpegs = m_incomingDir.entryList(QStringList()<<"*.jpg",QDir::Files,QDir::Time|QDir::Reversed);
    if (allJpegs.size()>0) {
        QImage incoming(m_incomingDir.filePath(allJpegs[0]));
        QList<QRect> faces;
        m_faceTracker->findFaces(incoming, faces);
        if (faces.size()>0) {
            QPixmap pic = QPixmap::fromImage( cropAroundFace(incoming,faces[0]) );
            m_currentPortrait->setPixmap( pic.scaled(m_faceW,m_faceH) );
            searchAnimation();
        }
    }
}

QImage BioDisplay::cropAroundFace(const QImage &orig, const QRect &face)
{
    int w = face.width() * 4 / 3, h = w * m_faceH / m_faceW;
    int dw = w - face.width(), dh = h - face.height();

    return orig.copy( face.x()-dw/2, face.y()-dh/2, w, h );
}
