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
#include "../../shared/chipdefs.h"

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
    void clear(bool addButton = false, bool widgetsOnly = false);

    std::vector<quint8>& getFuseData() { return m_fuse_data; }

    bool isChanged() { return m_changed; }
    bool isLoaded() { return !m_fuses.empty(); }

    chip_definition& getChipDef() { return m_chip; }

    void enableButtons(bool enable);

protected:
    void contextMenuEvent( QContextMenuEvent * event );

private slots:
    void changed(int index);
    void rememberFuses();
    void translateFuses(bool checked);
    void hideReserved(bool checked);

private:
    struct fuse_line
    {
        QLabel *label;
        QComboBox *box;
        chip_definition::fuse fuse;
    };

    void translateFuseName(fuse_line *line);
    bool addFuseOpt(fuse_line *line, const QString& bin, std::vector<std::pair<QString, QVariant> > &list);
    void createReadFusesBtn();

    QVBoxLayout *m_layout;
    QFormLayout *m_fuse_layout;
    QPushButton *m_readFusesBtn;

    chip_definition m_chip;

    std::vector<fuse_line*> m_fuses;
    std::vector<quint8> m_fuse_data;

    QMenu *m_contextMenu;
    QAction *m_rememberAct;
    QAction *m_readAct;
    QAction *m_writeAct;
    QAction *m_translateFuseAct;
    QAction *m_hideReservedAct;

    bool m_changed;
    bool m_enableButtons;
};

#endif // FUSEWIDGET_H
