#pragma once

#include <core\settings.h>

class DatabaseSettings : public Settings
{
public:
    DatabaseSettings():
        Settings("plugins/database", true)
    {
    }
    PROPLIST_BEGIN(connections)
        PROP(QString, driver)
        PROP(QString, database)
        PROP(QString, host)
        PROP(QString, username)
        PROP(QString, password)
    PROPLIST_END(connections)
};