#include "BioDisplay.h"
#include "BiometricThread.h"
#include "CommonTypes.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QRegExp>

template<class X>
X random(QList<X> lst)
{
    return lst[ random() % lst.size() ];
}

template<class Key, class X>
X random(QHash<Key, X> map)
{
    return map[random( map.keys() )];
}

BioDisplay::BioDisplay(QWidget *parent)
    : QMainWindow(parent)
{
    setupTimers();
    setupUI();
    QApplication::processEvents();
    setupBiometricThread();
}

BioDisplay::~BioDisplay()
{
    if (m_bioThread->isRunning()) {
        // proper thread destruction
        m_bioThread->quit();
        if (m_bioThread->wait( 2000 ))
            delete m_bioThread;
    }
}

void BioDisplay::setupUI()
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
    m_faceH = h*19/30, m_faceW = m_faceH / Bio::CROP_RATIO;
    m_vertSpace = h/2 - m_faceH/2;
    int hmargin = (w - h*4/3)/2;
    int hgap = (w - hmargin*2 - 3*m_faceW)/2;

    // big portraits
    QGraphicsRectItem * rectItem = m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(QColor(50,50,50)) );
    rectItem->setPos( hmargin + m_faceW/2 ,h/2);
    m_currentPortrait = new QGraphicsPixmapItem(rectItem);
    m_currentPortrait->setOffset( -m_faceW/2, -m_faceH/2 );
    m_currentPortrait->setFlag(QGraphicsItem::ItemStacksBehindParent);

    rectItem =  m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(QColor(50,50,50)) );
    rectItem->setPos( hmargin + m_faceW*3/2 + hgap ,h/2);
    m_matchPortrait = new QGraphicsPixmapItem(rectItem);
    m_matchPortrait->setOffset( -m_faceW/2, -m_faceH/2 );
    m_matchPortrait->setFlag(QGraphicsItem::ItemStacksBehindParent);

    rectItem =  m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(QColor(50,50,50)) );
    rectItem->setPos( w - hmargin - m_faceW/2, h/2);
    m_origPortrait = new QGraphicsPixmapItem(rectItem);
    m_origPortrait->setOffset( -m_faceW/2, -m_faceH/2 );
    m_origPortrait->setFlag(QGraphicsItem::ItemStacksBehindParent);

    // small portraits
    m_smallFaceH = m_vertSpace*3/4, m_smallFaceW = m_smallFaceH / Bio::CROP_RATIO;
    int nSmallFaces = (w - hmargin*2) / m_smallFaceW - 1;
    double smallFaceGap = (double)(w-hmargin*2-m_smallFaceW) / (nSmallFaces-1) - m_smallFaceW;
    for(int i = 0; i<nSmallFaces; i++) {
        QGraphicsRectItem * smallPortrait = m_scene->addRect( -m_smallFaceW/2, -m_smallFaceH/2, m_smallFaceW, m_smallFaceH,
                                                          QPen(QColor(50,50,50)) );
        smallPortrait->setPos(hmargin + m_smallFaceW/2 + (smallFaceGap + m_smallFaceW) * i, m_smallFaceH / 2);
        QGraphicsPixmapItem * item = new QGraphicsPixmapItem(smallPortrait);
        item->setOffset( -m_smallFaceW/2, -m_smallFaceH/2 );
        item->setFlag(QGraphicsItem::ItemStacksBehindParent);
        m_smallPortraits << item;
    }

    // text
    QFont fnt("TeX Gyre Adventor");
    fnt.setStretch(QFont::Condensed);
    fnt.setPixelSize( m_vertSpace / 6 );

    int textgap = hgap;
    int textwidth = (w-2*hmargin-2*textgap)/3;

    for(int i=0; i<3; i++) {
        m_text[i] = m_scene->addText( "", fnt );
        m_text[i]->setTextWidth(textwidth);
        m_text[i]->setPos( hmargin + (textwidth+textgap)*i, h - m_vertSpace );
        m_text[i]->setDefaultTextColor(QColor(100,100,100));
    }

    fnt.setPixelSize( m_vertSpace / 8 );
    fnt.setStretch(QFont::Condensed);
    QFontMetrics fm(fnt);
    QGraphicsSimpleTextItem * txt = m_scene->addSimpleText("Matching History", fnt);
    txt->setPos( hmargin, m_smallFaceH - fm.descent()); // not sure why we need to subtract descent, but it works
    txt->setBrush(QBrush(QColor(100,100,100)));
}

void BioDisplay::setCaption(const QString &text)
{
    m_text[1]->setHtml(QString("<center>%1</center>").arg(text));
}

void BioDisplay::searchAnimation()
{
    setCaption("searching...");
    m_rndPicTimer.start();
}

void BioDisplay::showNoMatch()
{
    setCaption("No match found");
    m_rndPicTimer.stop();
}

void BioDisplay::showMatch( const QString& slot, const QList<Bio::Portrait>& faces )
{
    m_rndPicTimer.stop();

    QString timeFormat = "dd-MM-yyyy";

    QString tag = slot;
    QString lang = "English";
    QRegExp tag_lang("(.*) ([^ ]*)$");
    if (tag_lang.exactMatch(slot)) {
        tag = tag_lang.cap(1);
        lang = tag_lang.cap(2);
    }

    setCaption(slot);

    Q_ASSERT(faces.size()>1);
    showPic(QPixmap::fromImage(faces[1].image));
    showPic(QPixmap::fromImage(faces.last().image), m_origPortrait);

    // show the rest of the slot...
    int i = 0;
    foreach(Bio::Portrait face, faces) {
        showSmallPic(QPixmap::fromImage(face.image), i++);
    }

    // text
    m_text[0]->setPlainText((tag + "\n\nFeeding back to internet").toUpper());
    m_text[1]->setPlainText(QString("Match: generation %1\nAdded to database: %2\nMatching Percentage: %3%")
                            .arg( faces.length()-1 ).arg(faces[1].timestamp.toString(timeFormat)).arg(random()%100).toUpper());
    m_text[2]->setPlainText(QString("Source: Google Images\nSearch Tag: %1\nSearch Language: %2").arg(tag).arg(lang).toUpper());
}


void BioDisplay::setupTimers()
{
    m_rndPicTimer.setInterval(50);
    Q_ASSERT( connect(&m_rndPicTimer, SIGNAL(timeout()), SLOT(showRndPic())) );

    m_textBlinkTimer.setInterval(250);
    Q_ASSERT( connect(&m_textBlinkTimer, SIGNAL(timeout()), SLOT(textBlink())) );
}

void BioDisplay::showRndPic()
{
    showPic( random( m_allFiles ) );
}

void BioDisplay::showPic(const QPixmap& pic, QGraphicsPixmapItem * where )
{
    if (!where)
        where = m_matchPortrait;
    where->setPixmap(pic.scaled(m_faceW,m_faceH));
}

void BioDisplay::showPic(const QString &path, QGraphicsPixmapItem * where )
{
    showPic(QPixmap( path ), where);
}

bool BioDisplay::showSmallPic(const QPixmap &image, int i)
{
    if (i >= m_smallPortraits.size()) return false;
    m_smallPortraits[i]->setPixmap( image.scaled(m_smallFaceW,m_smallFaceH) );
    return true;
}

void BioDisplay::incomingFace(QImage face)
{
    showPic(QPixmap::fromImage( face ), m_currentPortrait);
    searchAnimation();
}

void BioDisplay::setupBiometricThread()
{
    m_bioThread = new BiometricThread(this);

    Q_ASSERT( connect( m_bioThread, SIGNAL(incomingFace(QImage)), SLOT(incomingFace(QImage)) ) );
    // Q_ASSERT( connect( m_bioThread, SIGNAL(loadDbStarted()), SLOT(setCaptionLoading())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(dirReadStarted()), SLOT(setCaptionLoading())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(dirReadStarted()), &m_textBlinkTimer, SLOT(start())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(loadDbFinished()), SLOT(setCaptionLoaded())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(dirReadDone()), &m_textBlinkTimer, SLOT(stop())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(print(QString)), SLOT(setCaption(QString))));

    m_bioThread->start();
}

void BioDisplay::addImagePath(QString path)
{
    m_allFiles << path;
}

void BioDisplay::textBlink()
{
    QString caption = m_text[1]->toPlainText();
    QRegExp dots("\\.*$");
    Q_ASSERT( dots.indexIn(caption) > -1 );
    int numDots = (dots.matchedLength() + 1) % 4;

    // this doesn't do what I expect...
    // caption.replace(dots, QString(numDots, '.'));
    // ... which is:
    caption.remove(dots);
    caption += QString(numDots, '.');

    setCaption(caption);
}

