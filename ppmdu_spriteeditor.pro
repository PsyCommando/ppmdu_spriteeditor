#-------------------------------------------------
#
# Project created by QtCreator 2017-04-02T16:42:42
#
#-------------------------------------------------

QT       += core gui concurrent svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ppmdu_spriteeditor
TEMPLATE = app

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    src/spritecontainer.cpp \
    processingconsole.cpp \
    src/sprite.cpp \
    src/spritemanager.cpp \
    src/ppmdu/fmts/sir0.cpp \
    src/ppmdu/fmts/packfile.cpp \
    src/ppmdu/fmts/px_compression.cpp \
    src/ppmdu/fmts/pxhandler.cpp \
    src/ppmdu/fmts/wa_sprite.cpp \
    src/ppmdu/utils/imgutils.cpp \
    tabmanagers.cpp \
    diagsingleimgcropper.cpp \
    dialognewsprite.cpp \
    src/sprite_anim.cpp \
    src/scenerenderer.cpp \
    src/sprite_img.cpp \
    dialogexport.cpp \
    dialogabout.cpp \
    dialogprogressbar.cpp \
    src/ppmdu/fmts/compression_handler.cpp \
    src/frameeditor.cpp \
    src/extfmt/riff_palette.cpp \
    src/ppmdu/utils/gfileio.cpp \
    paletteeditor.cpp \
    src/sprite_util_impl.cpp \
    src/sprite_palette.cpp \
    src/sprite_frames.cpp

HEADERS  += mainwindow.h \
    src/spritemanager.h \
    src/spritecontainer.h \
    processingconsole.h \
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
    tabmanagers.hpp \
    diagsingleimgcropper.hpp \
    dialognewsprite.hpp \
    src/sprite_img.hpp \
    src/sprite_anim.hpp \
    src/scenerenderer.hpp \
    dialogexport.hpp \
    dialogabout.hpp \
    dialogprogressbar.hpp \
    src/frameeditor.hpp \
    src/extfmt/riff_palette.hpp \
    src/ppmdu/utils/gfileio.hpp \
    src/extfmt/text_palette.hpp \
    src/extfmt/gpl_palette.hpp \
    src/ppmdu/utils/color_utils.hpp \
    paletteeditor.hpp \
    src/sprite_frames.hpp \
    src/sprite_palette.hpp

FORMS    += mainwindow.ui \
    processingconsole.ui \
    diagsingleimgcropper.ui \
    dialognewsprite.ui \
    dialogexport.ui \
    dialogabout.ui \
    dialogprogressbar.ui \
    paletteeditor.ui

RESOURCES += \
    appres.qrc

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter #-Wno-reorder -Wno-extra
QMAKE_CXXFLAGS_WARN_OFF +=

DISTFILES +=
