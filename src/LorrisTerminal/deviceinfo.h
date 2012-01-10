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

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>

class DeviceInfo
{
public:
    DeviceInfo(QString idString)
    {
        mem_size = 0;
        patch_pos = 0;

        if     (idString == "m16")  { name = "ATmega16";    mem_size = 16128; page_size = 128; eeprom_size = 512;  }
        else if(idString == "m32")  { name = "ATmega32";    mem_size = 32256; page_size = 128; eeprom_size = 1024; }
        else if(idString == "m48")  { name = "ATmega48";    mem_size = 3840;  page_size = 64;  patch_pos = 3838; eeprom_size = 256; }
        else if(idString == "m88")  { name = "ATmega88";    mem_size = 7936;  page_size = 64;  eeprom_size = 512;  }
        else if(idString == "m162") { name = "ATmega162";   mem_size = 16128; page_size = 128; eeprom_size = 512;  }
        else if(idString == "m168") { name = "ATmega168";   mem_size = 16128; page_size = 128; eeprom_size = 512;  }
        else if(idString == "m328") { name = "ATmega328P";  mem_size = 32256; page_size = 128; eeprom_size = 1024; }
        else if(idString == "m8u2") { name = "ATmega8U2";   mem_size = 7680;  page_size = 64;  eeprom_size = 512;  }
        /* FIXME: only 16-bit addresses are available */
        else if(idString == "m128") { name = "ATmega128";   mem_size = 65536; page_size = 256; eeprom_size = 4096; }
        /* FIXME: only 16-bit addresses are available and too long chip name */
        else if(idString == "p128") { name = "ATmega1284P"; mem_size = 65536; page_size = 256; eeprom_size = 4096; }

        if(mem_size)
            id = idString;
    }

    bool isSet() { return (mem_size != 0); }

    QString name;
    QString id;
    quint32 mem_size;
    quint16 page_size;
    quint16 patch_pos;
    quint16 eeprom_size;
};

#endif // DEVICEINFO_H
