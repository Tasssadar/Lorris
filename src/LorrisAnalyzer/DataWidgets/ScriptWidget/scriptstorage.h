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

#ifndef SCRIPTSTORAGE_H
#define SCRIPTSTORAGE_H

#include <QObject>
#include <QHash>
#include <QScriptValue>
#include <QVariantList>

class DataFileParser;

struct ScriptData
{
    ScriptData()
    {
        len = 0;
        data = NULL;
    }

    quint32 len;
    char *data;
};

class ScriptStorage : public QObject
{
    Q_OBJECT
public:
    typedef QHash<QString, ScriptData*> DataHash;

    explicit ScriptStorage(QObject *parent = 0);
    ~ScriptStorage();

    void saveToFile(DataFileParser *file);
    void loadFromFile(DataFileParser *file);

public slots:
    void clear();

    void setBool                 (const QString& key, bool val);
    void setUInt32               (const QString& key, quint32 val);
    void setInt32                (const QString& key, qint32 val);
    void setFloat                (const QString& key, float val);
    void setString               (const QString& key, const QString& val);
    void setFloatArray           (const QString& key, const QVariantList& val);
    void setInt32Array           (const QString& key, const QVariantList& val);
    void setUInt32Array          (const QString& key, const QVariantList& val);

    bool         getBool         (const QString& key);
    quint32      getUInt32       (const QString& key);
    qint32       getInt32        (const QString& key);
    float        getFloat        (const QString& key);
    QString      getString       (const QString& key);
    QVariantList getFloatArray   (const QString& key);
    QVariantList getInt32Array   (const QString& key);
    QVariantList getUInt32Array  (const QString& key);

private:
    ScriptData *findKey(const QString& key);
    ScriptData *getKeyForSet(const QString& key);

    template <typename T> void setBaseType(const QString &key, T val);
    template <typename T> T getBaseType(const QString &key);

    template <typename T> void setArrayType(const QString& key, QVariantList val);
    template <typename T> QVariantList getArrayType(const QString& key);

    DataHash m_data;
};

template <typename T>
void ScriptStorage::setBaseType(const QString &key, T val)
{
    ScriptData *sc_data = getKeyForSet(key);

    sc_data->len = sizeof(T);
    sc_data->data = new char[sizeof(T)];

    char *p = (char*)(&val);
    std::copy(p, p+sizeof(T), sc_data->data);
}

template <typename T>
T ScriptStorage::getBaseType(const QString &key)
{
    ScriptData *sc_data = findKey(key);

    T val;

    if(!sc_data)
        return val;

    if(sizeof(T) != sc_data->len)
        return val;

    char *p = (char*)(&val);
    std::copy(sc_data->data, sc_data->data + sc_data->len, p);

    return val;
}

template <typename T>
void ScriptStorage::setArrayType(const QString& key, QVariantList val)
{
    ScriptData *sc_data = getKeyForSet(key);

    sc_data->len = val.size()*sizeof(T);
    sc_data->data = new char[sc_data->len];

    quint32 pos = 0;
    for(QVariantList::Iterator itr = val.begin(); itr != val.end(); ++itr)
    {
        T val = (*itr).value<T>();
        char *p = (char*)(&val);

        std::copy(p, p+sizeof(T), sc_data->data+pos);
        pos += sizeof(T);
    }
}

template <typename T>
QVariantList ScriptStorage::getArrayType(const QString& key)
{
    ScriptData *sc_data = findKey(key);
    if(!sc_data)
        return QVariantList();

    if(sc_data->len % sizeof(T) != 0 || sc_data->len < sizeof(T))
        return QVariantList();

    QVariantList ret;
    for(quint32 i = 0; i < sc_data->len; i += sizeof(T))
    {
        QVariant item(*( (T*)(sc_data->data + i) ));
        ret.push_back(item);
    }
    return ret;
}

#endif // SCRIPTSTORAGE_H
