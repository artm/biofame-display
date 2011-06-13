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
    void identified( QString slotName, QStringList ancestorsTail);
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
    class FaceTemplate {
    public:
        // FIXME: if we ever are to remove face templates on the fly
        // this has to be restored. Now it causes double free-ing of the pointer
        // (at exit, the only time face templates are destroyed at the moment)
        //typedef QSharedPointer<FaceTemplate> Ptr;
        typedef FaceTemplate * Ptr;

        // parse existing img filename and attach loaded data
        FaceTemplate( const QString& imgPath, const QByteArray& data );
        // use extracted data and derive metadata from parent
        FaceTemplate( const QByteArray& data, const Ptr parent );
        const QByteArray& data() const { return m_data; }
        const QString& imgPath() const { return m_imgPath; }
        const QString& slot() const { return m_slot; }
        QStringList ancestors() const;
        void save() const;

        static QDir s_newFacesDir;

    protected:
        QString m_imgPath;
        QString m_slot;
        int m_parentId, m_id, m_gen;
        QByteArray m_data;

        static QHash< QString, int > s_slotCounts; // how many images per slot?
        static QHash< QString, QHash< int, Ptr> > s_slots;
    };

    void saveTemplate( const FaceTemplate::Ptr face );
    QByteArray compressTemplate( HNLTemplate tpl );

    static QImage cropAroundFace(const QImage& orig, const QRect& face);
    static QString errorString(NResult result);
    static bool isOk(NResult result,
                     QString errorSuffix = QString(),
                     QString successMessage = QString());

    HNLExtractor m_extractor;
    HNMatcher m_matcher;
    QList< FaceTemplate::Ptr > m_templates;
};


#endif // VERILOOK_H
