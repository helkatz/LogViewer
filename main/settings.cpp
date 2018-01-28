#include "settings.h"
#include <QtXml/QtXml>
bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map);
bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map);
QString Settings::_organisation;
QString Settings::_application;
static const QSettings::Format xmlFormat = QSettings::registerFormat("xml", &readXmlFile, &writeXmlFile);

PropBaseClass::PropBaseClass(PropBaseClass *parent)
{ 
	_parent = parent; 
}

void PropBaseClass::setBasePath(const QString& path) 
{
	if (path.length())
		_basePath = "/" + path;
}

void PropBaseClass::setSubPath(const QString& group)
{
	_group = "/" + group;
}

QString PropBaseClass::getPath() const
{
	return path();
}

PropBaseClass *PropBaseClass::root() 
{
	PropBaseClass *p = this;
	while (p && p->_parent)
		p = p->_parent;
	return p;
}

const PropBaseClass *PropBaseClass::root() const
{
	return const_cast<PropBaseClass *>(this)->root();
}

QString PropBaseClass::path() const 
{
	QString p = _parent ? _parent->path() + "/" : _basePath;
	p += name() + _group;
	return std::move(p);
}

void PropBaseClass::set(const QString& name, const QVariant& value) 
{
	root()->set(path() + "/" + name, value);
}

QVariant PropBaseClass::get(const QString& name, const QVariant& def) const
{
	return root()->get(path() + "/" + name, def);
}

void PropBaseClass::remove(const QString& name)
{
	root()->remove(name.length() == 0 ? path() : path() + "/" + name);
}

QString PropBaseClass::name() const 
{ 
	return ""; 
}

bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map)
{
	QXmlStreamReader xmlReader(&device);
	QStringList elements;

	// Solange Ende nicht erreicht und kein Fehler aufgetreten ist
	while (!xmlReader.atEnd() && !xmlReader.hasError()) {
		// Nächsten Token lesen
		xmlReader.readNext();

		// Wenn Token ein Startelement
		if (xmlReader.isStartElement() && xmlReader.name() != "Settings") {
			// Element zur Liste hinzufügen
			elements.append(xmlReader.name().toString());
			// Wenn Token ein Endelement
		}
		else if (xmlReader.isEndElement()) {
			// Letztes Element löschen
			if (!elements.isEmpty()) elements.removeLast();
			// Wenn Token einen Wert enthält
		}
		else if (xmlReader.isCharacters() && !xmlReader.isWhitespace()) {
			QString key;

			// Elemente zu String hinzufügen
			for (int i = 0; i < elements.size(); i++) {
				if (i != 0) key += "/";
				key += elements.at(i);
			}

			// Wert in Map eintragen
			map[key] = xmlReader.text().toString();
		}
	}

	// Bei Fehler Warnung ausgeben
	if (xmlReader.hasError()) {
		qWarning() << xmlReader.errorString();
		return false;
	}

	return true;
}
bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map)
{
	QXmlStreamWriter xmlWriter(&device);

	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("Settings");

	QStringList prev_elements;
	QSettings::SettingsMap::ConstIterator map_i;

	// Alle Elemente der Map durchlaufen
	for (map_i = map.begin(); map_i != map.end(); map_i++) {

		QStringList elements = map_i.key().split("/");

		int x = 0;
		// Zu schließende Elemente ermitteln
		while (x < prev_elements.size() && elements.at(x) == prev_elements.at(x)) {
			x++;
		}

		// Elemente schließen
		for (int i = prev_elements.size() - 1; i >= x; i--) {
			xmlWriter.writeEndElement();
		}

		// Elemente öffnen
		for (int i = x; i < elements.size(); i++) {
			xmlWriter.writeStartElement(elements.at(i));
		}

		// Wert eintragen
		xmlWriter.writeCharacters(map_i.value().toString());

		prev_elements = elements;
	}

	// Noch offene Elemente schließen
	for (int i = 0; i < prev_elements.size(); i++) {
		xmlWriter.writeEndElement();
	}

	xmlWriter.writeEndElement();
	xmlWriter.writeEndDocument();

	return true;
}

void Settings::setOrganisation(const QString organisation)
{
	_organisation = organisation;
}

void Settings::setApplication(const QString application)
{
	_application = application;
}

Settings::Settings(QString basePath):
	//QSettings("Logviewer.xml", xmlFormat)
	QSettings(_organisation, _application)
{
	setBasePath(basePath);
	//qDebug() << fileName();
	ColorList cl;
	QVariant v;
	v.setValue(cl);
	ColorList clr = qvariant_cast<ColorList>(v);
	//QSettings::Format XmlFormat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);
	//QSettings settings(XmlFormat, QSettings::UserScope, "Organisation", "Name");
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

void Settings::set(const QString& name, const QVariant& value)
{
	/*qDebug("set %s value=%s",
	name.toStdString().c_str(),
	value.toString().toStdString().c_str());*/
	setValue(name, value);
}

QVariant Settings::get(const QString& name, const QVariant& def) const
{
	QVariant value = QSettings::value(name, def);
	/*qDebug("read %s value=%s",
	name.toStdString().c_str(),
	value.toString().toStdString().c_str());*/
	return value;
}

void Settings::remove(const QString& path)
{
	qDebug() << "remove" + path;
	QSettings::remove(path);
}
