#pragma once
#include <QFile>
#include <QSharedPointer>
#include <QMutex>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <qthread.h>
#include <deque>
#include <qfile.h>
#include <qtextstream.h>
#include <qregularexpression.h>
//#include <re2/prog.h>
#include <re2/re2.h>
#include <utils/utils.h>

#include <common/File.h>
struct Columnizer
{
	using List = QVector<Columnizer>;
	std::string name;
	std::string pattern;
	std::string lastPattern;
	std::string startPattern;
	std::string searchPattern;
	QList<QString> columnNames;

	static Columnizer::List _columnizers;

	static void loadAll();
	static Columnizer::List find(common::File& file);
	static boost::optional<Columnizer> find(const QString& name);
	static void loadOne(const QString& name);
};
