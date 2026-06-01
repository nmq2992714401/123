QT       += core gui multimedia xml svg location positioning network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 解决编译时报 “编译器堆空间不足”的问题
CONFIG += resources_big

# Basemap
# Vector basemap is loaded through QGIS/OGR from third_party/sanya_map.

PROJECT_ROOT = $$clean_path($$PWD)

# OpenCV
OpenCV_INSTALL_PATH = $$PROJECT_ROOT/third_party/OPENCV
INCLUDEPATH += $${OpenCV_INSTALL_PATH}/include

CONFIG(debug, debug|release) {
    LIBS += -L$${OpenCV_INSTALL_PATH}/lib -lopencv_world420d
} else {
    LIBS += -L$${OpenCV_INSTALL_PATH}/lib -lopencv_world420
}

# QGIS
QGIS_INSTALL_PATH = $$PROJECT_ROOT/third_party/QGIS_3.16
INCLUDEPATH += $${QGIS_INSTALL_PATH}/include
win32: LIBS += -L$${QGIS_INSTALL_PATH}/lib -lqgis_core -lqgis_gui -lqgis_app
DEFINES += USE_QGIS _USE_MATH_DEFINES

DDSManager_INSTALL_PATH = $$PROJECT_ROOT/third_party/DDSManager
INCLUDEPATH += $${DDSManager_INSTALL_PATH}/include
win32-msvc* {
    contains(QT_ARCH, i386) {
        DDS_ARCH_DIR = Win32
    } else {
        DDS_ARCH_DIR = x64
    }

    CONFIG(debug, debug|release) {
        DDS_CONFIG_DIR = Debug
        DDS_LIB_SUFFIX = d
    } else {
        DDS_CONFIG_DIR = Release
        DDS_LIB_SUFFIX =
    }

    DDS_LIB_PATH = $${DDSManager_INSTALL_PATH}/lib/$${DDS_ARCH_DIR}/$${DDS_CONFIG_DIR}
    LIBS += -L$${DDS_LIB_PATH} \
        -lnddscpp$${DDS_LIB_SUFFIX} \
        -lnddsc$${DDS_LIB_SUFFIX} \
        -lnddscore$${DDS_LIB_SUFFIX} \
        -lrunConfig$${DDS_LIB_SUFFIX} \
        -lddsLib$${DDS_LIB_SUFFIX}
} else:win32 {
    LIBS += -L$${DDSManager_INSTALL_PATH}/lib -lnddscpp
}


# 如果使用MSVC
DEFINES += USE_QGIS

BUILD_SUBDIR = debug
CONFIG(release, debug|release) {
    BUILD_SUBDIR = release
}

DESTDIR = $$OUT_PWD/bin
OBJECTS_DIR = $$OUT_PWD/obj/$$BUILD_SUBDIR
MOC_DIR = $$OUT_PWD/moc/$$BUILD_SUBDIR
RCC_DIR = $$OUT_PWD/rcc/$$BUILD_SUBDIR
UI_DIR = $$OUT_PWD/ui/$$BUILD_SUBDIR

win32 {
    QGIS_RUNTIME_BIN = $$replace($$DESTDIR, /, \\)
    QGIS_RUNTIME_PATH = $$replace($$QGIS_INSTALL_PATH, /, \\)
    QMAKE_POST_LINK += xcopy /Y /D /Q /I \"$${QGIS_RUNTIME_PATH}\\bin\\*.dll\" \"$${QGIS_RUNTIME_BIN}\\\" $$escape_expand(\\n\\t)
}

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    DDSManager.cpp \
    Simulation.cpp \
    buoyparamdialog.cpp \
    configmanager.cpp \
    dataprocessingwidget.cpp \
    datareceivingwidget.cpp \
    displaycontrolwidget.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    mapwidget.cpp \
    positionacquisitionwidget.cpp \
    qgismanager.cpp \
    remotecontrolwidget.cpp \
    submarinehighlightoverlay.cpp

HEADERS += \
    CommonTypes.h \
    DDSManager.h \
    GlobalDefines.h \
    ModelIO.h \
    Simulation.h \
    buoyparamdialog.h \
    configmanager.h \
    dataprocessingwidget.h \
    datareceivingwidget.h \
    displaycontrolwidget.h \
    logger.h \
    mainwindow.h \
    mapwidget.h \
    positionacquisitionwidget.h \
    qgismanager.h \
    remotecontrolwidget.h \
    submarinehighlightoverlay.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    style.qrc

DISTFILES += \
    config/buoy_config.json \
    config/system_config.json
