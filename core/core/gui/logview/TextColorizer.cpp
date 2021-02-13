#include <gui/logview/TextColorizer.h>
#include <core/settings.h>

#include <common/Logger.h>

#include <qpainter.h>
#include <qexception.h>

void TextColorizer::add(const TextColorize& colorize, int column)
{
	auto item = getByText(colorize.text);
	if (item == nullptr)
		_coloredTextParts.push_back(colorize);
	else
		*item = colorize;
	column = -1;	
}

void TextColorizer::set(const TextColorize::List& colorizeList, int column)
{
	_coloredTextParts = colorizeList;
}

bool TextColorizer::matchParts(const QString & text, Part::List & parts)
{
	QRegularExpression::PatternOptions options;

	QString rePattern;
	foreach(TextColorize ctp, _coloredTextParts) {
		QString pattern;
		if (ctp.caseSensitive == false)
			pattern += "(?i)";
		pattern += ctp.text;
		pattern.prepend("(").append(")");
		if (ctp.wordOnly)
			pattern.prepend("(?:^|[^\\w])").append("(?:[^\\w]|$)");
		rePattern += "|" + pattern;
	}
	rePattern = rePattern.mid(1);
	QRegularExpression re(rePattern, options);

	QRegularExpressionMatchIterator it = re.globalMatch(text);

	int nonMatchedStart = 0;
	bool hasMatches = it.hasNext();
	TextColorize *wholeLineColor = nullptr;
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		if (match.captured(0).length() == 0)
			continue;
		int matchedGroup = match.lastCapturedIndex();
		while (match.capturedTexts().at(matchedGroup).length() && --matchedGroup);

		/*qDebug()
			<< match.capturedTexts() << " - "
			<< match.capturedView() << " - "
			<< match.hasPartialMatch() << " - "
			<< matchedGroup;*/
		int nonMatechedEnd = match.capturedStart(0);
		int nonMatchedLength = nonMatechedEnd - nonMatchedStart;
		auto& ct = _coloredTextParts[matchedGroup];
		if (ct.wholeRow)
			wholeLineColor = &ct;
		if (nonMatchedLength)
			parts.push_back({ text.mid(nonMatchedStart, nonMatchedLength), nullptr });

		parts.push_back({ text.mid(match.capturedStart(0), match.capturedLength(0)), &ct });
		nonMatchedStart = match.capturedEnd(0);
	}
	if (nonMatchedStart < text.length())
		parts.push_back({ text.mid(nonMatchedStart), nullptr });
	if (wholeLineColor) {
		for (auto& part : parts)
			part.colorize = part.colorize == nullptr ? wholeLineColor : part.colorize;
	}
	return hasMatches;
}

void TextColorizer::removeText(const QString &text)
{
	std::remove_if(_coloredTextParts.begin(), _coloredTextParts.end(), [text](auto colorize) {
		return colorize.text == text;
	});
}

void TextColorizer::setFindText(const QString &textPart, QColor color, int column)
{
	_findCTP.foregroundColor = color;
	_findCTP.text = textPart;
}

void TextColorizer::unsetFindText(const QString &textPart)
{
	_findCTP.foregroundColor = QColor(0, 0, 0);
	_findCTP.text = "";
}

void TextColorizer::drawText(QPainter *painter, const QStyleOptionViewItem &option, int column, const QString &text)
{	
	column = -1;
	if (_findCTP.text.length()) {
		_coloredTextParts.push_back(_findCTP);
	}

	QRect rFill = option.rect;
	QRect rect;

	Part::List textParts;
	bool hasMatches = matchParts(text, textParts);
	int penPos = option.rect.left();

	QFont saveFont = painter->font();
	QPen savePen = painter->pen();

	QRect rectAvail = option.rect;
	bool textExeedBouderies = false;
	foreach(auto& part, textParts) {
		bool colorizePart = part.colorize != nullptr;
		/*bool colorizePart = part.matched
			|| hasMatches && (ctp.wholeCell || ctp.wholeRow);*/
		if (colorizePart) {
			painter->setFont(part.colorize->font);
			painter->setPen(part.colorize->foregroundColor);
		}

		QRect rectTextPart = painter->boundingRect(rectAvail, option.displayAlignment | Qt::TextWordWrap, part.text);
		if (rectTextPart.right() > rectAvail.right()) {
			rectTextPart.setRight(rectAvail.right());
			textExeedBouderies = true;
		}
		rectTextPart.moveLeft(penPos);
		rectAvail.adjust(rectTextPart.width(), 0, 0, 0);
		penPos += rectTextPart.width();
		
		if (colorizePart == false) {
			painter->drawText(rectTextPart, option.displayAlignment, part.text);
			continue;
		}

		painter->fillRect(rectTextPart, part.colorize->backgroundColor);
		painter->drawText(rectTextPart, option.displayAlignment, part.text);

		painter->setFont(saveFont);
		painter->setPen(savePen);
	}

	if (_findCTP.text.length())
		_coloredTextParts.pop_back();
	try {
		if(false)
			painter->drawText(option.rect, option.displayAlignment, text);
	}
	catch (QException& e) {
		log_error() << e.what();
	}
	catch (...) {
	}
	return;
}

TextColorize *TextColorizer::getByText(const QString &text)
{
	auto it = std::find_if(_coloredTextParts.begin(), _coloredTextParts.end(), [text](auto colorize) {
		return colorize.text == text;
	});
	if (it != _coloredTextParts.end())
		return &*it;
	return nullptr;
}

void TextColorizer::writeSettings(_settings::RowStyle& s)
{
	int idx = 0;
	foreach(TextColorize colorize, _coloredTextParts) {
		s.textColorizer(idx).text(colorize.text);
		s.textColorizer(idx).font(colorize.font);
		s.textColorizer(idx).useRegex(colorize.useRegex);
		s.textColorizer(idx).wordOnly(colorize.wordOnly);
		s.textColorizer(idx).wholeRow(colorize.wholeRow);
		s.textColorizer(idx).caseSensitive(colorize.caseSensitive);
		s.textColorizer(idx).foregroundColor(colorize.foregroundColor);
		s.textColorizer(idx).backgroundColor(colorize.backgroundColor);
		idx++;
	}
}

void TextColorizer::readSettings(_settings::RowStyle& s)
{
	_coloredTextParts.clear();
	for(auto pair: s.textColorizerList()) {
		auto& colorizer = pair.second;
		TextColorize colorize;
		colorize.text = colorizer.text();
		colorize.font = colorizer.font();
		colorize.useRegex = colorizer.useRegex();
		colorize.wordOnly = colorizer.wordOnly();
		colorize.wholeRow = colorizer.wholeRow();
		colorize.caseSensitive = colorizer.caseSensitive();
		colorize.foregroundColor = colorizer.foregroundColor();
		colorize.backgroundColor = colorizer.backgroundColor();
		_coloredTextParts.push_back(colorize);
	}
	QMap<QString, int> m;
	for (auto v : m) {
		v = 1;
	}
}