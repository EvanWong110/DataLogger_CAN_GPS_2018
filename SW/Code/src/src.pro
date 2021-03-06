#-------------------------------------------------
#
# Project created by QtCreator 2018-08-07T11:38:36
#
#-------------------------------------------------

include (../defaults.pri)

QT       += core gui widgets charts

TARGET = src
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17 -o0 -g -Wall

SOURCES += \
    AGHConfig/Config.cpp \
    AGHConfig/ConfigFrame.cpp \
    AGHConfig/ConfigSignal.cpp \
    AGHConfig/ValueType.cpp \
    AGHData/CSVSignalsWriter.cpp \
    AGHData/CSVWriterEventMode.cpp \
    AGHData/CSVWriterFrameByFrame.cpp \
    AGHData/CSVWriterStaticFreq.cpp \
    AGHData/DataFileClass.cpp \
    AGHData/SingleCANFrameData.cpp \
    AGHData/SingleGPSFrameData.cpp \
    AGHUtils/FixedPoint.cpp \
    AGHUtils/RawDataParser.cpp \
    AGHUtils/ReadingClass.cpp \
    AGHUtils/WritingClass.cpp \
    configure_logger_sd_dialog.cpp \
    conversion_warnings_dialog.cpp \
    convertfile_thread.cpp \
    downloaddatasd_dialog.cpp \
    downloadingprogress_dialog.cpp \
    main.cpp \
    mainwindow.cpp \
    AGHConfig/ConfigSignalNamedValue.cpp \
    AGHConfig/ConfigTrigger.cpp \
    trigger_dialog.cpp \
    frame_dialog.cpp \
    signal_dialog.cpp \
    AGHData/SingleCANErrorData.cpp \
    AGHData/CSVWriter.cpp


HEADERS += \
    AGHConfig/Config.h \
    AGHConfig/ConfigFrame.h \
    AGHConfig/ConfigSignal.h \
    AGHConfig/ValueType.h \
    AGHData/CSVSignalsWriter.h \
    AGHData/CSVWriter.h \
    AGHData/CSVWriterEventMode.h \
    AGHData/CSVWriterFrameByFrame.h \
    AGHData/CSVWriterStaticFreq.h \
    AGHData/DataFileClass.h \
    AGHData/SingleCANFrameData.h \
    AGHData/SingleGPSFrameData.h \
    AGHData/WritableToCSV.h \
    AGHUtils/FixedPoint.h \
    AGHUtils/RawDataParser.h \
    AGHUtils/ReadingClass.h \
    AGHUtils/WritingClass.h \
    QStringIntMap.h \
    configure_logger_sd_dialog.h \
    conversion_warnings_dialog.h \
    convertfile_thread.h \
    downloaddatasd_dialog.h \
    downloadingprogress_dialog.h \
    mainwindow.h \
    AGHConfig/ConfigSignalNamedValue.h \
    AGHConfig/ConfigTrigger.h \
    trigger_dialog.h \
    frame_dialog.h \
    signal_dialog.h \
    AGHData/SingleCANErrorData.h

FORMS += \
    conversion_warnings_dialog.ui \
    downloaddatasd_dialog.ui \
    mainwindow.ui \
    downloadingprogress_dialog.ui \
    configure_logger_sd_dialog.ui \
    trigger_dialog.ui \
    frame_dialog.ui \
    signal_dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    agh_racing_logo.png

RESOURCES += \
    images.qrc
