#include "logview.h"
#include <qpainter.h>
#include <qexception.h>

void TextPartColorizer::addText(const QString &textPart, QColor color, int column)
{
	column = -1;
	TColoredTextPart ctp;
	ctp.color = color;
	ctp.textPart = textPart;
	_coloredTextParts[textPart] = ctp;
}


void TextPartColorizer::removeText(const QString &textPart)
{
	_coloredTextParts.remove(textPart);
}

void TextPartColorizer::setFindText(const QString &textPart, QColor color, int column)
{
	_findCTP.color = color;
	_findCTP.textPart = textPart;
}

void TextPartColorizer::unsetFindText(const QString &textPart)
{
	_findCTP.color = QColor(0, 0, 0);
	_findCTP.textPart = "";
}

void TextPartColorizer::drawText(QPainter *painter, const QStyleOptionViewItem &option, int column, const QString &text)
{
	column = -1;
	if (_findCTP.textPart.length()) {
		_coloredTextParts["__find_text_part"] = _findCTP;
	}
	foreach(TColoredTextPart ctp, _coloredTextParts) {
		QString textLeft = text;
		int pos = textLeft.indexOf(ctp.textPart, 0, Qt::CaseInsensitive);
		QRect rFill = option.rect;
		while(pos >= 0) {
			QString textPart = textLeft.mid(0, pos);
			textLeft = textLeft.mid(pos);
			QRect rLeft = painter->boundingRect(option.rect, option.decorationAlignment, textPart);
			QRect rPart = painter->boundingRect(option.rect, option.decorationAlignment, ctp.textPart);
			rFill.setTop(rPart.top());
			rFill.setHeight(rPart.height());
			rFill.setLeft(rFill.left() + rLeft.width());
			rFill.setWidth(rPart.width());

			painter->fillRect(rFill, ctp.color);
			if(ctp.textPart.length() == 0)
				break;
			textLeft = textLeft.mid(ctp.textPart.length());
			pos = textLeft.indexOf(ctp.textPart, 0, Qt::CaseInsensitive);
		}
	}
	if (_findCTP.textPart.length())
		_coloredTextParts.remove("__find_text_part");
	try {
		painter->drawText(option.rect, option.displayAlignment, text);
	}
	catch (QException& e) {
		log_error() << e.what();
	}
	catch (...) {
	}
	return;
}

TColoredTextPart *TextPartColorizer::getByText(const QString &text)
{
	if(_coloredTextParts.find(text) != _coloredTextParts.end())
		return &(*_coloredTextParts.find(text));
	return NULL;
}
