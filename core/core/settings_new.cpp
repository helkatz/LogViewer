#include <core/settings_new.h>

#include <QRegularExpression>
#include <QtXml/QtXml>
#include <common/logger.h>
QString concatPath(const QString& path1, const QString& path2)
{
	if (path1.length() && path2.length()) {
		return path1 + "/" + path2;
	}
	if (path1.length())
		return path1;
	return path2;
}

QString PropClass::Private::getPath() const {
	auto pp = parent ? parent->getPath() : "";
	return concatPath(pp, name);
}

QString PropClass::Private::fullPath(const QString& key) const {
	auto pp = getPath();
	return pp.length() ? pp + "/" + key : key;
}

QStringList PropClass::Private::keys(const QString& path, FetchKeysMode mode) const
{
	QString pattern = path.length() ? path + "\/" : "";
	pattern.replace("/", "\\/");
	pattern = "^(" + pattern + ")";
	if (mode == FetchKeysMode::NodeGroups)
		pattern += "([^\\/]+)\\/";
	else if (mode == FetchKeysMode::NodeKeys)
		pattern += "([^\\/]+)$";
	else if (mode == FetchKeysMode::AllGroups)
		pattern += "(.+)\/";
	else if (mode == FetchKeysMode::AllKeys)
		pattern += "(.+[^\\/]$)";
	else
		pattern += "(.+)";
	QRegularExpression re(pattern, QRegularExpression::DotMatchesEverythingOption
		| QRegularExpression::MultilineOption
	);
	QMap<QString, bool> cachedGroups;
	//auto& storage = Private::commonStorage;
	auto& storage = root->localStorage;
	for (auto it = storage.begin(); it != storage.end(); ++it) {
		 
		auto& cd = storage[it.key()];
		//log_debug() << pattern.toStdString() << it.key().toStdString() << cd.isRemoved;
		auto m = re.match(it.key());
		if (m.hasMatch() && cd.isRemoved == false) {
			auto capturedTexts = m.capturedTexts();
			cachedGroups[capturedTexts[2]] = true;
		}
	}
	return cachedGroups.keys();
}

void PropClass::Private::configure()
{

}

void PropClass::Private::configurePersistent(const QString& organisation
	, const QString& application)
{
	persistentStorage = std::make_shared<QSettings>(
		organisation, application
		);
	persistentStorage->beginGroup(getPath());
	auto& storage = root->localStorage;
	for (auto& k : persistentStorage->allKeys()) {
		auto value = persistentStorage->value(k);
		localStorage[expand(k)].value = value;
	}
	persistentStorage->endGroup();
}

void PropClass::Private::bind(PropClass& other, bool apply)
{
	if (apply) {
		set(other);
	}

	other.impl_->parent = shared_from_this();
	other.impl_->root = root;
	other.impl_->persistentStorage = persistentStorage;
}

void PropClass::Private::unbind()
{
	if (root == shared_from_this())
		return;
	
	auto keys = childKeys(FetchKeysMode::All);

	//name = "";
	localStorage.clear();
	for (auto& k : keys) {
		auto path = fullPath(k);
		auto& storage = root->localStorage;
		auto found = storage.find(path);
		if (found != storage.end()) {
			localStorage[k/*concatPath(name, k)*/] = *found;
		}
	}
	persistentStorage.reset();
	root = shared_from_this();
	//path = "";
	name = "";
	parent.reset();
}

void PropClass::Private::setPath(const QString& path)
{
	//basePath = path;
	name = path;
	//path = expand(path);
}

QStringList PropClass::Private::childGroups() const
{
	return keys(getPath(), FetchKeysMode::NodeGroups);
}

QStringList PropClass::Private::childKeys(FetchKeysMode mode) const
{
	return keys(getPath(), mode);
}

QString PropClass::Private::expand(const QString& name) const
{
	return fullPath(name);
	//if (path.length() && name.length())
	//	return path + "/" + name;
	//if (path.length())
	//	return path;
	//if (name.length())
	//	return name;
	//return "";
}

void PropClass::Private::set(const PropClass::PrivatePtr other)
{
	auto keys = other->childKeys(FetchKeysMode::All);
	if (root != other->root && other->name.length()) {
		remove(other->name);
	}
	
	for (auto k : keys) {
		auto targetPath = k;
		if (root != other->root)
			targetPath = concatPath(other->name, k);
		set(targetPath, other->get(k));
	}
}

void PropClass::Private::set(const PropClass& other)
{
	set(other.impl_);
}

void PropClass::Private::set(const QString& name, const QVariant& value)
{
	get(name);
	auto path = expand(name);
	auto& storage = root->localStorage;
	auto& data = storage[path];
	data.isDirty = true;
	data.value = value;
	data.isRemoved = false;
	//log_debug() << "set path" << path.toStdString()
	//	<< "persistentStorage" << (bool)persistentStorage;
	if (persistentStorage) {
		persistentStorage->setValue(expand(name), value);
	}
}

QVariant PropClass::Private::get(const QString& name, const QVariant& def) const
{
	auto path = expand(name);
	auto& storage = root->localStorage;
	auto found = storage.find(path);
	if (found != storage.end()) {
		return found->value;
	}
	return def;
}

void PropClass::Private::remove()
{
	remove("");
}

void PropClass::Private::remove(const QString& name)
{
	
	auto& storage = root->localStorage;
	auto savePersistent = persistentStorage;
	auto path = expand(name);

	//log_debug() << "remove path" << path.toStdString() << "name" 
	//	<< name.toStdString() << "savePersistent" << (bool)savePersistent;
	for (auto it = storage.begin(); it != storage.end();) {
		if (it.key().indexOf(path) == 0) {
			if (savePersistent) {
				it = storage.erase(it);
				continue;
			}
			it->isDirty = true;
			it->isRemoved = true;
		}
		++it;
	}
	if (savePersistent) {
		persistentStorage->remove(path);
	}
}

void PropClass::Private::rename(const QString& name)
{
	PropClass newC(name, parent);
	newC->set(shared_from_this());
	remove();
}

QString PropClass::Private::path() const
{
	return getPath();
}

void PropClass::Private::clearCache()
{
	root->localStorage.clear();
}

void PropClass::Private::saveCache()
{
	auto& storage = root->localStorage;
	for (auto it = storage.begin(); it != storage.end();) {
		auto& data = it.value();
		if (data.isDirty == false) {
			it++;
			continue;
		}
		if (data.isRemoved) {
			if (persistentStorage) {
				persistentStorage->remove(it.key());
			}
			it = storage.erase(it);
		}
		else {
			if (persistentStorage) {
				persistentStorage->setValue(it.key(), data.value);
			}
			data.isDirty = false;
			it++;
		}
	}
}

PropClass::~PropClass()
{
}

PropClass::PropClass() :
	PropClass::PropClass("")
{
}

PropClass::PropClass(const QString& name) :
	impl_(std::make_shared<Private>())
{
	impl_->name = name;
	impl_->root = impl_;
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
	impl_->name = name;
}

PropClass::PrivatePtr PropClass::operator -> ()
{
	return impl_;
}

const PropClass::PrivatePtr PropClass::operator -> () const
{
	return impl_;
}

void PropClass::configure()
{

}

void PropClass::setPath(const QString& path)
{
	impl_->name = path;
}

void PropClass::configurePersistent(const QString& organisation
	, const QString& application)
{
	impl_->configurePersistent(organisation, application);
}

#if 0
QStringList PropClass::keys(const QString& path, FetchKeysMode mode) const
{
	QString pattern = path.length() ? path + "\/" : "";
	pattern.replace("/", "\\/");
	pattern = "^(" + pattern + ")";
	if (mode == FetchKeysMode::NodeGroups)
		pattern += "([^\\/]+)\\/";
	else if (mode == FetchKeysMode::NodeKeys)
		pattern += "([^\\/]+)$";
	else if (mode == FetchKeysMode::AllGroups)
		pattern += "(.+)\/";
	else if (mode == FetchKeysMode::AllKeys)
		pattern += "(.+[^\\/]$)";
	else
		pattern += "(.+)";
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

void PropClass::configurePersistent(const QString& organisation
	, const QString& application)
{
	impl_->persistentStorage = std::make_shared<QSettings>(
		organisation, application
		);
	impl_->persistentStorage->beginGroup(impl_->getPath());
	auto& storage = impl_->root->localStorage;
	for (auto& k : impl_->persistentStorage->allKeys()) {
		auto value = impl_->persistentStorage->value(k);
		impl_->localStorage[expand(k)].value = value;
	}
	impl_->persistentStorage->endGroup();
}

void PropClass::bind(PropClass& other, bool apply)
{
	if (apply) {
		set(other);
	}

	other.impl_->parent = impl_;
	other.impl_->root = impl_->root;
	other.impl_->persistentStorage = impl_->persistentStorage;
}

void PropClass::unbind()
{
	if (impl_->root == impl_)
		return;

	auto keys = childKeys(FetchKeysMode::All);
	impl_->localStorage.clear();
	for (auto& k : keys) {
		auto path = impl_->fullPath(k);
		auto& storage = impl_->root->localStorage;
		auto found = storage.find(path);
		if (found != storage.end()) {
			impl_->localStorage[concatPath(impl_->name, k)] = *found;
		}
	}
	impl_->persistentStorage.reset();
	impl_->root = impl_;
	//impl_->path = "";
	impl_->parent.reset();
}

void PropClass::setPath(const QString& path)
{
	//impl_->basePath = path;
	impl_->name = path;
	//impl_->path = expand(path);
}

QStringList PropClass::childGroups() const
{
	return keys(impl_->getPath(), FetchKeysMode::NodeGroups);
}

QStringList PropClass::childKeys(FetchKeysMode mode) const
{
	return keys(impl_->getPath(), mode);
}

QString PropClass::expand(const QString& name) const
{
	return impl_->fullPath(name);
	//if (impl_->path.length() && name.length())
	//	return impl_->path + "/" + name;
	//if (impl_->path.length())
	//	return impl_->path;
	//if (name.length())
	//	return name;
	//return "";
}

void PropClass::set(const PropClass& other)
{
	auto keys = other.childKeys(FetchKeysMode::All);
	if (impl_->root != other.impl_->root && other.name().length()) {
		remove(other.name());
	}
	for (auto k : keys) {
		auto targetPath = k;
		if (impl_->root != other.impl_->root)
			targetPath = concatPath(other.name(), k);
		set(targetPath, other.get(k));
	}
}

void PropClass::set(const QString& name, const QVariant& value)
{
	get(name);
	auto path = expand(name);
	auto& storage = impl_->root->localStorage;
	auto& data = storage[path];
	data.isDirty = true;
	data.value = value;
	data.isRemoved = false;
	if (impl_->persistentStorage) {
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
	auto savePersistent = impl_->persistentStorage;
	auto path = expand(name);
	for (auto it = storage.begin(); it != storage.end();) {
		if (it.key().indexOf(path) == 0) {
			if (savePersistent) {
				it = storage.erase(it);
				continue;
			}
			it->isDirty = true;
			it->isRemoved = true;
		}
		++it;
	}
	if (savePersistent) {
		impl_->persistentStorage->remove(path);
	}
}

QString PropClass::path() const
{
	return impl_->getPath();
}

QString PropClass::name() const
{
	return impl_->name;
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
#endif