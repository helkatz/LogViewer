#include <core/settings_new.h>

#include <QRegularExpression>
#include <QtXml/QtXml>

struct PropClass::Private {
	QString path;
	QString name;
	PrivatePtr root;
	static QString organisation;
	static QString application;
	static StorageMap commonStorage;
	StorageMap localStorage;
	bool saveOnlyToCache;
	std::shared_ptr<QSettings> persistentStorage;
	PrivatePtr parent;
	QList<PrivatePtr> children;
};
PropClass::StorageMap PropClass::Private::commonStorage;
QString PropClass::Private::organisation;
QString PropClass::Private::application;


PropClass::~PropClass()
{
	if(impl_->parent)
		impl_->parent->children.removeOne(impl_);
}

PropClass::PropClass() :
	PropClass::PropClass("")
{
}

PropClass::PropClass(const QString& basePath) :
	impl_(std::make_shared<Private>())
{
	impl_->path = basePath;
	impl_->root = impl_;
	impl_->saveOnlyToCache = false;
}

PropClass::PropClass(const PropClass& other)
{
	impl_ = other.impl_;
}

PropClass& PropClass::operator = (const PropClass& other)
{
	impl_ = other.impl_;
	return *this;
}

PropClass::PropClass(const QString& name, PrivatePtr parent):
	impl_(std::make_shared<Private>())
{
	*impl_ = *parent;
	impl_->localStorage.clear();
	impl_->parent = parent;
	parent->children.push_back(impl_);
	impl_->path.replace(QRegularExpression("^[\\/]*(.*?)[\\/]*$"), "\\1");
	if (name.length()) {
		if (impl_->path.length())
			impl_->path += "/";
		impl_->path += name;
	}
}

QStringList PropClass::keys(const QString& path, FetchKeysMode mode) const
{
	QString pattern = path.length() ? path + "\/" : "";
	pattern.replace("/", "\\/");
	pattern = "^(" + pattern + ")([^\\/]+)";
	if (mode == FetchKeysMode::Groups)
		pattern += "\\/";
	if (mode == FetchKeysMode::Keys)
		pattern += "$";

	QRegularExpression re(pattern, QRegularExpression::DotMatchesEverythingOption
		| QRegularExpression::MultilineOption
	);
	QMap<QString, bool> cachedGroups;
	//auto& storage = Private::commonStorage;
	auto& storage = impl_->root->localStorage;
	for (auto it = storage.begin(); it != storage.end(); ++it) {
		auto& cd = storage[it.key()];
		auto m = re.match(it.key());
		if (m.hasMatch() && cd.isRemoved == false) {
			auto capturedTexts = m.capturedTexts();
			cachedGroups[capturedTexts[2]] = true;
		}
	}
	return cachedGroups.keys();
}

QString PropClass::path() const
{
	return impl_->path;
}

void PropClass::configure()
{

}

void PropClass::configurePersistent(const QString& organisation
	, const QString& application)
{
	impl_->persistentStorage = std::make_shared<QSettings>(
		organisation, application
		);
	impl_->persistentStorage->beginGroup(impl_->path);
	auto& storage = impl_->root->localStorage;
	for (auto& k : impl_->persistentStorage->allKeys()) {
		auto value = impl_->persistentStorage->value(k);
		impl_->localStorage[expand(k)].value = value;
	}
	impl_->persistentStorage->endGroup();
}

void PropClass::bindTo(PropClass& other)
{
	impl_->parent = other.impl_;
	impl_->root = other.impl_->root;
	impl_->path = expand(other.impl_->path);
	impl_->persistentStorage = other.impl_->persistentStorage;
}

void PropClass::unbind()
{
	if (impl_->root == impl_)
		return;
	auto keys = childKeys(FetchKeysMode::All);
	impl_->localStorage.clear();
	for (auto& k : keys) {
		auto path = expand(k);
		auto& storage = impl_->root->localStorage;
		auto found = storage.find(path);
		if (found != storage.end()) {
			impl_->localStorage[k] = *found;
		}
	}
	impl_->persistentStorage.reset();
	impl_->root = impl_;
	impl_->parent.reset();
	impl_->path = "";
}

void PropClass::setPath(const QString& path)
{
	impl_->path = expand(path);
}

QStringList PropClass::childGroups() const
{
	return keys(impl_->path, FetchKeysMode::Groups);
}

QStringList PropClass::childKeys(FetchKeysMode mode) const
{
	return keys(impl_->path, mode);
}

QString PropClass::expand(const QString& name) const
{
	if (impl_->path.length() && name.length())
		return impl_->path + "/" + name;
	if (impl_->path.length())
		return impl_->path;
	if (name.length())
		return name;
	return "";
}

void PropClass::set(const PropClass& other)
{
	auto keys = other.childKeys(FetchKeysMode::All);
	for (auto& k : keys) {
		set(k, other.get(k));
	}
}

void PropClass::set(const QString& name, const QVariant& value)
{
	get(name);
	auto path = expand(name);
	auto& storage = impl_->root->localStorage;
	auto& data = storage[path];
	data.isDirty = data.value != value;
	data.value = value;
	data.isRemoved = false;
	if (!impl_->saveOnlyToCache && impl_->persistentStorage) {
		impl_->persistentStorage->setValue(expand(name), value);
	}
}

QVariant PropClass::get(const QString& name, const QVariant& def) const
{
	auto path = expand(name);
	auto& storage = impl_->root->localStorage;
	auto found = storage.find(path);
	if (found != storage.end()) {
		return found->value;
	}
	return def;
}

void PropClass::remove()
{
	remove("");
}

void PropClass::remove(const QString& name)
{
	auto& storage = impl_->root->localStorage;
	auto savePersistent = impl_->saveOnlyToCache && impl_->persistentStorage;
	auto path = expand(name);
	for (auto it = storage.begin(); it != storage.end(); ++it) {
		if (it.key().indexOf(path) == 0) {
			if (savePersistent) {
				it = storage.erase(it);
				continue;
			}
			it->isDirty = impl_->saveOnlyToCache;
			it->isRemoved = true;
		}
	}
	if (savePersistent) {
		impl_->persistentStorage->remove(path);
	}
}

QString PropClass::name() const
{
	return impl_->name;
}

void Settings::setOrganisation(const QString& organisation)
{
	Private::organisation = organisation;
}

void Settings::setApplication(const QString& application)
{
	Private::application = application;
}

void PropClass::clearCache()
{
	impl_->root->localStorage.clear();
}

void PropClass::saveCache()
{
	auto& storage = impl_->root->localStorage;
	for (auto it = storage.begin(); it != storage.end();) {
		auto& data = it.value();
		if (data.isDirty == false) {
			it++;
			continue;
		}
		if (data.isRemoved) {
			if (impl_->persistentStorage) {
				impl_->persistentStorage->remove(it.key());
			}
			it = storage.erase(it);
		}
		else {
			if (impl_->persistentStorage) {
				impl_->persistentStorage->setValue(it.key(), data.value);
			}
			data.isDirty = false;
			it++;
		}
	}
}

Settings::Settings()
{
}

void Settings::loadIntoCache(const QString& path)
{
	QString basePath = path;
	if (basePath.length()) {
		basePath += "/";
	}

	impl_->persistentStorage->beginGroup(basePath);
	for (auto& k : impl_->persistentStorage->allKeys()) {
		auto value = impl_->persistentStorage->value(k);
		Private::commonStorage[basePath + k].value = value;
		//impl_->localStorage[basePath + k].value = value;
	}
	impl_->persistentStorage->endGroup();
}

void Settings::loadIntoLocalStorage()
{
	impl_->persistentStorage->beginGroup(impl_->path);
	for (auto& k : impl_->persistentStorage->childKeys()) {
		auto value = impl_->persistentStorage->value(k);
		impl_->localStorage[expand(k)].value = value;
	}
	for (auto& k : impl_->persistentStorage->childGroups()) {
		auto value = impl_->persistentStorage->value(k);
		impl_->localStorage[expand(k)].value = value;
	}
	impl_->persistentStorage->endGroup();
}

void Settings::configure()
{
}