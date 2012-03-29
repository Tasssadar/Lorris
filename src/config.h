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

#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QHash>

#include "singleton.h"

enum cfg_quint32
{
    CFG_QUINT32_CONNECTION_TYPE = 0,
    CFG_QUINT32_TAB_TYPE,
    CFG_QUINT32_SERIAL_BAUD,
    CFG_QUINT32_ANALYZER_UPDATE_TIME,
    CFG_QUINT32_SHUPITO_MODE,
    CFG_QUINT32_SHUPITO_PRG_SPEED,
    CFG_QUINT32_LANGUAGE,
    CFG_QUINT32_TERMINAL_FMT,
    CFG_QUINT32_TCP_PORT,
    CFG_QUINT32_TERMINAL_INPUT,
    CFG_QUINT32_PROXY_PORT,
    CFG_QUINT32_SHUPITO_VERIFY,
    CFG_QUINT32_ANALYZER_PLAY_DEL,
    CFG_QUITN32_SHUPITO_TERM_FMT,

    CFG_QUINT32_NUM
};

enum cfg_string
{
    CFG_STRING_SERIAL_PORT = 0,
    CFG_STRING_SHUPITO_PORT,
    CFG_STRING_HEX_FOLDER,
    CFG_STRING_ANALYZER_FOLDER,
    CFG_STRING_SHUPITO_HEX_FOLDER,
    CFG_STRING_SHUPITO_TUNNEL,
    CFG_STRING_TCP_ADDR,
    CFG_STRING_PROXY_ADDR,
    CFG_STRING_ANALYZER_JS,

    CFG_STRING_NUM
};

enum cfg_bool
{
    CFG_BOOL_SHUPITO_TUNNEL = 0,
    CFG_BOOL_SHUPITO_SHOW_LOG,
    CFG_BOOL_SHUPITO_SHOW_FUSES,

    CFG_BOOL_NUM
};

class Config : public Singleton<Config>
{
    typedef QHash<cfg_quint32, quint32> def_map_quint32;
    typedef QHash<cfg_string, QString> def_map_string;
    typedef QHash<cfg_bool, bool> def_map_bool;

public:
    Config();
    ~Config();

    quint32 get(cfg_quint32 item);
    QString get(cfg_string item);
    bool    get(cfg_bool item);

    void set(cfg_quint32 item, quint32        val);
    void set(cfg_string  item, const QString& val);
    void set(cfg_bool    item, bool           val);

private:
    void openSettings();

    QSettings *m_settings;
    def_map_quint32 m_def_quint32;
    def_map_string m_def_string;
    def_map_bool m_def_bool;
};

#define sConfig Config::GetSingleton()

#endif // CONFIG_H
