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
#include <QKeyEvent>

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
    , m_feedProgress(0.0)
    , m_feedIncrement(2.57)
{
    loadTags();
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
    setCursor(Qt::BlankCursor);
    m_scene->setSceneRect(0,0,width(),height());
    m_view->fitInView( m_scene->sceneRect() );

    int w = width(), h = height();
    m_faceH = h*19/30, m_faceW = m_faceH / Bio::CROP_RATIO;
    m_vertSpace = h/2 - m_faceH/2;
    int hmargin = (w - h*4/3)/2;
    int hgap = (w - hmargin*2 - 3*m_faceW)/2;

    QColor rectColor = Qt::black, textColor = Qt::white;

    m_rootItem = new QGraphicsItemGroup(0, m_scene);

    // big portraits
    QGraphicsRectItem * rectItem = m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(rectColor) );
    rectItem->setParentItem(m_rootItem);
    rectItem->setPos( hmargin + m_faceW/2 ,h/2);
    m_currentPortrait = new QGraphicsPixmapItem(rectItem);
    m_currentPortrait->setOffset( -m_faceW/2, -m_faceH/2 );
    m_currentPortrait->setFlag(QGraphicsItem::ItemStacksBehindParent);

    rectItem =  m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(rectColor) );
    rectItem->setParentItem(m_rootItem);
    rectItem->setPos( hmargin + m_faceW*3/2 + hgap ,h/2);
    m_matchPortrait = new QGraphicsPixmapItem(rectItem);
    m_matchPortrait->setOffset( -m_faceW/2, -m_faceH/2 );
    m_matchPortrait->setFlag(QGraphicsItem::ItemStacksBehindParent);

    rectItem =  m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(rectColor) );
    rectItem->setParentItem(m_rootItem);
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
                                                          QPen(rectColor) );
        smallPortrait->setParentItem(m_rootItem);
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
        m_text[i]->setDefaultTextColor(textColor);
        m_text[i]->setParentItem(m_rootItem);

    }

    fnt.setPixelSize( m_vertSpace / 8 );
    fnt.setStretch(QFont::Condensed);
    QFontMetrics fm(fnt);
    m_histCaption = m_scene->addSimpleText("Matching History", fnt);
    m_histCaption->setPos( hmargin, m_smallFaceH); // not sure why we need to subtract descent, but it works
    m_histCaption->setBrush(QBrush(textColor));
    m_histCaption->setParentItem(m_rootItem);
    m_histCaption->hide();
}

void BioDisplay::hideDisplay()
{
    m_rootItem->hide();
}

void BioDisplay::showDisplay()
{
    m_rootItem->show();
}

void BioDisplay::setCaption(const QString &text, int delay)
{
    m_text[1]->setHtml(QString("<center>%1</center>").arg(text));

    if (delay > 0) {
        // uncancelable, don't want to fix now
        //QTimer::singleShot(delay,this,SLOT(clearCaption()));
    }
}

void BioDisplay::searchAnimation()
{
    setCaption("searching...");
    m_rndPicTimer.start();
}

void BioDisplay::showNoMatch()
{
    setCaption("No match found", 10000);
    m_text[0]->hide();
    m_text[2]->hide();
    m_histCaption->hide();

    m_rndPicTimer.stop();
}

void BioDisplay::showMatch( const QString& slot, const QList<Bio::Portrait>& faces, double score )
{
    m_rndPicTimer.stop();

    m_histCaption->show();

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
    showPic(QPixmap::fromImage(faces[0].image), m_currentPortrait);
    showPic(QPixmap::fromImage(faces[1].image), m_matchPortrait);
    showPic(QPixmap::fromImage(faces.last().image), m_origPortrait);

    // show the rest of the slot...
    int i = 0;
    foreach(Bio::Portrait face, faces) {
        showSmallPic(QPixmap::fromImage(face.image), i++);
    }

    // text
    m_text[0]->show();
    m_text[1]->show();
    m_text[2]->show();
    m_text[0]->setPlainText((tag + "\n\nFeeding back to internet: 0.0%").toUpper());
    m_text[1]->setPlainText(QString("Match: generation %1\nAdded to database: %2\nMatching Percentage: %3%")
                            .arg( faces.length()-1 ).arg(faces[1].timestamp.toString(timeFormat)).arg(score/180*100,3,'f',1).toUpper());
    m_text[2]->setPlainText(QString("Source: Google Images\nSearch Tag: %1\nSearch Language: %2")
                            .arg(translateTag(lang,tag))
                            .arg(lang)
                            .toUpper());

    m_feedProgress = 0.0;
    m_feedingTimer.start();
}

void BioDisplay::setPercents(int id, float value)
{
    QRegExp percent("[0-9.]+%");
    QString text = m_text[id]->toPlainText();

    if (! text.contains(percent))
        return;

    if (value < 100.0)
        m_text[id]->setPlainText( text.replace(percent, QString("%1%").arg(value,3,'f',1)));
    else
        m_text[id]->setPlainText( text.replace(percent, "done"));
}

void BioDisplay::setupTimers()
{
    m_rndPicTimer.setInterval(50);
    Q_ASSERT( connect(&m_rndPicTimer, SIGNAL(timeout()), SLOT(showRndPic())) );

    m_textBlinkTimer.setInterval(250);
    Q_ASSERT( connect(&m_textBlinkTimer, SIGNAL(timeout()), SLOT(textBlink())) );

    m_feedingTimer.setInterval(250);
    Q_ASSERT( connect(&m_feedingTimer, SIGNAL(timeout()), SLOT(feedSome())) );
}

void BioDisplay::showRndPic()
{
    showPic( random( m_allFiles ) );
}

void BioDisplay::showPic(const QPixmap& pic, QGraphicsPixmapItem * where )
{
    if (!where)
        where = m_matchPortrait;
    where->show();
    where->setPixmap(pic.scaled(m_faceW,m_faceH));
}

void BioDisplay::showPic(const QString &path, QGraphicsPixmapItem * where )
{
    showPic(QPixmap( path ), where);
}

bool BioDisplay::showSmallPic(const QPixmap &image, int i)
{
    if (i >= m_smallPortraits.size()) return false;
    m_smallPortraits[i]->show();
    m_smallPortraits[i]->setPixmap( image.scaled(m_smallFaceW,m_smallFaceH) );
    return true;
}

void BioDisplay::incomingFace(QImage face)
{
    showPic(QPixmap::fromImage( face ), m_matchPortrait);

    m_origPortrait->hide();
    m_currentPortrait->hide();

    foreach(QGraphicsPixmapItem* item, m_smallPortraits) {
        item->hide();
    }

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

void BioDisplay::clearCaption()
{
    setCaption("");
}

void BioDisplay::keyPressEvent(QKeyEvent * kev)
{
    switch (kev->key()) {
    case Qt::Key_F:
        if (kev->modifiers() & Qt::SHIFT)
            emit requestFakeNoMatch();
        else
            emit requestFakeMatch();
        break;
    case Qt::Key_Escape:
        if (isFullScreen())
            showMaximized();
        else
            showFullScreen();
        break;
    }
}

void BioDisplay::feedSome()
{
    m_feedProgress += m_feedIncrement;
    setPercents(0, m_feedProgress);
    if (m_feedProgress > 100.0)
        m_feedingTimer.stop();
}

void BioDisplay::loadTags()
{
    QDir tagDir(QDir::homePath() + "/Pictures/Faces/text");
    if (!tagDir.exists()) return;

    QFile tagFile(tagDir.filePath("ALL_TEXTS.txt"));
    if (!tagFile.exists()) return;

    QTextStream text(&tagFile);

    QRegExp langRe("^([A-Z]+):$"), tagRe("^\\[x\\]\t([^\t]+)(?:\t+([^\t]+))?$");

    tagFile.open(QIODevice::ReadOnly);
    QString lang;

    while(!text.atEnd()) {
        QString line = text.readLine();

        if (langRe.exactMatch(line)) {
            lang = langRe.cap(1);
        } else if (tagRe.exactMatch(line)) {
            QString tagEn = tagRe.cap(1);
            QString tag;

            if (lang=="ENGLISH") {
                tag = tagEn;
            } else {
                tag = tagRe.cap(2);
            }

            m_tags[ QString("%1:%2").arg(lang).arg(tagEn) ] = tag;

        }

    }
}

QString BioDisplay::translateTag(QString lang, QString tagEn)
{
    QString key = QString("%1:%2").arg(lang.toUpper()).arg(tagEn);

    return m_tags.value( key, tagEn );
}

