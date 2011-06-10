#ifndef VERILOOK_H
#define VERILOOK_H

#include <QThread>
#include <QImage>

#include <NCore.h>
#include <NLExtractor.h>
#include <NLicensing.h>

class Verilook : public QThread {
    Q_OBJECT
public:
    explicit Verilook(QObject * parent);
    virtual ~Verilook();
    void findFaces(const QImage& frame, QList<QRect>& faces);
    bool usesVerilook() const { return m_extractor != 0; }

public slots:
    /* verilook specific */
    void setMinIOD(int value);
    void setMaxIOD(int value);
    void setConfidenceThreshold(double value);
    void setQualityThreshold(int value);

private:
    HNLExtractor m_extractor;


    static QString errorString(NResult result);
    static bool isOk(NResult result,
                     QString errorSuffix = QString(),
                     QString successMessage = QString());
};


#endif // VERILOOK_H
