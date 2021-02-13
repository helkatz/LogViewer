#pragma once
#include <core/settings.h>

//#include <common/properties/Properties.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qexception.h>
#include <qmap.h>
#include <boost/fusion/adapted.hpp>
struct TextColorize
{
	using Ptr = QSharedPointer<TextColorize>;
	using List = QList<TextColorize>;
	using Map = QMap<QString, TextColorize>;

	QString text;
	QFont font;
	bool useRegex = false;
	bool wordOnly = false;
	bool wholeRow = false;
	bool wholeCell = false;
	bool caseSensitive = false;
	QColor foregroundColor;
	QColor backgroundColor;
};

Q_DECLARE_METATYPE(TextColorize);
Q_DECLARE_METATYPE(TextColorize::Ptr);

//BOOST_FUSION_ADAPT_STRUCT(TextColorize, text, font, useRegex, word)
class TextColorizer
{
	TextColorize::List _coloredTextParts;
	TextColorize _findCTP;
	
	void colorizeTextPart(const TextColorize& ctp);
	
	struct Part {
		using List = QList<Part>;
		QString text;
		TextColorize *colorize;
	};
	bool matchParts(const QString& text, Part::List& parts);
public:
	void removeText(const QString& textPart);
	void add(const TextColorize&, int column = -1);
	void set(const TextColorize::List&, int column = -1);
	void setFindText(const QString& textPart, QColor color, int column = -1);
	void unsetFindText(const QString& textPart);
	void drawText(QPainter *painter, const QStyleOptionViewItem &option, int column, const QString& text);
	TextColorize *getByText(const QString& text);
	const TextColorize::List& getList() const { return _coloredTextParts; }

	void writeSettings(_settings::RowStyle&);

	void readSettings(_settings::RowStyle&);
};
