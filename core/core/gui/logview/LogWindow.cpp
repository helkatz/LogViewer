#include <interfaces/Factory.h>
#include <gui/logview/LogView.h>

LogWindow::LogWindow()
{
	setOrientation(Qt::Vertical);

	mainView_ = QSharedPointer<LogView>{ new LogView(this) };	
	mainView_->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	mainView_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	detailView_ = QSharedPointer<DetailView>{ new DetailView(this) };
}

LogWindow::~LogWindow()
{
	if (model_ == nullptr)
		return;
	model_->removeView(mainView_.data());
}

LogWindow *LogWindow::create(LogWindow::CreateParams& params)
{	
	_settings::QueryParams qp;
	if (params.settings) {
		qp = params.settings->queryParams();
	}
	else if (params.queryParams) {
		qp = *params.queryParams;
	}
	else {
		return nullptr;
	}
	qp.unbind();
	LogWindow * window = nullptr;
	try {
		QSharedPointer<LogModel> model(
			plugin_factory::Factory::Create<LogModel *>("models", qp.modelClass(), nullptr));

		if (model.isNull())
			throw std::exception("invalid modelClass");

		model->setQueryParams(qp);

		if (model->query() == false) {
			return nullptr;
		}

		window = new LogWindow();
		window->setModel(model);

		bool templateWindowFound = false;
		
		if (params.determineTemplateWindow) {
			// when true search for a stored template window
			for(auto it: appSettings().logWindowTemplatesList()) {
				auto& templ = it.second;
				QString filter = templ.viewNameFilter();
				QRegExp regex(filter);
				if (model->getTitle().contains(regex)) {
					window->readSettings(templ.templateWindow());
					templateWindowFound = true;
				}
			}
		}
		else if (params.templateWindow) {
			// here use another window as template
			window->setSettings(params.templateWindow);
		}
		if (!templateWindowFound && params.settings) {
			window->readSettings(*params.settings);
		}

		//if (window->query() == false) {
		//	delete window;
		//	return nullptr;
		//}

		window->refreshTitle();
		window->refresh();
		return window;
	}
	catch (std::exception&) {
		if (window)
			delete window;
		throw;
	}
}

QSharedPointer<LogView> LogWindow::getLogView() const
{
	return mainView_;
}

QSharedPointer<DetailView> LogWindow::getDetailView() const
{
	return detailView_;
}

bool LogWindow::query()
{
	model_->query();
	refreshTitle();
	return true;
}

void LogWindow::refreshTitle()
{
	QString title = model_->getTitle();
	setWindowTitle(title);
	if (this->isActiveWindow()) {
		emit refreshWindowTitle();
	}
	mainView_->updateHeader();
}

void LogWindow::setModel(QSharedPointer<LogModel> model)
{
	if (model == nullptr)
		return;
	model->addView(mainView_.data());
	model_ = model;
	mainView_->setModel(model.data());
	detailView_->setModel(model.data());
	connect(model.data(), SIGNAL(layoutChanged()), mainView_.data(), SLOT(dataChanged()));
	connect(mainView_.data(), SIGNAL(scrolltable(QModelIndex)), mainView_.data(), SLOT(doScroll(QModelIndex index)));
	connect(model.data(), SIGNAL(setModifiedPos(quint32)), mainView_.data(), SLOT(setModifiedPos(quint32)));
	connect(mainView_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
		detailView_.data(), SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	//setTabKeyNavigation(true);
}

void LogWindow::writeSettings(_settings::LogWindow& s)
{
	model_->writeSettings(s.queryParams());
	mainView_->writeSettings(s);
	detailView_->writeSettings(s);
	s.splitter(saveState());
}

void LogWindow::readSettings(_settings::LogWindow& s)
{
	model_->readSettings(s.queryParams());
	mainView_->readSettings(s);
	detailView_->readSettings(s);
	restoreState(s.splitter());
}

void LogWindow::setSettings(LogWindow* from)
{
	settings_new::LogWindow tmpSettings;
	from->writeSettings(tmpSettings);
	readSettings(tmpSettings);
}

void LogWindow::setFollowMode(bool enabled)
{
	mainView_->setFollowMode(enabled);
}

void LogWindow::showFindWidget(bool show)
{
	mainView_->showFindWidget(show);
}
