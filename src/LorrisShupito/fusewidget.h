/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FUSEWIDGET_H
#define FUSEWIDGET_H

#include <QFrame>
#include <vector>
#include "../shared/chipdefs.h"

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
    void writeFuses();

public:
    explicit FuseWidget(QWidget *parent = 0);
    ~FuseWidget();

    void setFuses(chip_definition& chip);
    void clear(bool addButton = false);

    std::vector<quint8>& getFuseData() { return m_fuse_data; }

    bool isChanged() { return m_changed; }
    bool isLoaded() { return !m_fuses.empty(); }

    chip_definition& getChipDef() { return m_chip; }

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

    void translateFuseName(fuse_line *line);
    void addFuseOpt(fuse_line *line, const QString& bin);

    QVBoxLayout *m_layout;
    QFormLayout *m_fuse_layout;
    QPushButton *readFusesBtn;

    chip_definition m_chip;

    std::vector<fuse_line*> m_fuses;
    std::vector<quint8> m_fuse_data;

    QMenu *contextMenu;
    QAction *rememberAct;
    QAction *writeAct;

    bool m_changed;
};

#endif // FUSEWIDGET_H
