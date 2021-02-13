#include <gui/logview/LogView.h>
#include <gui/FontStyleWidget.h>
#include <gui/logview/SyntaxHighLighter.h>
#include <gui/logview/MessageFormatter.h>
#include <qcheckbox.h>
#include <qwidgetaction.h>
#include <qscrollbar.h>
#include <qmenu.h>
#include <qthread.h>
#include <boost/regex.hpp>
#include <QRegExp>
#include <QDebug>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QJsonDocument>

MessageFormatter::MessageFormatter() {
		
	Format fmt;
	fmt.type = Type::Json;
	QString pattern = R"(
		(?(DEFINE)
		# Note that everything is atomic, JSON does not need backtracking if it's valid
		# and this prevents catastrophic backtracking
		(?<json>(?>\\s*(?&object)\\s*|\\s*(?&array)\\s*))
		(?<object>(?>\{\\s*(?>(?&pair)(?>\\s*,\\s*(?&pair))*)?\\s*\}))
		(?<pair>(?>(?&STRING)\\s*:\\s*(?&value)))
		(?<array>(?>\[\\s*(?>(?&value)(?>\\s*,\\s*(?&value))*)?\\s*\]))
		(?<value>(?>true|false|null|(?&STRING)|(?&NUMBER)|(?&object)|(?&array)))
		(?<STRING>(?>"(?>\\(?>["\\\/bfnrt]|u[a-fA-F0-9]{4})|[^"\\\0-\x1F\x7F]+)*"))
		(?<NUMBER>(?>-?(?>0|[1-9][0-9]*)(?>\.[0-9]+)?(?>[eE][+-]?[0-9]+)?))
		)
		(?&json))
	)";

	//boost::regex re(pattern.toStdString());
	//re.error_code();
	QString err;
	if(!fmt.pattern.isValid())
		err = fmt.pattern.errorString();
	formats.push_back(fmt);
}

//struct Parser {
//	QChar* data_;
//	QChar* cur_;
//	Parser(QChar* data)
//		: data_(data)
//		, cur_(data)
//	{}
//
//	int parseIdentifier() {
//		while (*cur_ != 0) {
//	}
//	Parser& parseObject() {
//		while (*cur_ != 0) {
//			if (*cur_ == '{') {
//				parseObject();
//			}
//		}
//	}
//
//	}
//};
void MessageFormatter::format(QString& text) {

	QChar *psz = text.data();
	enum State {
		InCurlyBracket,
		InSquareBracket
	};
	int openPos = 0;
	int closePos = 0;
	int openCurly = 0;
	int openSquare = 0;
	while (*psz != 0) {
		if (*psz == '{' && openSquare == 0) {
			openCurly++;
			openPos = psz - text.data();
		}
		else if (*psz == '[' && openCurly == 0) {
			openSquare++;
			openPos = psz - text.data();
		}
		else if (*psz == '}' && openCurly > 0) {
			openCurly--;
			if (openCurly == 0) {
				closePos = psz - text.data();
			}
		}
		else if (*psz == ']' && openSquare > 0) {
			openSquare--;
			if (openSquare == 0) {
				closePos = psz - text.data();
			}
		}
		psz++;
	}

	if (openPos > 0 && closePos > openPos) {
		QJsonDocument jdoc;
		auto jtext = text.mid(openPos, (closePos - openPos) + 1).toStdString();
		jdoc.fromRawData(jtext.c_str(), jtext.size());
		auto formatted = jdoc.toJson(QJsonDocument::JsonFormat::Indented);
		jtext = formatted.toStdString();
	}
	for (auto& fmt : formats) {
		QRegExp expression(fmt.pattern);
		int index = expression.indexIn(text);
		while (index >= 0) {
			index = expression.pos(1);
			int length = expression.cap(1).length();
			index = expression.indexIn(text, index + length);
		}
	}
}

Highlighter::Highlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	//numbers
	rule.pattern = QRegExp("([-0-9.]+)(?!([^\"]*\"[\\s]*\\:))");
	rule.format.setForeground(QColor(174, 129, 248));
	rules.append(rule);

	//key
	rule.pattern = QRegExp("(\"[^\"]*\")");
	rule.format.setForeground(QColor(102, 217, 239));
	rules.append(rule);

	//value
	rule.pattern = QRegExp(":\\s*([\"](?:[^\"])*[\"])");
	rule.format.setForeground(QColor(220, 229, 236));
	rules.append(rule);

	//reserved words
	rule.pattern = QRegExp("(true|false|null)(?!\"[^\"]*\")");
	rule.format.setForeground(QColor(142, 189, 0));
	rules.append(rule);
}

void Highlighter::highlightBlock(const QString& text)
{
	foreach(const HighlightingRule & rule, rules) {
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);

		while (index >= 0) {
			index = expression.pos(1);
			int length = expression.cap(1).length();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}
}

