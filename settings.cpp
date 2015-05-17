#include "settings.h"

QString Settings::_organisation;
QString Settings::_application;

void Settings::setOrganisation(const QString organisation)
{
    _organisation = organisation;
}

void Settings::setApplication(const QString application)
{
    _application = application;
}

Settings::Settings(QString basePath):
    QSettings(_organisation, _application)
{
    setBasePath(basePath);
}


QStringList Settings::childGroups(const QString& group)
{
    beginGroup(_basePath + "/" + group);
    QStringList r = QSettings::childGroups();
    endGroup();
    return r;
}

QStringList Settings::childKeys(const QString &group)
{
    beginGroup(_basePath + "/" + group);
    QStringList r = QSettings::childKeys();
    endGroup();
    return r;
}
