#include "logview.h"
#include "mainwindow.h"

LogWindow::LogWindow()
{
	//model_ = new LogModel(NULL);
	setOrientation(Qt::Vertical);
	mainView_ = QSharedPointer<LogView>{ new LogView(this) };
	detailView_ = QSharedPointer<DetailView>{ new DetailView(this) };
	//setModel(model_);
	mainView_->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	mainView_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

LogWindow::~LogWindow()
{
	model_->removeView(mainView_.data());
}

LogWindow *LogWindow::create(Conditions qc, bool useTemplate)
{
	LogWindow *window = new LogWindow();
	LogModel *model = nullptr;
	try {
		auto model = LogModelFactory::Create(qc.modelClass());

		if (model == nullptr)
			throw std::exception("invalid modelClass");

		model->setQueryConditions(qc);
		window->setModel(model);
		if (window->query(model->getQueryConditions()) == false) {
			delete window;
			return nullptr;
		}

		if (useTemplate) {
			Settings s;
			foreach(const QString& group, s.childGroups("windowTemplates")) {
				QString filter = s.windowTemplates(group).viewNameFilter();
				QRegExp regex(filter);
				if (model->getTitle().contains(regex))
					window->readSettings(s.windowTemplates(group).logView().getPath());
			}
		}

		window->refreshTitle();
		return window;
	}
	catch (std::exception&) {
		if (window)
			delete window;
		throw;
	}
}

LogWindow *LogWindow::create(Conditions qc, LogView *templateView)
{
	auto window = create(qc, false);
	if (!window)
		return nullptr;

	// now use the present settings technoligy to move needed windows settings
	Settings s;
	QString tmpName = "tmporary_template";
	templateView->writeSettings(s.windowTemplates(tmpName).logView().getPath());
	window->readSettings(s.windowTemplates(tmpName).logView().getPath());
	s.windowTemplates(tmpName).remove();
	return window;
}

LogWindow *LogWindow::create(const QString& settingsPath)
{
	Conditions qc;
	qc.readSettings(settingsPath);
	LogWindow *window = create(qc);
	window->readSettings(settingsPath);
	return window;
}

bool LogWindow::query(const Conditions& qc)
{
	model_->query(qc);
	refreshTitle();
	return true;
}

void LogWindow::refreshTitle()
{
	QString title = model_->getTitle();
	setWindowTitle(title);
	if (this->isActiveWindow()) {
		MainWindow::instance().refreshWindowTitle();
	}
	mainView_->updateHeader();
}

void LogWindow::setModel(QSharedPointer<LogModel> model)
{
	//QAbstractItemModel *itemModel = dynamic_cast<QAbstractItemModel *>(model);
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

void LogWindow::writeSettings(const QString &basePath)
{
	Settings s(basePath);
	model_->writeSettings(basePath);
	mainView_->writeSettings(basePath);
	detailView_->writeSettings(basePath);
	s.view().splitter(saveState());
}

void LogWindow::readSettings(const QString &basePath)
{
	Settings s(basePath);
	model_->readSettings(basePath);
	mainView_->readSettings(basePath);
	detailView_->readSettings(basePath);
	restoreState(s.view().splitter());
}

void LogWindow::setFollowMode(bool enabled)
{
	mainView_->setFollowMode(enabled);
}

void LogWindow::showFindWidget(bool show)
{
	mainView_->showFindWidget(show);
}
