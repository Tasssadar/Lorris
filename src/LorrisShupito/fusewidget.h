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

#ifndef FUSEWIDGET_H
#define FUSEWIDGET_H

#include <QFrame>
#include <vector>
#include "shared/chipdefs.h"

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

    void setFuses(std::vector<chip_definition::fuse>& fuses);
    void clear();

    std::vector<quint8>& getFuseData() { return m_fuse_data; }

    bool isChanged() { return m_changed; }
    bool isLoaded() { return !m_fuses.empty(); }

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
