#include <core/settings.h>

#include <QRegularExpression>
#include <QtXml/QtXml>
#if 0
struct PSettings::Private {
	QString path;
	QString name;
	PSettings* root;
	static QString organisation;
	static QString application;
	static StorageMap commonStorage;
	StorageMap localStorage;
	bool saveOnlyToCache;
};
PSettings::StorageMap PSettings::Private::commonStorage;
QString PSettings::Private::organisation;
QString PSettings::Private::application;


PSettings::~PSettings()
{
}

PSettings::PSettings() :
	PSettings("")
{
}

PSettings::PSettings(const QString& basePath) :
	QSettings(Private::organisation, Private::application),
	impl_(std::make_shared<Private>())
{
	impl_->path = basePath;
	impl_->root = this;
	impl_->saveOnlyToCache = false;
	if (Private::commonStorage.isEmpty()) {
		loadIntoCache("");
	}
	configure();
}

PSettings::PSettings(const PSettings& other)
{
	impl_ = other.impl_;
}

PSettings& PSettings::operator = (const PSettings& other)
{
	impl_ = other.impl_;
	return *this;
}

PSettings::PSettings(const QString& name, PrivatePtr parent):
	impl_(std::make_shared<Private>())
{
	if(parent.get())
		*impl_ = *parent;
	impl_->path.replace(QRegularExpression("^[\\/]*(.*?)[\\/]*$"), "\\1");
	if (name.length()) {
		if (impl_->path.length())
			impl_->path += "/";
		impl_->path += name;
	}
}

QStringList PSettings::childGroupsFromCache(const QString& path)
{
	QString pattern = path;
	pattern.replace("/", "\\/");
	pattern = "^(" + pattern + ")[\\/]+([^\\/]+|$)";
	QRegularExpression re(pattern, QRegularExpression::DotMatchesEverythingOption
		| QRegularExpression::MultilineOption
	);
	QMap<QString, bool> cachedGroups;
	for (auto it = Private::commonStorage.begin(); it != Private::commonStorage.end(); ++it) {
		if (it.key().indexOf(path) != 0)
			continue;
		auto& cd = Private::commonStorage[it.key()];
		auto m = re.match(it.key());
		if (m.hasMatch() && cd.isRemoved == false) {
			auto capturedTexts = m.capturedTexts();
			cachedGroups[capturedTexts[2]] = true;
		}
	}
	return cachedGroups.keys();
}

QString PSettings::path() const
{
	return impl_->path;
}

void PSettings::loadIntoCache(const QString& path)
{
	QString basePath = path;
	if (basePath.length()) {
		basePath += "/";
	}

	beginGroup(basePath);
	for (auto& k : allKeys()) {
		auto value = QSettings::value(k);
		Private::commonStorage[basePath + k].value = value;
	}
	endGroup();
}

void PSettings::configure()
{

}

void PSettings::setPath(const QString& path)
{
	impl_->path = path;
}

QStringList PSettings::childGroups(const QString path)
{
	return childGroupsFromCache(expand(path));
}

QString PSettings::expand(const QString& name) const
{
	if (impl_->path.length() && name.length())
		return impl_->path + "/" + name;
	if (impl_->path.length())
		return impl_->path;
	if (name.length())
		return name;
	return "";
}

void PSettings::set(const QString& name, const QVariant& value)
{
	auto path = expand(name);
	auto& data = impl_->localStorage[name];
	data.isDirty = data.value != value;
	data.value = value;
	if (impl_->root) {
		auto& data = Private::commonStorage[path];
		data.value = value;
		data.isDirty = impl_->saveOnlyToCache;
		data.isRemoved = false;
		if (!impl_->saveOnlyToCache)
			impl_->root->setValue(expand(name), value);
	}
}

QVariant PSettings::get(const QString& name, const QVariant& def) const
{
	auto found = impl_->localStorage.find(name);
	if (found != impl_->localStorage.end()) {

	}
	found = Private::commonStorage.find(expand(name));
	if (found != Private::commonStorage.end() && !found->isRemoved)
		return found.value().value;
	return def;
}

void PSettings::remove()
{
	remove("");
}

void PSettings::remove(const QString& name)
{
	auto path = expand(name);
	for (auto it = Private::commonStorage.begin(); it != Private::commonStorage.end(); ++it) {
		if (it.key().indexOf(path) == 0) {
			it->isDirty = impl_->saveOnlyToCache;
			it->isRemoved = true;
		}
	}
	if (!impl_->saveOnlyToCache) {
		QSettings::remove(path);
		Private::commonStorage.remove(path);
	}
	//_root->remove(name.length() == 0 ? path() : path() + "/" + name);
}

QString PSettings::name() const
{
	return impl_->name;
}

void PSettings::setOrganisation(const QString organisation)
{
	Private::organisation = organisation;
}

void PSettings::setApplication(const QString application)
{
	Private::application = application;
}

void PSettings::clearCache()
{
	impl_->commonStorage.clear();
}

void PSettings::saveCache()
{
	for (auto it = impl_->commonStorage.begin(); it != impl_->commonStorage.end();) {
		auto& data = it.value();
		if (data.isDirty == false) {
			it++;
			continue;
		}
		if (data.isRemoved) {
			QSettings::remove(it.key());
			it = impl_->commonStorage.erase(it);
		}
		else {
			setValue(it.key(), data.value);
			data.isDirty = false;
			it++;
		}
	}
}
#endif