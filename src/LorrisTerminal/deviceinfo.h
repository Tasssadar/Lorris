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

        if(idString == "m16")       { name = "ATmega16";  mem_size = 16128; page_size = 128; }
        else if(idString == "m32")  { name = "ATmega32";  mem_size = 32256; page_size = 128; }
        else if(idString == "m48")  { name = "ATmega48";  mem_size = 3840;  page_size = 64; patch_pos = 3838; }
        else if(idString == "m88")  { name = "ATmega88";  mem_size = 7936;  page_size = 64;  }
        else if(idString == "m162") { name = "ATmega162"; mem_size = 16128; page_size = 128; }
        else if(idString == "m168") { name = "ATmega168"; mem_size = 16128; page_size = 128; }
        else if(idString == "m328") { name = "ATmega328"; mem_size = 32256; page_size = 128; }
        /* FIXME: only 16-bit addresses are available */
        else if(idString == "m128") { name = "ATmega128"; mem_size = 65536; page_size = 256; }

        if(mem_size)
            id = idString;
    }

    bool isSet() { return (mem_size != 0); }

    QString name;
    QString id;
    uint32_t mem_size;
    uint16_t page_size;
    uint16_t patch_pos;
};

#endif // DEVICEINFO_H
