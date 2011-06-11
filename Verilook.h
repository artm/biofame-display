#ifndef VERILOOK_H
#define VERILOOK_H

#include <QThread>
#include <QImage>
#include <QSharedPointer>

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
    bool usesVerilook() const { return m_extractor != 0; }

signals:
    void incomingFace(const QImage& where, const QRect& face);

public slots:
    void setMinIOD(int value);
    void setMaxIOD(int value);
    void setConfidenceThreshold(double value);
    void setQualityThreshold(int value);
    void addDbFace(const QString& imgPath);
    void scrutinize(const QImage& image);

private:
    struct FaceTemplate {
        QString m_imgPath, m_tplPath;
        QByteArray m_data;

        FaceTemplate(const QString& imgPath,
                     const QString& tplPath,
                     const QByteArray& data);
    };
    typedef QSharedPointer<FaceTemplate> FaceTemplatePtr;

    HNLExtractor m_extractor;
    HNMatcher m_matcher;
    QList< FaceTemplatePtr > m_templates;
    static QString errorString(NResult result);
    static bool isOk(NResult result,
                     QString errorSuffix = QString(),
                     QString successMessage = QString());
};


#endif // VERILOOK_H
