/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#define QT_USE_FAST_CONCATENATION

#include <QLabel>
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QSignalMapper>
#include <QStringBuilder>
#include <QScriptEngine>

#include "numberwidget.h"
#include "../../ui/floatinginputdialog.h"

void NumberWidget::addEnum()
{
    REGISTER_ENUM(NUM_UINT8);
    REGISTER_ENUM(NUM_UINT16);
    REGISTER_ENUM(NUM_UINT32);
    REGISTER_ENUM(NUM_UINT64);

    REGISTER_ENUM(NUM_INT8);
    REGISTER_ENUM(NUM_INT16);
    REGISTER_ENUM(NUM_INT32);
    REGISTER_ENUM(NUM_INT64);

    REGISTER_ENUM(NUM_FLOAT);
    REGISTER_ENUM(NUM_DOUBLE);

    REGISTER_ENUM(FMT_DECIMAL);
    REGISTER_ENUM(FMT_EXPONENT);
    REGISTER_ENUM(FMT_HEX);
    REGISTER_ENUM(FMT_BINARY);
    REGISTER_ENUM(FMT_COUNT);
}

REGISTER_DATAWIDGET(WIDGET_NUMBER, Number, &NumberWidget::addEnum)
W_TR(QT_TRANSLATE_NOOP("DataWidget", "Number"))

NumberWidget::NumberWidget(QWidget *parent) : DataWidget(parent)
{
    m_num = new QLabel("0", this);
    m_num->setAlignment(Qt::AlignCenter);

    m_digits = 2;

    // FIXME
    //m_num->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    QFont font = Utils::getMonospaceFont(4);
    m_num->setFont(font);
    layout->addWidget(m_num);

    adjustSize();
    setMinimumSize(size());
    resize(150, 80);
}

NumberWidget::~NumberWidget()
{

}

void NumberWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    setUseErrorLabel(true);

    m_numberType = NUM_UINT8;
    m_format = FMT_DECIMAL;
    m_level = false;

    QMenu *bitsMenu = contextMenu->addMenu(tr("Data type"));
    QMenu *formatMenu = contextMenu->addMenu(tr("Format"));
    QMenu *precMenu = contextMenu->addMenu(tr("Precision"));

    static const QString dataTypes[] =
    {
        tr("unsigned 8bit"),
        tr("unsigned 16bit"),
        tr("unsigned 32bit"),
        tr("unsigned 64bit"),

        tr("signed 8bit"),
        tr("signed 16bit"),
        tr("signed 32bit"),
        tr("signed 64bit"),

        tr("float (4 bytes)"),
        tr("double (8 bytes)"),

        tr("Null-terminated string"),
    };

    QSignalMapper *signalMapBits = new QSignalMapper(this);

    for(quint8 i = 0; i < NUM_COUNT_WITH_STRING; ++i)
    {
        if(i == 4 || i == 8 || i == 10)
            bitsMenu->addSeparator();

        m_bitsAction[i] = new QAction(dataTypes[i], this);
        m_bitsAction[i]->setCheckable(true);
        bitsMenu->addAction(m_bitsAction[i]);
        signalMapBits->setMapping(m_bitsAction[i], i);
        connect(m_bitsAction[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
    }
    m_bitsAction[0]->setChecked(true);
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(setDataType(int)));

    static const QString formatStr[] =
    {
        tr("Decimal"),
        tr("Decimal (w/ exponent)"),
        tr("Hex"),
        tr("Binary")
    };

    QSignalMapper *signalMapFmt = new QSignalMapper(this);
    for(quint8 i = 0; i < FMT_COUNT; ++i)
    {
        m_fmtAction[i] = new QAction(formatStr[i], this);
        m_fmtAction[i]->setCheckable(true);
        formatMenu->addAction(m_fmtAction[i]);
        signalMapFmt->setMapping(m_fmtAction[i], i);
        connect(m_fmtAction[i], SIGNAL(triggered()), signalMapFmt, SLOT(map()));
    }
    m_fmtAction[FMT_DECIMAL]->setChecked(true);
    m_fmtAction[FMT_EXPONENT]->setEnabled(false);
    connect(signalMapFmt, SIGNAL(mapped(int)), SLOT(setFormat(int)));

    QSignalMapper *signalMapPrec = new QSignalMapper(this);
    for(int i = 0; i < PREC_COUNT-1; ++i)
    {
        m_precAct[i] = precMenu->addAction(tr("%1 digits").arg(i));
        m_precAct[i]->setCheckable(true);
        signalMapPrec->setMapping(m_precAct[i], i);
        connect(m_precAct[i], SIGNAL(triggered()), signalMapPrec, SLOT(map()));
    }

    connect(signalMapPrec, SIGNAL(mapped(int)), this, SLOT(setPrecisionAct(int)));

    m_precAct[m_digits]->setChecked(true);
    precMenu->addSeparator();
    m_precAct[PREC_COUNT-1] = precMenu->addAction(tr("Set precision..."));
    m_precAct[PREC_COUNT-1]->setCheckable(true);
    signalMapPrec->setMapping(m_precAct[PREC_COUNT-1], PREC_COUNT-1);
    connect(m_precAct[PREC_COUNT-1], SIGNAL(triggered()), signalMapPrec, SLOT(map()));

    m_levelAction = new QAction(tr("Level off"), this);
    m_levelAction->setCheckable(true);
    contextMenu->addAction(m_levelAction);
    connect(m_levelAction, SIGNAL(triggered()), this, SLOT(levelSelected()));

    QAction *formula = contextMenu->addAction(tr("Set formula..."));
    connect(formula, SIGNAL(triggered()), SLOT(showFormulaDialog()));

    connect(&m_eval, SIGNAL(setError(bool,QString)), SLOT(setError(bool,QString)));
}

void NumberWidget::processData(analyzer_data *data)
{
    if(m_numberType != NUM_STRING) {
        QVariant var = DataWidget::getNumFromPacket(data, m_info.pos, m_numberType);
        setValue(var);
    } else {
        setValue(data->getString(m_info.pos));
    }
}

void NumberWidget::setValue(QVariant var)
{
    if(var.isNull())
    {
        m_num->setText("N/A");
        return;
    }

    if(var.type() == QVariant::String) {
        m_num->setText(var.toString());
        return;
    }

    QString n;

    static const char fmt[] = { 'f', 'e' };
    static const quint8 base[] = { 10, 10, 16, 2 };

    if(m_eval.isActive())
    {
        // .toString() on char type makes one
        // character string instead of value transcript
        if ((int)var.type() == QMetaType::QChar ||
            (int)var.type() == QMetaType::UChar)
            var.convert(QVariant::Int);

        QVariant res = m_eval.evaluate(var.toString());
        if(res.isValid())
        {
            switch(res.type())
            {
                case QVariant::Int:
                    n.setNum(res.toInt(), base[m_format]);
                    break;
                case QVariant::Double:
                    n.setNum(res.toDouble(), fmt[m_format], m_digits);
                    break;
                default:
                    n = res.toString();
                    break;
            }
        }
    }
    else
    {
        if(m_numberType < NUM_INT8)        n.setNum(var.toULongLong(), base[m_format]);
        else if(m_numberType < NUM_FLOAT)  n = var.toString();
        else                             n.setNum(var.toDouble(), fmt[m_format], m_digits);
    }

    switch(m_format)
    {
        case FMT_DECIMAL:
        case FMT_EXPONENT:
        {
            if(!m_level)
                break;

            static const quint8 levelPos[] =
            {
                3,  //NUM_INT8,  NUM_UINT8,
                5,  //NUM_INT16, NUM_UINT16,
                10, //NUM_INT32, NUM_UINT32,
                19, //NUM_INT64, NUM_UINT64,
                0,  //NUM_FLOAT
                0,  //NUM_DOUBLE
            };

            quint8 len = levelPos[m_numberType >= 4 ? m_numberType - 4 : m_numberType];
            prependZeros(n, len);
            break;
        }
        case FMT_HEX:
        {
            if(m_level)
                prependZeros(n, (1 << (m_numberType%4))*2);
            n = "0x" % n.toUpper();
            break;
        }
        case FMT_BINARY:
        {
            if(m_level)
                prependZeros(n, (1 << (m_numberType%4))*8);
            n.prepend("0b");
            break;
        }
    }

    m_num->setText(n);
}

void NumberWidget::setFormat(int fmt)
{
    if(fmt < 0 || fmt >= FMT_COUNT)
        return;

    for(quint8 y = 0; y < FMT_COUNT; ++y)
        m_fmtAction[y]->setChecked(y == fmt);
    m_format = fmt;
    emit updateForMe();
}

void NumberWidget::setDataType(int i)
{
    for(quint8 y = 0; y < NUM_COUNT_WITH_STRING; ++y)
        m_bitsAction[y]->setChecked(y == i);

    if(i >= NUM_INT8 && m_numberType < NUM_FLOAT)
        setFormat(FMT_DECIMAL);

    if(m_numberType == i)
        return;

    m_numberType = i;

    m_fmtAction[FMT_HEX]->setEnabled(i < NUM_INT8);
    m_fmtAction[FMT_BINARY]->setEnabled(i < NUM_INT8);
    m_fmtAction[FMT_EXPONENT]->setEnabled(i >= NUM_FLOAT);
    emit updateForMe();
}

void NumberWidget::levelSelected()
{
    m_level = !m_level;
    m_levelAction->setChecked(m_level);
    emit updateForMe();
}

void NumberWidget::resizeEvent(QResizeEvent *event)
{
    QSize old = event->oldSize();
    if(event->oldSize().height() < minimumHeight())
        old = minimumSize();

    QFont f = m_num->font();
    f.setPointSize(f.pointSize() + event->size().height() - old.height());
    m_num->setFont(f);
    DataWidget::resizeEvent(event);
}

void NumberWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    // data type
    file->writeBlockIdentifier("numWType");
    *file << m_numberType;

    // Format
    file->writeBlockIdentifier("numWFormat");
    *file << m_format;

    // Level off
    file->writeBlockIdentifier("numWLevel");
    *file << m_level;

    // formula
    file->writeBlockIdentifier("numWFormula");
    *file << m_eval.getFormula();

    // precision
    file->writeBlockIdentifier("numWPrec");
    *file << m_digits;
}

void NumberWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    // data type
    if(file->seekToNextBlock("numWType", BLOCK_WIDGET))
    {
        *file >> m_numberType;
        setDataType(m_numberType);
    }

    // Format
    if(file->seekToNextBlock("numWFormat", BLOCK_WIDGET))
    {
        *file >> m_format;
        setFormat(m_format);
    }

    // Level off
    if(file->seekToNextBlock("numWLevel", BLOCK_WIDGET))
    {
        *file >> m_level;
        m_levelAction->setChecked(m_level);
    }

    // Formula
    if(file->seekToNextBlock("numWFormula", BLOCK_WIDGET))
        m_eval.setFormula(file->readString());

    // Precision
    if(file->seekToNextBlock("numWPrec", BLOCK_WIDGET))
    {
        *file >> m_digits;
        int idx = std::min((int)m_digits, PREC_COUNT-1);
        for(int i = 0; i < PREC_COUNT; ++i)
            m_precAct[i]->setChecked(idx == i);
    }
}

void NumberWidget::prependZeros(QString &n, quint8 len)
{
    if(!len)
        return;

    bool negative = n.contains('-');
    quint8 numLen = n.length() - quint8(negative);

    if(numLen >= len)
        return;

    n.insert(int(negative), QString("%1").arg("", len - numLen, '0'));
}

void NumberWidget::setFormula(const QString &formula)
{
    m_eval.setFormula(formula);
    emit updateForMe();
}

void NumberWidget::showFormulaDialog()
{
    m_eval.showFormulaDialog();
    emit updateForMe();
}

void NumberWidget::setPrecisionAct(int idx)
{
    for(int i = 0; i < PREC_COUNT; ++i)
        m_precAct[i]->setChecked(idx == i);

    if(idx < PREC_COUNT-1)
        setPrecision(idx);
    else
    {
        int res = FloatingInputDialog::getInt(tr("Decimal digits:"), m_digits, 0, 255);
        if(res != m_digits)
            setPrecision(res);
    }
}

void NumberWidget::setPrecision(quint8 digits)
{
    m_digits = digits;
    emit updateForMe();
}
