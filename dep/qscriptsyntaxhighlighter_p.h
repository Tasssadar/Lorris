/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtSCriptTools module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSCRIPTSYNTAXHIGHLIGHTER_P_H
#define QSCRIPTSYNTAXHIGHLIGHTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

#ifndef QT_NO_SYNTAXHIGHLIGHTER

#include <QtGui/qsyntaxhighlighter.h>

#include <QtGui/qtextformat.h>

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT QScriptSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    QScriptSyntaxHighlighter(QTextDocument *document = 0);
    ~QScriptSyntaxHighlighter();

protected:
    void highlightBlock(const QString &text);

private:
    void highlightWord(int currentPos, const QString &buffer);

    enum ScriptFormats {
        ScriptTextFormat, ScriptNumberFormat,
        ScriptStringFormat, ScriptTypeFormat,
        ScriptKeywordFormat, ScriptPreprocessorFormat,
        ScriptLabelFormat, ScriptCommentFormat,
        NumScriptFormats
    };
    QTextCharFormat m_formats[NumScriptFormats];

private:
    Q_DISABLE_COPY(QScriptSyntaxHighlighter)
};

QT_END_NAMESPACE

#endif // QT_NO_SYNTAXHIGHLIGHTER

#endif

#ifndef QFUNCTIONS_P_H
#define QFUNCTIONS_P_H

#include <QtCore/qglobal.h>

#if defined(Q_OS_WINCE)
#  include "QtCore/qfunctions_wince.h"
#elif defined(Q_OS_VXWORKS)
#  include "QtCore/qfunctions_vxworks.h"
#elif defined(Q_OS_NACL)
#  include "QtCore/qfunctions_nacl.h"
#endif

#ifdef Q_CC_RVCT
// rvct doesn't see static operators when using our qalgorithms
#  define Q_STATIC_GLOBAL_OPERATOR inline
#  define Q_STATIC_GLOBAL_INLINE_OPERATOR inline
#else
#  define Q_STATIC_GLOBAL_OPERATOR static
#  define Q_STATIC_GLOBAL_INLINE_OPERATOR static inline
#endif

QT_BEGIN_HEADER
QT_END_HEADER

#endif
