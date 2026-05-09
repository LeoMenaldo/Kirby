# 基础模块
QT       += core gui widgets

# 包含你的所有代码文件
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    gameobject.cpp \
    player.cpp \
    tile.cpp

HEADERS += \
    mainwindow.h \
    gameobject.h \
    player.h \
    tile.h

# 包含你的界面和资源
FORMS += mainwindow.ui
RESOURCES += res.qrc