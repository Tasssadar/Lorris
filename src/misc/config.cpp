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

#include "../connection/connection.h"
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
    "general/last_update_check", // CFG_QUINT32_LAST_UPDATE_CHECK
};

static const quint32 def_quint32[CFG_QUINT32_NUM] =
{
    MAX_CON_TYPE,                // CFG_QUINT32_CONNECTION_TYPE
    0,                           // CFG_QUINT32_TAB_TYPE
    38400,                       // CFG_QUINT32_SERIAL_BAUD
    0,                           // CFG_QUINT32_SHUPITO_MODE
    0,                           // CFG_QUINT32_SHUPITO_PRG_SPEED
    0,                           // CFG_QUINT32_LANGUAGE
    0,                           // CFG_QUINT32_TERMINAL_FMT
    0,                           // CFG_QUINT32_TCP_PORT
    0,                           // CFG_QUINT32_TERMINAL_INPUT
    0,                           // CFG_QUINT32_PROXY_PORT
    1,                           // CFG_QUINT32_SHUPITO_VERIFY
    1000,                        // CFG_QUINT32_ANALYZER_PLAY_DEL
    0,                           // CFG_QUITN32_SHUPITO_TERM_FMT
    10,                          // CFG_QUINT32_ANALYZER_GRID_SIZE
    0,                           // CFG_QUINT32_LAST_UPDATE_CHECK
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
    "analyzer/graph_export_path", // CFG_STRING_GRAPH_EXPORT
    "general/font",               // CFG_STRING_APP_FONT
};

static const QString def_string[CFG_STRING_NUM] =
{
    "",                           // CFG_STRING_SERIAL_PORT
    "",                           // CFG_STRING_SHUPITO_PORT
    "",                           // CFG_STRING_HEX_FOLDER
    "",                           // CFG_STRING_ANALYZER_FOLDER
    "",                           // CFG_STRING_SHUPITO_HEX_FOLDER
    "app",                        // CFG_STRING_SHUPITO_TUNNEL
    "127.0.0.1",                  // CFG_STRING_TCP_ADDR
    "0",                          // CFG_STRING_PROXY_ADDR
    "",                           // CFG_STRING_ANALYZER_JS
    "",                           // CFG_STRING_TERMINAL_TEXTFILE
    "",                           // CFG_STRING_TERMINAL_SETTINGS
    "",                           // CFG_STRING_SHUPITO_TERM_SET
    "",                           // CFG_STRING_ANALYZER_IMPORT
    "",                           // CFG_STRING_WINDOW_PARAMS
    "",                           // CFG_STRING_GRAPH_EXPORT
    "",                           // CFG_STRING_APP_FONT
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
    "terminal/show_bootloader",   // CFG_BOOL_TERMINAL_SHOW_BOOTLOADER
    "terminal/show_warn",         // CFG_BOOL_TERMINAL_SHOW_WARN
    "shupito/show_flash_warn",    // CFG_BOOL_SHUPITO_SHOW_FLASH_WARN
    "general/auto_update",        // CFG_BOOL_AUTO_UPDATE
    "general/check_for_updates",  // CFG_BOOL_CHECK_FOR_UPDATE
    "general/load_last_session",  // CFG_BOOL_LOAD_LAST_SESSION
    "general/session_connect",    // CFG_BOOL_SESSION_CONNECT
    "general/portable",           // CFG_BOOL_PORTABLE
};

static const bool def_bool[CFG_BOOL_NUM] =
{
    true,                         // CFG_BOOL_SHUPITO_TUNNEL
    false,                        // CFG_BOOL_SHUPITO_SHOW_LOG
    true,                         // CFG_BOOL_SHUPITO_SHOW_FUSES
    false,                        // CFG_BOOL_SHUPITO_OVERVOLTAGE
    false,                        // CFG_BOOL_SHUPITO_TURNOFF_VCC
    true,                         // CFG_BOOL_SHUPITO_TRANSLATE_FUSES
    true,                         // CFG_BOOL_SHUPITO_HIDE_RESERVED
    true,                         // CFG_BOOL_ANALYZER_ENABLE_GRID,
    false,                        // CFG_BOOL_ANALYZER_SHOW_GRID,
    true,                         // CFG_BOOL_SHUPITO_SHOW_SETTINGS
    false,                        // CFG_BOOL_TERMINAL_SHOW_BOOTLOADER
    true,                         // CFG_BOOL_TERMINAL_SHOW_WARN
    true,                         // CFG_BOOL_SHUPITO_SHOW_FLASH_WARN
    false,                        // CFG_BOOL_AUTO_UPDATE
    true,                         // CFG_BOOL_CHECK_FOR_UPDATE
    false,                        // CFG_BOOL_LOAD_LAST_SESSION
    true,                         // CFG_BOOL_SESSION_CONNECT
    false,                        // CFG_BOOL_PORTABLE
};

static const QString keys_variant[CFG_VARIANT_NUM] =
{
    "general/connections",        // CFG_VARIANT_CONNECTIONS
};

static const QString keys_float[CFG_FLOAT_NUM] =
{
    "shupito/overvoltage_val",    // CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL
};

static const float def_float[CFG_FLOAT_NUM] =
{
    5.5f,                         // CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL
};

Config::Config()
{
    openSettings();
}

Config::~Config()
{
    delete m_settings;
}

void Config::openSettings()
{
    static QString cfgFileLocations[] =
    {
        "./data/config.ini",
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/config.ini"
    };

    QFile file;
    bool portable = false;
    for(quint8 i = 0; i < sizeof(cfgFileLocations)/sizeof(QString); ++i)
    {
        file.setFileName(cfgFileLocations[i]);
        if(file.open(QIODevice::ReadOnly))
        {
            portable = (i == 0);
            break;
        }
    }

    QString filename = cfgFileLocations[1];

    if(file.isOpen())
    {
        filename = file.fileName();
        file.close();
    }

    m_settings = new QSettings(filename, QSettings::IniFormat);

    set(CFG_BOOL_PORTABLE, portable);
}

void Config::closeSettings()
{
    delete m_settings;
    m_settings = NULL;
}

void Config::resetToDefault()
{
    for(int i = 0; i < CFG_QUINT32_NUM; ++i)
        set((cfg_quint32)i, def_quint32[i]);

    for(int i = 0; i < CFG_STRING_NUM; ++i)
        set((cfg_string)i, def_string[i]);

    for(int i = 0; i < CFG_BOOL_NUM; ++i)
    {
        if(i != CFG_BOOL_PORTABLE)
            set((cfg_bool)i, def_bool[i]);
    }

    for(int i = 0; i < CFG_VARIANT_NUM; ++i)
        set((cfg_variant)i, QVariant());

    for(int i = 0; i < CFG_FLOAT_NUM; ++i)
        set((cfg_float)i, def_float[i]);
}

quint32 Config::get(cfg_quint32 item)
{
    return (quint32)m_settings->value(keys_quint32[item], def_quint32[item]).toInt();
}

void Config::set(cfg_quint32 item, quint32 val)
{
    m_settings->setValue(keys_quint32[item], val);
}

QString Config::get(cfg_string item)
{
    return m_settings->value(keys_string[item], def_string[item]).toString();
}

void Config::set(cfg_string item, const QString &val)
{
    m_settings->setValue(keys_string[item], val);
}

bool Config::get(cfg_bool item)
{
    return m_settings->value(keys_bool[item], def_bool[item]).toBool();
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
    return m_settings->value(keys_float[item], def_float[item]).toFloat();
}

void Config::set(cfg_float item, float val)
{
    m_settings->setValue(keys_float[item], val);
}
