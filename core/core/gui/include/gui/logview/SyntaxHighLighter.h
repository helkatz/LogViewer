#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class Highlighter : public QSyntaxHighlighter
{
public:
	Highlighter(QTextDocument* parent = 0);
private:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> rules;


protected:
	void highlightBlock(const QString& text);
};
