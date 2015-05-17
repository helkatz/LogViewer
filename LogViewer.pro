#-------------------------------------------------
#
# Project created by QtCreator 2014-05-20T20:41:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += sql webkit webkitwidgets
TARGET = LogViewer
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    logview.cpp \
    settings.cpp \
    logupdater.cpp \
    logmodel.cpp \
    logsqlmodel.cpp \
    utils.cpp \
    signalmapper.cpp \
    logfilemodel.cpp \
    queryconditions.cpp \
    forms/statusbar.cpp \
    forms/aboutdialog.cpp \
    forms/connectionswidget.cpp \
    forms/aboutdialog.cpp \
    forms/querydialog.cpp \
    forms/qsqlconnectiondialog.cpp \
    forms/settingsdialog.cpp \
    forms/logviewcontextmenu.cpp \
    forms/contextmenu.cpp \
    forms/contextmenufilterdialog.cpp \
    forms/generellwidget.cpp \
    forms/finddialog.cpp \
    forms/findwidget.cpp \
    forms/templateswidget.cpp \
    forms/rowlayoutwidget.cpp \
    forms/qtcolorpicker.cpp \
    forms/fontstylewidget.cpp

HEADERS  += \
    mainwindow.h \
    logview.h \
    settings.h \
    logupdater.h \
    logmodel.h \
    logsqlmodel.h \
    utils.h \
    signalmapper.h \
    logfilemodel.h \
    queryconditions.h \
    forms/statusbar.h \
    forms/aboutdialog.h \
    forms/aboutdialog.h \
    forms/querydialog.h \
    forms/qsqlconnectiondialog.h \
    forms/settingsdialog.h \
    forms/connectionswidget.h \
    forms/generellwidget.h \
    forms/logviewcontextmenu.h \
    forms/contextmenu.h \
    forms/contextmenufilterdialog.h \
    forms/finddialog.h \
    forms/findwidget.h \
    forms/templateswidget.h \
    forms/rowlayoutwidget.h \
    forms/qtcolorpicker.h \
    Properties.h \
    forms/fontstylewidget.h

FORMS += \
    mainwindow.ui \
    forms/aboutdialog.ui \
    forms/qsqlconnectiondialog.ui \
    forms/settingsdialog.ui \
    forms/connectionswidget.ui \
    forms/querydialog.ui \
    forms/generellwidget.ui \
    forms/statusbar.ui \
    forms/logviewcontextmenu.ui \
    forms/contextmenu.ui \
    forms/contextmenufilterdialog.ui \
    forms/finddialog.ui \
    forms/findwidget.ui \
    forms/templateswidget.ui \
    forms/rowlayoutwidget.ui \
    forms/fontstylewidget.ui

RESOURCES += \
    LogViewer.qrc \
    recource.qrc

OTHER_FILES += \
          LogViewer.ico
RC_FILE = LogViewer.rc

wince*: {
    DEPLOYMENT_PLUGIN += qsqlite
}
