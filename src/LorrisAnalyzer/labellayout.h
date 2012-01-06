#ifndef LABELLAYOUT_H
#define LABELLAYOUT_H

#include <QHBoxLayout>
#include <QWidget>
#include <vector>
#include <QLabel>

class CmdTabWidget;
class DeviceTabWidget;

struct analyzer_header;
class QLabel;
class QString;
class QByteArray;
class QSpacerItem;

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

    QPoint getLabelPos(quint32 pos);

    CmdTabWidget *getCmdTab() { return cmd_w; }
    DeviceTabWidget *getDeviceTab() { return dev_w; }

public slots:
    void lenChanged(int len);
    void changePos(int this_label, int dragged_label);
    void UpdateTypes();

protected:
    void SetLabelType(QLabel *label, quint8 type);
    virtual quint8 GetTypeForPos(quint32 pos);

    std::vector<QLabel*> m_labels;
    analyzer_header *m_header;

private:
    QSpacerItem *m_spacer;
    bool m_enableReorder;
    bool m_enableDrag;

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

class DraggableLabel : public QLabel
{
    Q_OBJECT
Q_SIGNALS:
    void changePos(int this_label, int dragged_label);

public:
    DraggableLabel(const QString & text, bool drop = false, bool drag = false,
                   LabelLayout *l = NULL, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~DraggableLabel();

protected:
    void mousePressEvent ( QMouseEvent * event );
    void dragEnterEvent( QDragEnterEvent *event );
    void dragLeaveEvent( QDragLeaveEvent *event );
    void dropEvent( QDropEvent *event );

private:
    bool m_drag;
    bool m_drop;
    LabelLayout *layout;
};

#endif // LABELLAYOUT_H
