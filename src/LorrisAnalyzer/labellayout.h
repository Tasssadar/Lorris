/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LABELLAYOUT_H
#define LABELLAYOUT_H

#include <QHBoxLayout>
#include <QWidget>
#include <vector>

class FilterTabWidget;

struct analyzer_header;
class analyzer_data;
class QLabel;
class QString;
class QByteArray;
class QSpacerItem;
class DraggableLabel;

enum DataFormat
{
    FORMAT_HEX    = 0,
    FORMAT_BYTE   = 1,
    FORMAT_STRING = 2
};

class LabelLayout : public QHBoxLayout
{
    Q_OBJECT
Q_SIGNALS:
    void orderChanged();

public:
    explicit LabelLayout(analyzer_header *header, bool enable_reorder, bool enable_drag,
                         FilterTabWidget *filters = NULL, QWidget *parent = 0);
    ~LabelLayout();

    void ClearLabels();
    void AddLabel(QString value, qint8 type);
    void RemoveLabel(quint16 index);
    void RemoveLabel()
    {
        RemoveLabel(quint16(m_labels.size()-1));
    }
    void RemoveLabel(quint8 type)
    {
        for(quint16 i = 0; i < m_labels.size(); ++i)
        {
            quint8 typep = GetTypeForPos(i);
            if(typep == type)
            {
                RemoveLabel(i);
                return;
            }
        }
    }

    quint32 GetLabelCount()
    {
        return m_labels.size();
    }

    QString getLabelText(quint32 index);
    int getLabelPos(DraggableLabel *label);

    bool setHightlightLabel(quint32 pos, bool highlight);

    FilterTabWidget *getFilterTabs() { return m_filterWidget; }

    void setHeader(analyzer_header *header);

public slots:
    void lenChanged(int len);
    void changePos(int this_label, int dragged_label);
    void UpdateTypes();

protected:
    void SetLabelType(DraggableLabel *label, quint8 type);
    quint8 GetTypeForPos(quint32 pos);

    std::vector<DraggableLabel*> m_labels;
    std::vector<DraggableLabel*> m_freedLabels;
    analyzer_header *m_header;

    inline quint32 getFirstLabelPos(bool withLabelsSize)
    {
        quint32 res = m_spacer_l ? 1 : 0;
        if(withLabelsSize)
            res += m_labels.size();
        return res;
    }

private slots:
    void setLabelFreed(DraggableLabel *label);
    void freeLabels();

private:
    QSpacerItem *m_spacer_r;
    QSpacerItem *m_spacer_l;
    bool m_enableReorder;
    bool m_enableDrag;

    FilterTabWidget *m_filterWidget;
};

class ScrollDataLayout : public LabelLayout
{
    Q_OBJECT
public:
    explicit ScrollDataLayout(analyzer_header *header, bool enable_reorder, bool enable_drag,
                              FilterTabWidget *filters = NULL, QWidget *parent = 0);
    ~ScrollDataLayout();

    void SetData(analyzer_data *data);
    int getCurrentFmt() const { return m_format; }

public slots:
    void fmtChanged(int fmt);

private:
    quint8 m_format;

};

class DraggableLabel : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void changePos(int this_label, int dragged_label);

public:
    DraggableLabel(const QString & text, quint32 pos, bool drop = false,
                   bool drag = false, LabelLayout *l = NULL, QWidget *parent = 0, Qt::WindowFlags = 0);
    ~DraggableLabel();

    bool isHighligted() { return m_highlighted; }
    void setHighlighted(bool highlight);

    void setLabelStyleSheet(const QString &css);
    void setLabelText(const QString& text);
    void setPos(quint32 pos);

    QString GetText();

protected:
    void mousePressEvent ( QMouseEvent * event );
    void dragEnterEvent( QDragEnterEvent *event );
    void dragLeaveEvent( QDragLeaveEvent *event );
    void dropEvent( QDropEvent *event );

private:
    bool m_drag;
    bool m_drop;
    bool m_highlighted;
    LabelLayout *labelLayout;

    QLabel *valueLabel;
    QLabel *posLabel;
};

#endif // LABELLAYOUT_H
