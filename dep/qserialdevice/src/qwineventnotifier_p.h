/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWINEVENTNOTIFIER_P_H
#define QWINEVENTNOTIFIER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qobject.h"
#include "QtCore/qt_windows.h"

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QWinEventNotifier : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QObject)

public:
    explicit QWinEventNotifier(QObject *parent = 0);
    explicit QWinEventNotifier(HANDLE hEvent, QObject *parent = 0);
    ~QWinEventNotifier();

    void setHandle(HANDLE hEvent);
    HANDLE handle() const;

    bool isEnabled() const;

public Q_SLOTS:
    void setEnabled(bool enable);

Q_SIGNALS:
    void activated(HANDLE hEvent);

protected:
    bool event(QEvent * e);

private:
    Q_DISABLE_COPY(QWinEventNotifier)

    HANDLE handleToEvent;
    bool enabled;
};

QT_END_NAMESPACE

#endif // QWINEVENTNOTIFIER_P_H
