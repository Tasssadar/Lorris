#ifndef FUSEWIDGET_H
#define FUSEWIDGET_H

#include <QFrame>
#include <vector>
#include "chipdefs.h"

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QComboBox;
class QFormLayout;
class QMenu;

class FuseWidget : public QFrame
{
    Q_OBJECT
Q_SIGNALS:
    void readFuses();
    void status(const QString& text);

public:
    explicit FuseWidget(QWidget *parent = 0);
    ~FuseWidget();

    void setFuses(std::vector<chip_definition::fuse>& fuses);
    void clear();

    std::vector<quint8>& getFuseData() { return m_fuse_data; }

protected:
    void contextMenuEvent( QContextMenuEvent * event );

private slots:
    void changed(int index);
    void rememberFuses();

private:
    struct fuse_line
    {
        QLabel *label;
        QComboBox *box;
        chip_definition::fuse fuse;
    };

    QVBoxLayout *m_layout;
    QFormLayout *m_fuse_layout;
    QPushButton *readFusesBtn;

    std::vector<fuse_line*> m_fuses;
    std::vector<quint8> m_fuse_data;

    QMenu *contextMenu;
    QAction *rememberAct;
    QAction *writeAct;

    bool m_changed;
};

#endif // FUSEWIDGET_H
