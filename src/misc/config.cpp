/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QSettings>
#include <QFile>
#include <qextserialport.h>

#include "../connection/connection.h"
#include "config.h"
#include "utils.h"

static const QString keys_quint32[] =
{
    "main/connection_type",   // CFG_QUINT32_CONNECTION_TYPE
    "main/tab_type",          // CFG_QUINT32_TAB_TYPE
    "serial_port/baud_rate",     // CFG_QUINT32_SERIAL_BAUD
    "shupito/flash_mode",        // CFG_QUINT32_SHUPITO_MODE
    "shupito/prog_speed",        // CFG_QUINT32_SHUPITO_PRG_SPEED
    "main/language",          // CFG_QUINT32_LANGUAGE
    "terminal/format",           // CFG_QUINT32_TERMINAL_FMT
    "tcpsocket/port",            // CFG_QUINT32_TCP_PORT
    "terminal/input_handling",   // CFG_QUINT32_TERMINAL_INPUT
    "proxy/port",                // CFG_QUINT32_PROXY_PORT
    "shupito/verify_mode",       // CFG_QUINT32_SHUPITO_VERIFY
    "analyzer/play_delay",       // CFG_QUINT32_ANALYZER_PLAY_DEL
    "shupito/terminal_format",   // CFG_QUITN32_SHUPITO_TERM_FMT
    "analyzer/grid_size",        // CFG_QUINT32_ANALYZER_GRID_SIZE
    "main/last_update_check", // CFG_QUINT32_LAST_UPDATE_CHECK
    "analyzer/script_error_str", // CFG_QUINT32_SCRIPTEDITOR_STR
    "analyzer/script_edit_type", // CFG_QUINT32_SCRIPTEDITOR_TYPE
    "analyzer/script_engine",    // CFG_QUINT32_ANALYZER_SCRIPT_ENG
    "main/compress_block",    // CFG_QUINT32_COMPRESS_BLOCK
    "shupito/spi_tunnel_speed",  // CFG_QUINT32_SPI_TUNNEL_SPEED
    "shupito/spi_tunnel_modes",  // CFG_QUINT32_SPI_TUNNEL_MODES
    "main/freeze_timeout",    // CFG_QUINT32_SCRIPT_FREEZE_TIMEOUT
};

static const quint32 def_quint32[] =
{
    MAX_CON_TYPE,                // CFG_QUINT32_CONNECTION_TYPE
    0,                           // CFG_QUINT32_TAB_TYPE
    115200,                      // CFG_QUINT32_SERIAL_BAUD
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
    40,                          // CFG_QUINT32_SCRIPTEDITOR_STR
    UINT_MAX,                    // CFG_QUINT32_SCRIPTEDITOR_TYPE
    0,                           // CFG_QUINT32_ANALYZER_SCRIPT_ENG
    10*1024*1024,                // CFG_QUINT32_COMPRESS_BLOCK
    500000,                      // CFG_QUINT32_SPI_TUNNEL_SPEED
    0x200,                       // CFG_QUINT32_SPI_TUNNEL_MODES
    15000,                       // CFG_QUINT32_SCRIPT_FREEZE_TIMEOUT
};

static const QString keys_string[] =
{
    "serial_port/port",           // CFG_STRING_SERIAL_PORT
    "shupito/port",               // CFG_STRING_SHUPITO_PORT
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
    "main/window_params",      // CFG_STRING_WINDOW_PARAMS
    "analyzer/graph_export_path", // CFG_STRING_GRAPH_EXPORT
    "main/font",               // CFG_STRING_APP_FONT
    "analyzer/script_wnd_params", // CFG_STRING_SCRIPT_WND_PARAMS
    "proxy/tunnel_name",          // CFG_STRING_PROXY_TUNNEL_NAME
    "shupito/avr109_bootseq",     // CFG_STRING_AVR109_BOOTSEQ
    "shupito/zmodem_bootseq",     // CFG_STRING_ZMODEM_BOOTSEQ
};

static const QString def_string[] =
{
    "",                           // CFG_STRING_SERIAL_PORT
    "",                           // CFG_STRING_SHUPITO_PORT
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
    "",                           // CFG_STRING_SCRIPT_WND_PARAMS
    "Proxy tunnel",               // CFG_STRING_PROXY_TUNNEL_NAME
    "0x74 0x7E 0x7A 0x33",        // CFG_STRING_AVR109_BOOTSEQ
    "" /*"0x74 0x7E 0x7A 0x33"*/, // CFG_STRING_ZMODEM_BOOTSEQ
};

static const QString keys_bool[] =
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
    "shupito/show_flash_warn",    // CFG_BOOL_SHUPITO_SHOW_FLASH_WARN
    "main/check_for_updates",  // CFG_BOOL_CHECK_FOR_UPDATE
    "main/load_last_session",  // CFG_BOOL_LOAD_LAST_SESSION
    "main/session_connect",    // CFG_BOOL_SESSION_CONNECT
    "main/portable",           // CFG_BOOL_PORTABLE
    "analyzer/script_show_errors",// CFG_BOOL_SHOW_SCRIPT_ERROR
    "main/smooth_scaling",     // CFG_BOOL_SMOOTH_SCALING
    "proxy/enable_tunnel",        // CFG_BOOL_PROXY_TUNNEL
    "analyzer/script_input",      // CFG_BOOL_SCRIPT_SHOW_INPUT
    "main/one_instance",       // CFG_BOOL_ONE_INSTANCE
    "analyzer/placement_lines",   // CFG_BOOL_ANALYZER_PLACEMENT_LINES
    "analyzer/show_preview",      // CFG_BOOL_ANALYZER_SHOW_PREVIEW
    "shupito/enable_hw_button",   // CFG_BOOL_SHUPITO_ENABLE_HW_BUTTON
    "analyzer/show_bookmarks",    // CFG_BOOL_ANALYZER_SHOW_BOOKMARKS
    "main/connect_on_new_tab", // CFG_BOOL_CONN_ON_NEW_TAB
    "main/enable_sounds",      // CFG_BOOL_ENABLE_SOUNDS
    "analyzer/enable_search",     // CFG_BOOL_ANALYZER_SEARCH_WIDGET
    "shupito/spi_tunnel_lsb",     // CFG_BOOL_SPI_TUNNEL_LSB_FIRST
};

static const bool def_bool[] =
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
    true,                         // CFG_BOOL_SHUPITO_SHOW_FLASH_WARN
    true,                         // CFG_BOOL_CHECK_FOR_UPDATE
    false,                        // CFG_BOOL_LOAD_LAST_SESSION
    true,                         // CFG_BOOL_SESSION_CONNECT
    false,                        // CFG_BOOL_PORTABLE
    false,                        // CFG_BOOL_SHOW_SCRIPT_ERROR
    false,                        // CFG_BOOL_SMOOTH_SCALING
    false,                        // CFG_BOOL_PROXY_TUNNEL
    false,                        // CFG_BOOL_SCRIPT_SHOW_INPUT
    true,                         // CFG_BOOL_ONE_INSTANCE
    true,                         // CFG_BOOL_ANALYZER_PLACEMENT_LINES
    true,                         // CFG_BOOL_ANALYZER_SHOW_PREVIEW
    true,                         // CFG_BOOL_SHUPITO_ENABLE_HW_BUTTON
    true,                         // CFG_BOOL_ANALYZER_SHOW_BOOKMARKS
    true,                         // CFG_BOOL_CONN_ON_NEW_TAB
    true,                         // CFG_BOOL_ENABLE_SOUNDS
    true,                         // CFG_BOOL_ANALYZER_SEARCH_WIDGET
    false,                        // CFG_BOOL_SPI_TUNNEL_LSB_FIRST
};

static const QString keys_variant[] =
{
    "main/connections",        // CFG_VARIANT_CONNECTIONS
    "main/usb_yb_enumerator",  // CFG_VARIANT_USB_ENUMERATOR
    "kate/kate_sett_doc",         // CFG_VARIANT_KATE_SETTINGS_DOC
    "kate/kate_sett_view",        // CFG_VARIANT_KATE_SETTINGS_VIEW
    "main/serial_connections", // CFG_VARIANT_SERIAL_CONNECTIONS
};

static const QString keys_float[] =
{
    "shupito/overvoltage_val",    // CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL
};

static const float def_float[] =
{
    5.5f,                         // CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL
};

Config::Config()
{
    // Check defaults
    Q_ASSERT(sizeof_array(keys_quint32)   == CFG_QUINT32_NUM);
    Q_ASSERT(sizeof_array(def_quint32)    == CFG_QUINT32_NUM);
    Q_ASSERT(sizeof_array(keys_string)    == CFG_STRING_NUM);
    Q_ASSERT(sizeof_array(def_string)     == CFG_STRING_NUM);
    Q_ASSERT(sizeof_array(keys_bool)      == CFG_BOOL_NUM);
    Q_ASSERT(sizeof_array(def_bool)       == CFG_BOOL_NUM);
    Q_ASSERT(sizeof_array(keys_variant)   == CFG_VARIANT_NUM);
    Q_ASSERT(sizeof_array(def_float)      == CFG_FLOAT_NUM);
    Q_ASSERT(sizeof_array(keys_float)     == CFG_FLOAT_NUM);

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
        Utils::storageLocation(Utils::DocumentsLocation) + "/Lorris/config.ini",
        Utils::storageLocation(Utils::DataLocation) + "/config.ini"
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
