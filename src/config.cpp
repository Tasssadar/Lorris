/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QSettings>
#include <QFile>
#include <QDesktopServices>
#include <qextserialport.h>

#include "connection/connection.h"
#include "config.h"

static const QString keys_quint32[CFG_QUINT32_NUM] =
{
    "general/connection_type",   // CFG_QUINT32_CONNECTION_TYPE
    "general/tab_type",          // CFG_QUINT32_TAB_TYPE
    "serial_port/baud_rate",     // CFG_QUINT32_SERIAL_BAUD
    "shupito/flash_mode",        // CFG_QUINT32_SHUPITO_MODE
    "shupito/prog_speed",        // CFG_QUINT32_SHUPITO_PRG_SPEED
    "general/language",          // CFG_QUINT32_LANGUAGE
    "terminal/format",           // CFG_QUINT32_TERMINAL_FMT
    "tcpsocket/port",            // CFG_QUINT32_TCP_PORT
    "terminal/input_handling",   // CFG_QUINT32_TERMINAL_INPUT
    "proxy/port",                // CFG_QUINT32_PROXY_PORT
    "shupito/verify_mode",       // CFG_QUINT32_SHUPITO_VERIFY
    "analyzer/play_delay",       // CFG_QUINT32_ANALYZER_PLAY_DEL
    "shupito/terminal_format",   // CFG_QUITN32_SHUPITO_TERM_FMT
    "analyzer/grid_size",        // CFG_QUINT32_ANALYZER_GRID_SIZE
};

static const QString keys_string[CFG_STRING_NUM] =
{
    "serial_port/port",           // CFG_STRING_SERIAL_PORT
    "shupito/port",               // CFG_STRING_SHUPITO_PORT
    "terminal/hex_folder",        // CFG_STRING_HEX_FOLDER
    "analyzer/data_folder",       // CFG_STRING_ANALYZER_FOLDER
    "shupito/hex_folder",         // CFG_STRING_SHUPITO_HEX_FOLDER
    "shupito/tunnel_name",        // CFG_STRING_SHUPITO_TUNNEL
    "tcpsocket/address",          // CFG_STRING_TCP_ADDR
    "proxy/address",              // CFG_STRING_PROXY_ADDR
    "analyzer/js_source",         // CFG_STRING_ANALYZER_JS
    "terminal/textfile",          // CFG_STRING_TERMINAL_TEXTFILE
    "terminal/settings",          // CFG_STRING_TERMINAL_SETTINGS
    "shupito/term_settings",      // CFG_STRING_SHUPITO_TERM_SET
    "analyzer/import_folder",     // CFG_STRING_ANALYZER_IMPORT
    "general/window_params",      // CFG_STRING_WINDOW_PARAMS
};

static const QString keys_bool[CFG_BOOL_NUM] =
{
    "shupito/enable_tunnel",      // CFG_BOOL_SHUPITO_TUNNEL
    "shupito/show_log",           // CFG_BOOL_SHUPITO_SHOW_LOG
    "shupito/show_fuses",         // CFG_BOOL_SHUPITO_SHOW_FUSES
    "shupito/overvoltage_warn",   // CFG_BOOL_SHUPITO_OVERVOLTAGE
    "shupito/turnoff_vcc",        // CFG_BOOL_SHUPITO_TURNOFF_VCC
    "shupito/translate_fuses",    // CFG_BOOL_SHUPITO_TRANSLATE_FUSES
    "shupito/hide_reserved",      // CFG_BOOL_SHUPITO_HIDE_RESERVED
    "analyzer/enable_grid",       // CFG_BOOL_ANALYZER_ENABLE_GRID,
    "analyzer/show_grid",         // CFG_BOOL_ANALYZER_SHOW_GRID,
    "shupito/show_settings",      // CFG_BOOL_SHUPITO_SHOW_SETTINGS
};

static const QString keys_variant[CFG_VARIANT_NUM] =
{
    "general/connections",        // CFG_VARIANT_CONNECTIONS
};

static const QString keys_float[CFG_FLOAT_NUM] =
{
    "shupito/overvoltage_val",    // CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL
};

Config::Config()
{
    openSettings();

    // Fill default values
    m_def_quint32[CFG_QUINT32_CONNECTION_TYPE]     = MAX_CON_TYPE;
    m_def_quint32[CFG_QUINT32_TAB_TYPE]            = 0;
    m_def_quint32[CFG_QUINT32_SERIAL_BAUD]         = 38400;
    m_def_quint32[CFG_QUINT32_SHUPITO_MODE]        = 0;
    m_def_quint32[CFG_QUINT32_SHUPITO_PRG_SPEED]   = 0;
    m_def_quint32[CFG_QUINT32_LANGUAGE]            = 0;
    m_def_quint32[CFG_QUINT32_TERMINAL_FMT]        = 0;
    m_def_quint32[CFG_QUINT32_TCP_PORT]            = 0;
    m_def_quint32[CFG_QUINT32_TERMINAL_INPUT]      = 0;
    m_def_quint32[CFG_QUINT32_PROXY_PORT]          = 0;
    m_def_quint32[CFG_QUINT32_SHUPITO_VERIFY]      = 1;
    m_def_quint32[CFG_QUINT32_ANALYZER_PLAY_DEL]   = 1000;
    m_def_quint32[CFG_QUITN32_SHUPITO_TERM_FMT]    = 0;
    m_def_quint32[CFG_QUINT32_ANALYZER_GRID_SIZE]  = 10;

    m_def_string[CFG_STRING_SERIAL_PORT]           = "";
    m_def_string[CFG_STRING_SHUPITO_PORT]          = "";
    m_def_string[CFG_STRING_HEX_FOLDER]            = "";
    m_def_string[CFG_STRING_ANALYZER_FOLDER]       = "";
    m_def_string[CFG_STRING_SHUPITO_HEX_FOLDER]    = "";
    m_def_string[CFG_STRING_SHUPITO_TUNNEL]        = "app";
    m_def_string[CFG_STRING_TCP_ADDR]              = "127.0.0.1";
    m_def_string[CFG_STRING_PROXY_ADDR]            = "0";
    m_def_string[CFG_STRING_ANALYZER_JS]           = "";
    m_def_string[CFG_STRING_TERMINAL_TEXTFILE]     = "";
    m_def_string[CFG_STRING_TERMINAL_SETTINGS]     = "";
    m_def_string[CFG_STRING_SHUPITO_TERM_SET]      = "";
    m_def_string[CFG_STRING_ANALYZER_IMPORT]       = "";
    m_def_string[CFG_STRING_WINDOW_PARAMS]         = "";

    m_def_bool[CFG_BOOL_SHUPITO_TUNNEL]            = true;
    m_def_bool[CFG_BOOL_SHUPITO_SHOW_LOG]          = false;
    m_def_bool[CFG_BOOL_SHUPITO_SHOW_FUSES]        = true;
    m_def_bool[CFG_BOOL_SHUPITO_OVERVOLTAGE]       = false;
    m_def_bool[CFG_BOOL_SHUPITO_TURNOFF_VCC]       = false;
    m_def_bool[CFG_BOOL_SHUPITO_TRANSLATE_FUSES]   = true;
    m_def_bool[CFG_BOOL_SHUPITO_HIDE_RESERVED]     = true;
    m_def_bool[CFG_BOOL_ANALYZER_ENABLE_GRID]      = true;
    m_def_bool[CFG_BOOL_ANALYZER_SHOW_GRID]        = false;
    m_def_bool[CFG_BOOL_SHUPITO_SHOW_SETTINGS]     = true;

    m_def_float[CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL] = 5.5f;
}

Config::~Config()
{
    delete m_settings;
}

void Config::openSettings()
{
    static const QString cfgFileLocations[] =
    {
        "./config.ini",
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/config.ini"
    };

    QFile file;
    for(quint8 i = 0; i < sizeof(cfgFileLocations)/sizeof(QString); ++i)
    {
        file.setFileName(cfgFileLocations[i]);
        if(file.open(QIODevice::ReadOnly))
            break;
    }

    QString filename = cfgFileLocations[1];

    if(file.isOpen())
    {
        filename = file.fileName();
        file.close();
    }

    m_settings = new QSettings(filename, QSettings::IniFormat);
}

quint32 Config::get(cfg_quint32 item)
{
    return (quint32)m_settings->value(keys_quint32[item], m_def_quint32[item]).toInt();
}

void Config::set(cfg_quint32 item, quint32 val)
{
    m_settings->setValue(keys_quint32[item], val);
}

QString Config::get(cfg_string item)
{
    return m_settings->value(keys_string[item], m_def_string[item]).toString();
}

void Config::set(cfg_string item, const QString &val)
{
    m_settings->setValue(keys_string[item], val);
}

bool Config::get(cfg_bool item)
{
    return m_settings->value(keys_bool[item], m_def_bool[item]).toBool();
}

void Config::set(cfg_bool item, bool val)
{
    m_settings->setValue(keys_bool[item], val);
}

QVariant Config::get(cfg_variant item)
{
    return m_settings->value(keys_variant[item]);
}

void Config::set(cfg_variant item, QVariant const & val)
{
    m_settings->setValue(keys_variant[item], val);
}

float Config::get(cfg_float item)
{
    return m_settings->value(keys_float[item], m_def_float[item]).toFloat();
}

void Config::set(cfg_float item, float val)
{
    m_settings->setValue(keys_float[item], val);
}
