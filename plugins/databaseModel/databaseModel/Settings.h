#pragma once

#include <core\settings.h>

SETTINGSCLASS(DatabaseSettings, PropClass,
    _CONFIGURE(
        setPath("plugins/database");
    )

    SETTINGSCLASS(Connection, PropClass,
        PROP(QString, driver)
        PROP(QString, database)
        PROP(QString, host)
        PROP(QString, username)
        PROP(QString, password)
    )
    _PROPLIST(Connection, connections)
)