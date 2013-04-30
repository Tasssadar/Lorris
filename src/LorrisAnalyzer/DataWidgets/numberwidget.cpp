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
#include <QInputDialog>

#include "numberwidget.h"

static void addEnum()
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
}

REGISTER_DATAWIDGET(WIDGET_NUMBER, Number, &addEnum)

NumberWidget::NumberWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Number"));
    setIcon(":/dataWidgetIcons/num.png");

    m_widgetType = WIDGET_NUMBER;

    num = new QLabel("0", this);
    num->setAlignment(Qt::AlignCenter);

    m_digits = 2;

    // FIXME
    //num->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    QFont font = Utils::getMonospaceFont(20);
    num->setFont(font);
    layout->addWidget(num);

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

    numberType = NUM_UINT8;
    format = FMT_DECIMAL;
    level = false;

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
        tr("double (8 bytes)")
    };

    QSignalMapper *signalMapBits = new QSignalMapper(this);

    for(quint8 i = 0; i < NUM_COUNT; ++i)
    {
        if(i%4 == 0 && i != 0)
            bitsMenu->addSeparator();

        bitsAction[i] = new QAction(dataTypes[i], this);
        bitsAction[i]->setCheckable(true);
        bitsMenu->addAction(bitsAction[i]);
        signalMapBits->setMapping(bitsAction[i], i);
        connect(bitsAction[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
    }
    bitsAction[0]->setChecked(true);
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
        fmtAction[i] = new QAction(formatStr[i], this);
        fmtAction[i]->setCheckable(true);
        formatMenu->addAction(fmtAction[i]);
        signalMapFmt->setMapping(fmtAction[i], i);
        connect(fmtAction[i], SIGNAL(triggered()), signalMapFmt, SLOT(map()));
    }
    fmtAction[FMT_DECIMAL]->setChecked(true);
    fmtAction[FMT_EXPONENT]->setEnabled(false);
    connect(signalMapFmt, SIGNAL(mapped(int)), SLOT(fmtSelected(int)));

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

    levelAction = new QAction(tr("Level off"), this);
    levelAction->setCheckable(true);
    contextMenu->addAction(levelAction);
    connect(levelAction, SIGNAL(triggered()), this, SLOT(levelSelected()));

    QAction *formula = contextMenu->addAction(tr("Set formula..."));
    connect(formula, SIGNAL(triggered()), SLOT(showFormulaDialog()));

    connect(&m_eval, SIGNAL(setError(bool,QString)), SLOT(setError(bool,QString)));
}

void NumberWidget::processData(analyzer_data *data)
{
    QVariant var = DataWidget::getNumFromPacket(data, m_info.pos, numberType);
    setValue(var);
}

void NumberWidget::setValue(QVariant var)
{
    if(var.isNull())
    {
        num->setText("N/A");
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
                    n.setNum(res.toInt(), base[format]);
                    break;
                case QVariant::Double:
                    n.setNum(res.toDouble(), fmt[format], m_digits);
                    break;
                default:
                    n = res.toString();
                    break;
            }
        }
    }
    else
    {
        if(numberType < NUM_INT8)        n.setNum(var.toULongLong(), base[format]);
        else if(numberType < NUM_FLOAT)  n = var.toString();
        else                             n.setNum(var.toDouble(), fmt[format], m_digits);
    }

    switch(format)
    {
        case FMT_DECIMAL:
        case FMT_EXPONENT:
        {
            if(!level)
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

            quint8 len = levelPos[numberType >= 4 ? numberType - 4 : numberType];
            prependZeros(n, len);
            break;
        }
        case FMT_HEX:
        {
            if(level)
                prependZeros(n, (1 << (numberType%4))*2);
            n = "0x" % n.toUpper();
            break;
        }
        case FMT_BINARY:
        {
            if(level)
                prependZeros(n, (1 << (numberType%4))*8);
            n.prepend("0b");
            break;
        }
    }

    num->setText(n);
}

void NumberWidget::fmtSelected(int i)
{
    for(quint8 y = 0; y < FMT_COUNT; ++y)
        fmtAction[y]->setChecked(y == i);
    format = i;
    emit updateForMe();
}

void NumberWidget::setDataType(int i)
{
    for(quint8 y = 0; y < NUM_COUNT; ++y)
        bitsAction[y]->setChecked(y == i);

    if(i >= NUM_INT8 && numberType < NUM_FLOAT)
        fmtSelected(FMT_DECIMAL);

    if(numberType == i)
        return;

    numberType = i;

    fmtAction[FMT_HEX]->setEnabled(i < NUM_INT8);
    fmtAction[FMT_BINARY]->setEnabled(i < NUM_INT8);
    fmtAction[FMT_EXPONENT]->setEnabled(i >= NUM_FLOAT);
    emit updateForMe();
}

void NumberWidget::levelSelected()
{
    level = !level;
    levelAction->setChecked(level);
    emit updateForMe();
}

void NumberWidget::resizeEvent(QResizeEvent *event)
{
    QSize old = event->oldSize();
    if(event->oldSize().height() < minimumHeight())
        old = minimumSize();

    QFont f = num->font();
    f.setPointSize(f.pointSize() + event->size().height() - old.height());
    num->setFont(f);
    DataWidget::resizeEvent(event);
}

void NumberWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    // data type
    file->writeBlockIdentifier("numWType");
    file->write((char*)&numberType, sizeof(numberType));

    // Format
    file->writeBlockIdentifier("numWFormat");
    file->write((char*)&format, sizeof(format));

    // Level off
    file->writeBlockIdentifier("numWLevel");
    file->write((char*)&level, sizeof(level));

    // formula
    file->writeBlockIdentifier("numWFormula");
    file->writeString(m_eval.getFormula());

    // precision
    file->writeBlockIdentifier("numWPrec");
    file->writeVal(m_digits);
}

void NumberWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    // data type
    if(file->seekToNextBlock("numWType", BLOCK_WIDGET))
    {
        file->read((char*)&numberType, sizeof(numberType));
        setDataType(numberType);
    }

    // Format
    if(file->seekToNextBlock("numWFormat", BLOCK_WIDGET))
    {
        file->read((char*)&format, sizeof(format));
        fmtSelected(format);
    }

    // Level off
    if(file->seekToNextBlock("numWLevel", BLOCK_WIDGET))
    {
        file->read((char*)&level, sizeof(level));
        levelAction->setChecked(level);
    }

    // Formula
    if(file->seekToNextBlock("numWFormula", BLOCK_WIDGET))
        m_eval.setFormula(file->readString());

    // Precision
    if(file->seekToNextBlock("numWPrec", BLOCK_WIDGET))
    {
        m_digits = file->readVal<quint8>();
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
        int res = QInputDialog::getInt(this, tr("Number precision"), tr("Enter number of digits"), m_digits, 0, 255);
        if(res != m_digits)
            setPrecision(res);
    }
}

void NumberWidget::setPrecision(quint8 digits)
{
    m_digits = digits;
    emit updateForMe();
}

NumberWidgetAddBtn::NumberWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Number"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/num.png"));

    m_widgetType = WIDGET_NUMBER;
}
