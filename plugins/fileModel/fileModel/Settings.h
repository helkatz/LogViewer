#pragma once

#include <core\settings.h>


SETTINGSCLASS(LogFileSettings, PropClass,
	_CONFIGURE(
		setPath("plugins/logfile");
		//bindTo(appSettings());
	)

	SETTINGSCLASS(Columnizer, PropClass,
		SETTINGSCLASS(Column, PropClass,
			PROP(QString, name)
			PROP(QString, pattern)
			PROP(QString, format)
			PROP(bool, enabled)
			PROP(QString, fmtFunc)
			PROP(QString, fmtFrom)
			PROP(QString, fmtTo)
		)
		PROP(QString, startPattern)
		PROP(QString, subject)
		PROP(QByteArray, tableState)
		_PROPLIST(Column, columns)
	)

	PROP(int, virtualRowsMinSize)
	_PROPLIST(Columnizer, columnizers)
);