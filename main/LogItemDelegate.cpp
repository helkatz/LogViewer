#include "logview.h"
#include <qpainter.h>
#include <qcryptographichash.h>
LogItemDelegate::LogItemDelegate(QObject *parent):
	QStyledItemDelegate(parent)
{

}

QColor LogItemDelegate::xgetColorFromString(QString s, bool sameRGB) const
{
	QString hex = QCryptographicHash::hash(s.toStdString().c_str(),QCryptographicHash::Md5).toHex().toUpper().mid(0, 6);
	if(sameRGB)
		hex = hex.mid(0, 2) + hex.mid(0, 2) + hex.mid(0, 2);
	hex = "0x" + hex;
	bool ok;
	QRgb rgb = hex.toUInt(&ok, 16);
	QColor color(rgb);
	return color;
}

void LogItemDelegate::paint(QPainter *painter,
	const QStyleOptionViewItem &option,
	const QModelIndex &index) const
{
	//qDebug() << "painter" << painter;
	QVariant text = index.model()->data(index, Qt::DisplayRole);
	if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
		if (option.state & QStyle::State_Active)
			painter->fillRect(option.rect, QColor(0xeeeeee));
		else {
			QPalette p = option.palette;
			painter->fillRect(option.rect, p.color(QPalette::Inactive, QPalette::Background));
		}
		painter->drawText(option.rect, option.displayAlignment, text.toString().split("\n").at(0));
		return;
	}
	if (true) {
		//QVariant text = index.model()->data(index, Qt::DisplayRole);
		if (text.type() == QVariant::DateTime) {
			text = text.toDateTime().toString("dd.MMM hh:mm:ss.zzz");
			//text = text.toDateTime().toString();
		}
		QStyleOptionViewItem myOption = option;
		//myOption.displayAlignment 1= Qt::AlignRight | Qt::AlignVCenter;
		QBrush orgBrush = painter->brush();
		QPen savePen = painter->pen();
		LogView *view = qobject_cast<LogView *>(this->parent());
		auto qsum = [](const QString& s) -> quint32 {
			quint32 qsum_ = 0;
			foreach(auto ch, s.toStdString())
			{
				qsum_ += ch;
			}
			return qsum_;
		};
		if (view) {
			// colorize the forground depends on chars
			CellColorizer& colorizer = view->getRowStyle().getCellColorizer(index.column());
			if (colorizer.colorizeByChars) {
				QColor color = colorizer.getColor(index.column(), text.toString());
				QPen pen = painter->pen();
				pen.setColor(color);
				painter->setPen(pen);
			}

			RowColorizer& rColorizer = view->getRowStyle().getRowColorizer();
			// colrize the background
			if (rColorizer.boundColumn && rColorizer.colorizeByChars) {
				QModelIndex colIndex = index.model()->index(index.row(), rColorizer.boundColumn, index);
				auto colorizeByText = index.model()->data(colIndex).toString().mid(0, rColorizer.colorizeByChars);
				QColor color = rColorizer.getColor(index.column(), colorizeByText);
				painter->fillRect(myOption.rect, color);
			}
		}
		auto lineEnd = text.toString().indexOf('\n');
		QString messagePart = text.toString().mid(0, lineEnd);
		view->getRowStyle().textPartColorizer.drawText(painter, myOption, index.column(), messagePart);
		//painter->drawText(myOption.rect.bottomLeft(), text.toString());

		QRect r;
		//QString textLeft = text.toString();
		//myOption.palette.setColor(QPalette::HighlightedText, Qt::blue);
		if (false && index.row() == view->currentIndex().row()) {
			QColor bgColor = QColor(Qt::lightGray);
			painter->fillRect(myOption.rect, bgColor);
		}
		painter->setPen(savePen);
	} else{
		QPen pen(Qt::green, 30, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		painter->setPen(pen);
		QStyleOptionViewItem newOption(option);
		//drawBackground(painter, option, index);

		QStyledItemDelegate::paint(painter, newOption, index);
	}
}

QString LogItemDelegate::xdisplayText(const QVariant &value, const QLocale &) const
{
	if(value.type() == QVariant::DateTime) {
		return value.toDateTime().toString("dd.MMM hh:mm:ss.zzz");
	}
	return value.toString();
}

void LogItemDelegate::xdrawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.column() == 1) {//assume that id keeps in a second column
		const int id = index.data().toInt();
		painter->fillRect(option.rect, Qt::red);
	}
}

