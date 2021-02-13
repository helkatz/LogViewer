#pragma once

#include <gui/logview/SyntaxHighLighter.h>
#include <QRegExp>

class MessageFormatter {
	enum class Type {
		Text,
		Json
	};
	struct Format {
		QRegExp pattern;
		Type type;
	};
	QVector<Format> formats;
public:
	MessageFormatter();
	void format(QString& text);
};