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

#include "config.h"
#include "connection/connectionmgr.h"
#include "qserialdevice/abstractserial.h"

static const QString keys_quint32[CFG_QUINT32_NUM] =
{
    "general/connection_type",   // CFG_QUINT32_CONNECTION_TYPE
    "general/tab_type",          // CFG_QUINT32_TAB_TYPE
    "serial_port/baud_rate",     // CFG_QUINT32_SERIAL_BAUD
    "analyzer/update_time",      // CFG_QUINT32_ANALYZER_UPDATE_TIME
    "shupito/flash_mode",        // CFG_QUINT32_SHUPITO_MODE
    "shupito/prog_speed",        // CFG_QUINT32_SHUPITO_PRG_SPEED
    "general/language",          // CFG_QUINT32_LANGUAGE
};

static const QString keys_string[CFG_STRING_NUM] =
{
    "serial_port/port",           // CFG_STRING_SERIAL_PORT
    "shupito/port",               // CFG_STRING_SHUPITO_PORT
    "terminal/hex_folder",        // CFG_STRING_HEX_FOLDER
    "analyzer/data_folder",       // CFG_STRING_ANALYZER_FOLDER
    "shupito/hex_folder",         // CFG_SHUPITO_HEX_FOLDER
};

static const QString keys_bool[CFG_BOOL_NUM] =
{
    "shupito/enable_tunnel",      // CFG_BOOL_SHUPITO_TUNNEL
    "shupito/show_log",           // CFG_BOOL_SHUPITO_SHOW_LOG
    "shupito/show_fuses",         // CFG_BOOL_SHUPITO_SHOW_FUSES
    "shupito/verify",             // CFG_BOOL_SHUPITO_VERIFY
};

Config::Config()
{
    m_settings = new QSettings("config.ini", QSettings::IniFormat);

    // Fill default values
    m_def_quint32[CFG_QUINT32_CONNECTION_TYPE]     = MAX_CON_TYPE;
    m_def_quint32[CFG_QUINT32_TAB_TYPE]            = 0;
    m_def_quint32[CFG_QUINT32_SERIAL_BAUD]         = AbstractSerial::BaudRate38400;
    m_def_quint32[CFG_QUINT32_ANALYZER_UPDATE_TIME]= 100;
    m_def_quint32[CFG_QUINT32_SHUPITO_MODE]        = 0;
    m_def_quint32[CFG_QUINT32_SHUPITO_PRG_SPEED]   = 0;
    m_def_quint32[CFG_QUINT32_LANGUAGE]            = 0;

    m_def_string[CFG_STRING_SERIAL_PORT]           = "";
    m_def_string[CFG_STRING_SHUPITO_PORT]          = "";
    m_def_string[CFG_STRING_HEX_FOLDER]            = "";
    m_def_string[CFG_STRING_ANALYZER_FOLDER]       = "";
    m_def_string[CFG_SHUPITO_HEX_FOLDER]           = "";

    m_def_bool[CFG_BOOL_SHUPITO_TUNNEL]            = true;
    m_def_bool[CFG_BOOL_SHUPITO_SHOW_LOG]          = true;
    m_def_bool[CFG_BOOL_SHUPITO_SHOW_FUSES]        = true;
    m_def_bool[CFG_BOOL_SHUPITO_VERIFY]            = true;
}

Config::~Config()
{
    delete m_settings;
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
