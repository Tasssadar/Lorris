#ifndef ANALYZERMDI_H
#define ANALYZERMDI_H

#include <QMdiArea>

class AnalyzerMdi : public QMdiArea
{
    Q_OBJECT
public:
    explicit AnalyzerMdi(QWidget *parent = 0);
    ~AnalyzerMdi();

signals:

public slots:

};

#endif // ANALYZERMDI_H
