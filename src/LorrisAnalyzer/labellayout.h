/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef LABELLAYOUT_H
#define LABELLAYOUT_H

#include <QHBoxLayout>
#include <QWidget>
#include <vector>
#include <QVector>

class CmdTabWidget;
class DeviceTabWidget;

struct analyzer_header;
struct data_widget_info;
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
                         CmdTabWidget *cmd = NULL, DeviceTabWidget *dev = NULL, QWidget *parent = 0);
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

    bool setHightlightLabel(const data_widget_info& info, bool highlight);

    CmdTabWidget *getCmdTab() { return cmd_w; }
    DeviceTabWidget *getDeviceTab() { return dev_w; }

    void setHeader(analyzer_header *header);

    const QVector<quint32>& getSelected() const { return m_selectedPos; }
    DraggableLabel* getLabel(quint32 pos) { return pos >= m_labels.size() ? NULL : m_labels[pos]; }
    quint32 selected(quint32 pos, bool selected);

public slots:
    void lenChanged(int len);
    void changePos(int this_label, int dragged_label);
    void UpdateTypes();
    void unselectAll();

protected:
    void SetLabelType(DraggableLabel *label, quint8 type);
    virtual quint8 GetTypeForPos(quint32 pos);

    std::vector<DraggableLabel*> m_labels;
    analyzer_header *m_header;

    inline quint32 getFirstLabelPos(bool withLabelsSize)
    {
        quint32 res = m_spacer_l ? 1 : 0;
        if(withLabelsSize)
            res += m_labels.size();
        return res;
    }

private:
    QSpacerItem *m_spacer_r;
    QSpacerItem *m_spacer_l;
    bool m_enableReorder;
    bool m_enableDrag;

    QVector<quint32> m_selectedPos;

    CmdTabWidget *cmd_w;
    DeviceTabWidget *dev_w;
};

class ScrollDataLayout : public LabelLayout
{
    Q_OBJECT
public:
    explicit ScrollDataLayout(analyzer_header *header, bool enable_reorder, bool enable_drag,
                              CmdTabWidget *cmd = NULL, DeviceTabWidget *dev = NULL, QWidget *parent = 0);
    ~ScrollDataLayout();

    void SetData(const QByteArray &data);

public slots:
    void fmtChanged(int len);

protected:
    quint8 GetTypeForPos(quint32 pos);

private:
    quint8 m_format;

};

class DraggableLabel : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void changePos(int this_label, int dragged_label);
    void unselectAll();

public:
    DraggableLabel(const QString & text, quint32 pos, bool drop = false,
                   bool drag = false, LabelLayout *l = NULL, QWidget *parent = 0, Qt::WindowFlags = 0);
    ~DraggableLabel();

    bool isHighligted() { return m_highlighted; }
    void setHighlighted(bool highlight)
    {
        m_highlighted = highlight;
        updateColor();
    }

    bool isSelected() const { return m_selected; }
    void setSelected(bool selected)
    {
        m_selected = selected;
        updateColor();
    }

    void updateColor();

    void setLabelStyleSheet(const QString &css);
    void setLabelText(const QString& text);
    void setPos(quint32 pos);
    quint32 getPos() const { return m_pos; }

    QString GetText();

protected:
    void mousePressEvent ( QMouseEvent * event );
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent( QDragEnterEvent *event );
    void dragLeaveEvent( QDragLeaveEvent *event );
    void dropEvent( QDropEvent *event );

private:
    void setOrder(quint32 order);

    bool m_drag;
    bool m_drop;
    bool m_highlighted;
    bool m_selected;
    bool m_draggin;
    LabelLayout *labelLayout;

    QLabel *valueLabel;
    QLabel *posLabel;
    quint32 m_pos;
    QString m_order;
};

#endif // LABELLAYOUT_H
