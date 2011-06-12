#ifndef VERILOOK_H
#define VERILOOK_H

#include <QThread>
#include <QImage>
#include <QSharedPointer>
#include <QHash>
#include <QDir>

#include <NCore.h>
#include <NLExtractor.h>
#include <NLicensing.h>
#include <NMatcher.h>

class Verilook : public QThread {
    Q_OBJECT
public:
    explicit Verilook(QObject * parent);
    virtual ~Verilook();
    void findFaces(const QImage& frame, QList<QRect>& faces);
    void setNewFacesDir(const QString& path);

signals:
    void incomingFace(QImage face);
    void identified(QString imgPath);
    void faceAdded(QString imgPath);
    void noMatchFound();

public slots:
    void setMinIOD(int value);
    void setMaxIOD(int value);
    void setConfidenceThreshold(double value);
    void setQualityThreshold(int value);
    void addDbFace(const QString& imgPath);
    void scrutinize(const QImage& image);

private:
    static QString slotName(const QString& path, int * num = 0);
    void markSlot(const QString& imgPath); // increase slot use count
    void saveToSlot( const QString& slot, QImage image, HNLTemplate tpl );
    void saveTemplate( const QString& imgPath, const QString& tplPath, HNLTemplate tpl );

    struct FaceTemplate {
        QString m_imgPath, m_tplPath;
        QByteArray m_data;

        FaceTemplate(const QString& imgPath,
                     const QString& tplPath,
                     const QByteArray& data);
    };
    typedef QSharedPointer<FaceTemplate> FaceTemplatePtr;

    static QImage cropAroundFace(const QImage& orig, const QRect& face);
    static QString errorString(NResult result);
    static bool isOk(NResult result,
                     QString errorSuffix = QString(),
                     QString successMessage = QString());

    HNLExtractor m_extractor;
    HNMatcher m_matcher;
    QList< FaceTemplatePtr > m_templates;
    QHash< QString, int > m_slotCounts; // how many images per slot?
    QDir m_newFacesDir;
};


#endif // VERILOOK_H
