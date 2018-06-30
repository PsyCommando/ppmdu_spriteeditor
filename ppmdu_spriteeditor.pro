#-------------------------------------------------
#
# Project created by QtCreator 2017-04-02T16:42:42
#
#-------------------------------------------------

QT       += core gui concurrent svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
GIT_VERSION = $$system(GitVersion -output json -showvariable SemVer) ##(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
GIT_MAJORMINOR_VERSION = $$system(GitVersion -output json -showvariable MajorMinorPatch)
TARGET_FILENAME = ppmdu_spriteeditor


TARGET = ppmdu_spriteeditor
TEMPLATE = app

CONFIG += c++14

RC_FILE = resources.rc

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\" \
           GIT_MAJORMINOR_VERSION=\\\"$$GIT_MAJORMINOR_VERSION\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += src/main.cpp\
        src/ui/mainwindow.cpp \
    src/spritecontainer.cpp \
    src/ui/processingconsole.cpp \
    src/sprite.cpp \
    src/spritemanager.cpp \
    src/ppmdu/fmts/sir0.cpp \
    src/ppmdu/fmts/packfile.cpp \
    src/ppmdu/fmts/px_compression.cpp \
    src/ppmdu/fmts/pxhandler.cpp \
    src/ppmdu/fmts/wa_sprite.cpp \
    src/ppmdu/utils/imgutils.cpp \
    src/ui/diagsingleimgcropper.cpp \
    src/ui/dialognewsprite.cpp \
    src/sprite_anim.cpp \
    src/scenerenderer.cpp \
    src/sprite_img.cpp \
    src/ui/dialogabout.cpp \
    src/ui/dialogprogressbar.cpp \
    src/ppmdu/fmts/compression_handler.cpp \
    src/frameeditor.cpp \
    src/extfmt/riff_palette.cpp \
    src/ppmdu/utils/gfileio.cpp \
    src/ui/paletteeditor.cpp \
    src/sprite_util_impl.cpp \
    src/sprite_palette.cpp \
    src/sprite_frames.cpp \
    src/ui/mw_tabproperties.cpp \
    src/ui/mw_tabframe.cpp \
    src/ui/mw_tabanimtable.cpp \
    src/ui/mw_tabanimseq.cpp \
    src/ui/mw_tabimages.cpp

HEADERS  += src/ui/mainwindow.h \
    src/spritemanager.h \
    src/spritecontainer.h \
    src/ui/processingconsole.h \
    src/sprite.h \
    src/ppmdu/fmts/sir0.hpp \
    src/ppmdu/utils/byteutils.hpp \
    src/ppmdu/fmts/packfile.hpp \
    src/treeelem.hpp \
    src/ppmdu/utils/sequentialgenerator.hpp \
    src/ppmdu/fmts/integer_encoding.hpp \
    src/ppmdu/fmts/px_compression.hpp \
    src/ppmdu/fmts/compression_handler.hpp \
    src/ppmdu/fmts/wa_sprite.hpp \
    src/ppmdu/utils/algo.hpp \
    src/ppmdu/utils/imgutils.hpp \
    src/ui/diagsingleimgcropper.hpp \
    src/ui/dialognewsprite.hpp \
    src/sprite_img.hpp \
    src/sprite_anim.hpp \
    src/scenerenderer.hpp \
    src/ui/dialogabout.hpp \
    src/ui/dialogprogressbar.hpp \
    src/frameeditor.hpp \
    src/extfmt/riff_palette.hpp \
    src/ppmdu/utils/gfileio.hpp \
    src/extfmt/text_palette.hpp \
    src/extfmt/gpl_palette.hpp \
    src/ppmdu/utils/color_utils.hpp \
    src/ui/paletteeditor.hpp \
    src/sprite_frames.hpp \
    src/sprite_palette.hpp \
    version.hpp

FORMS    += src/ui/mainwindow.ui \
    src/ui/processingconsole.ui \
    src/ui/diagsingleimgcropper.ui \
    src/ui/dialognewsprite.ui \
    src/ui/dialogabout.ui \
    src/ui/dialogprogressbar.ui \
    src/ui/paletteeditor.ui

RESOURCES += \
    appres.qrc

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter #-Wno-reorder -Wno-extra
QMAKE_CXXFLAGS_WARN_OFF +=

DISTFILES += \
    resources.rc \
    resources/app.ico
