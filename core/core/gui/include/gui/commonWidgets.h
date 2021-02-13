#pragma once
#include <qlineedit.h>
namespace commonwidgets
{
	struct LineEdit : public QLineEdit
	{
		Q_OBJECT;
		using QLineEdit::QLineEdit;
		void focusInEvent(QFocusEvent *e) override
		{
			emit focusIn();
		}
	signals:
		void focusIn();
	};
}