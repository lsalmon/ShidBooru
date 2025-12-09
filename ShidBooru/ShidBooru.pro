QT       += core gui widgets sql multimedia multimediawidgets concurrent

greaterThan(QT_MAJOR_VERSION, 5): QT +=

CONFIG += c++11
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

INCLUDEPATH += /usr/include/opencv4
DEPENDPATH += /usr/include/opencv4

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += opencv4

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BooruMenu.cpp \
    ItemContextMenu.cpp \
    ItemEditor.cpp \
    ItemEditorDelegate.cpp \
    MainMenu.cpp \
    QSqlQueryHelper.cpp \
    SearchTagDialog.cpp \
    SelectFilesDialog.cpp \
    main.cpp

HEADERS += \
    BooruItemType.h \
    BooruMenu.h \
    HelperFunctions.h \
    ItemContextMenu.h \
    ItemEditor.h \
    ItemEditorDelegate.h \
    LoadingAnimation.h \
    MainMenu.h \
    QSqlQueryHelper.h \
    SearchTagDialog.h \
    SelectFilesDialog.h \
    TagFilterProxyModel.h

FORMS += \
    BooruMenu.ui \
    ItemEditor.ui \
    LoadingAnimation.ui \
    MainMenu.ui \
    SearchTagDialog.ui \
    SelectFilesDialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    MainMenuIcon.qrc
