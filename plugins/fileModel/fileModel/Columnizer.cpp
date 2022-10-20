#include "Columnizer.h"
#include "Settings.h"
#include <Utils/utils.h>
#include <gui/ColumnizerChooser.h>

#include <QFile>
#include <QTextStream>
#include <QRegularExpressionMatch>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QThread>

#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include <deque>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <string>
#include <stdlib.h> 

Columnizer::List Columnizer::_columnizers;


void Columnizer::loadAll()
{
	if (_columnizers.length())
		return;

	auto s = appSettings()->as<LogFileSettings>();
	foreach(const QString& name, s.columnizers()->childGroups()) {
		loadOne(name);
	}

	Columnizer columnizer;
	columnizer.name = "default_columnizer";
	columnizer.columnNames.push_back("Message");
	columnizer.pattern = "(.*)";
	columnizer.startPattern = "(.*)";
	_columnizers.push_back(columnizer);
}

Columnizer::List Columnizer::find(common::File& file)
{
	Columnizer::List ret;
	int tryLines = 20;
	auto line = file.readLine();
	foreach(const Columnizer& columnizer, _columnizers) {
		//QRegularExpression re(columnizer.pattern.c_str());
		//QRegularExpressionMatch match = re.match(line);
		boost::regex re(columnizer.pattern.c_str());
		boost::smatch m;
		if (boost::regex_match(line, re) == false)
			continue;
		//if (match.hasMatch() == false || match.capturedTexts().length() == 0)
		//	continue;
		ret.push_back(columnizer);
	}
	return ret;
}

void Columnizer::loadOne(const QString& name)
{
	//LogFileSettings settings;
	auto settings = appSettings()->as<LogFileSettings>();
	auto found = std::find_if(_columnizers.begin(), _columnizers.end(), [&name](const Columnizer& c) {
		return c.name == name.toStdString();
		});
	Columnizer* columnizer = nullptr;
	if (found == _columnizers.end()) {
		_columnizers.push_back(Columnizer{});
		columnizer = &_columnizers.back();
	}
	else {
		columnizer = &*found;
	}

	columnizer->name = name.toStdString();
	foreach(const QString& col, settings.columnizers(name).columns()->childGroups())
	{
		auto s = settings.columnizers(name).columns(col);
		// when not set then set it with the first field pattern
		if (columnizer->startPattern.length() == 0)
			columnizer->startPattern = s.pattern().toStdString();
		if (s.name().length() == 0)
			continue;
		columnizer->columnNames.push_back(s.name());
		columnizer->pattern += s.pattern().toStdString();
		columnizer->lastPattern = s.pattern().toStdString();
		QRegularExpression re("(^.*?)\\((.*?)\\)(.*)");
		QString searchPattern = s.pattern();
		searchPattern.replace(re, QString("\\1(COLUMNNAME_%1)\\3").arg(s.name()));
		columnizer->searchPattern += searchPattern.toStdString();
	}
	std::sort(_columnizers.begin(), _columnizers.end(), [](const auto& lhs, const auto& rhs) -> bool {
		return lhs.columnNames.size() > rhs.columnNames.size() && lhs.name != "default_columnizer";
	});
}

boost::optional<Columnizer> Columnizer::find(const QString & name)
{
	for (auto& columnizer : _columnizers) {
		if (columnizer.name == name.toStdString())
			return columnizer;
	}
	return {};
}

