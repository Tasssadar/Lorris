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

#include <QSettings>
#include <QFile>
#include <QDesktopServices>
#include <qextserialport.h>

#include "config.h"
#include "connection/connectionmgr.h"

static const QString keys_quint32[CFG_QUINT32_NUM] =
{
    "general/connection_type",   // CFG_QUINT32_CONNECTION_TYPE
    "general/tab_type",          // CFG_QUINT32_TAB_TYPE
    "serial_port/baud_rate",     // CFG_QUINT32_SERIAL_BAUD
    "analyzer/update_time",      // CFG_QUINT32_ANALYZER_UPDATE_TIME
    "shupito/flash_mode",        // CFG_QUINT32_SHUPITO_MODE
    "shupito/prog_speed",        // CFG_QUINT32_SHUPITO_PRG_SPEED
    "general/language",          // CFG_QUINT32_LANGUAGE
    "terminal/format",           // CFG_QUINT32_TERMINAL_FMT
    "tcpsocket/port",            // CFG_QUINT32_TCP_PORT
    "terminal/input_handling",   // CFG_QUINT32_TERMINAL_INPUT
    "proxy/port",                // CFG_QUINT32_PROXY_PORT
    "shupito/verify_mode",       // CFG_QUINT32_SHUPITO_VERIFY
    "analyzer/play_delay",       // CFG_QUINT32_ANALYZER_PLAY_DEL
    "shupito/reload_hex",        // CFG_QUINT32_SHUPITO_HEX_RELOAD
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
};

static const QString keys_bool[CFG_BOOL_NUM] =
{
    "shupito/enable_tunnel",      // CFG_BOOL_SHUPITO_TUNNEL
    "shupito/show_log",           // CFG_BOOL_SHUPITO_SHOW_LOG
    "shupito/show_fuses",         // CFG_BOOL_SHUPITO_SHOW_FUSES
};

Config::Config()
{
    openSettings();

    // Fill default values
    m_def_quint32[CFG_QUINT32_CONNECTION_TYPE]     = MAX_CON_TYPE;
    m_def_quint32[CFG_QUINT32_TAB_TYPE]            = 0;
    m_def_quint32[CFG_QUINT32_SERIAL_BAUD]         = BAUD38400;
    m_def_quint32[CFG_QUINT32_ANALYZER_UPDATE_TIME]= 100;
    m_def_quint32[CFG_QUINT32_SHUPITO_MODE]        = 0;
    m_def_quint32[CFG_QUINT32_SHUPITO_PRG_SPEED]   = 0;
    m_def_quint32[CFG_QUINT32_LANGUAGE]            = 0;
    m_def_quint32[CFG_QUINT32_TERMINAL_FMT]        = 0;
    m_def_quint32[CFG_QUINT32_TCP_PORT]            = 0;
    m_def_quint32[CFG_QUINT32_TERMINAL_INPUT]      = 0;
    m_def_quint32[CFG_QUINT32_PROXY_PORT]          = 0;
    m_def_quint32[CFG_QUINT32_SHUPITO_VERIFY]      = 1;
    m_def_quint32[CFG_QUINT32_ANALYZER_PLAY_DEL]   = 1000;
    m_def_quint32[CFG_QUINT32_SHUPITO_HEX_RELOAD]  = 0;

    m_def_string[CFG_STRING_SERIAL_PORT]           = "";
    m_def_string[CFG_STRING_SHUPITO_PORT]          = "";
    m_def_string[CFG_STRING_HEX_FOLDER]            = "";
    m_def_string[CFG_STRING_ANALYZER_FOLDER]       = "";
    m_def_string[CFG_STRING_SHUPITO_HEX_FOLDER]    = "";
    m_def_string[CFG_STRING_SHUPITO_TUNNEL]        = "app";
    m_def_string[CFG_STRING_TCP_ADDR]              = "127.0.0.1";
    m_def_string[CFG_STRING_PROXY_ADDR]            = "0";
    m_def_string[CFG_STRING_ANALYZER_JS]           = "";

    m_def_bool[CFG_BOOL_SHUPITO_TUNNEL]            = true;
    m_def_bool[CFG_BOOL_SHUPITO_SHOW_LOG]          = false;
    m_def_bool[CFG_BOOL_SHUPITO_SHOW_FUSES]        = true;
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
