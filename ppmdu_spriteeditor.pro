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

CONFIG += c++17

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
    src/data/content_factory.cpp \
    src/data/content_manager.cpp \
    src/data/sprite/animframe.cpp \
    src/data/sprite/animgroup.cpp \
    src/data/sprite/animsequence.cpp \
    src/data/sprite/animsequences.cpp \
    src/data/sprite/animtable.cpp \
    src/data/sprite/effectoffsetcontainer.cpp \
    src/data/sprite/frame.cpp \
    src/data/sprite/framepart.cpp \
    src/data/sprite/framescontainer.cpp \
    src/data/sprite/image.cpp \
    src/data/sprite/imagescontainer.cpp \
    src/data/sprite/models/animframe_delegate.cpp \
    src/data/sprite/models/animframe_model.cpp \
    src/data/sprite/models/animgroup_delegate.cpp \
    src/data/sprite/models/animgroup_model.cpp \
    src/data/sprite/models/animsequences_list_model.cpp \
    src/data/sprite/models/animsequences_list_picker_model.cpp \
    src/data/sprite/models/animtable_delegate.cpp \
    src/data/sprite/models/animtable_model.cpp \
    src/data/sprite/models/effect_container_model.cpp \
    src/data/sprite/models/framepart_delegate.cpp \
    src/data/sprite/models/framepart_model.cpp \
    src/data/sprite/models/frames_list_model.cpp \
    src/data/sprite/models/images_list_model.cpp \
    src/data/sprite/models/palette_model.cpp \
    src/data/sprite/models/sprite_overview_model.cpp \
    src/data/sprite/models/sprite_props_delegate.cpp \
    src/data/sprite/models/sprite_props_handler.cpp \
    src/data/sprite/models/sprite_props_model.cpp \
    src/data/sprite/palettecontainer.cpp \
    src/data/sprite/sprite_container.cpp \
    src/utility/file_support.cpp \
    src/utility/randomgenhelper.cpp \
    src/ui/editor/frame/frame_editor.cpp \
    src/ui/editor/frame/frame_editor_part.cpp \
    src/ui/editor/palette/paletteeditor.cpp \
    src/ui/errorhelper.cpp \
    src/ui/mainwindow.cpp \
    src/ui/processingconsole.cpp \
    src/data/sprite/sprite.cpp \
    src/data/sprite/spritemanager.cpp \
    src/ppmdu/fmts/sir0.cpp \
    src/ppmdu/fmts/packfile.cpp \
    src/ppmdu/fmts/px_compression.cpp \
    src/ppmdu/fmts/pxhandler.cpp \
    src/ppmdu/fmts/wa_sprite.cpp \
    src/ppmdu/utils/imgutils.cpp \
    src/ui/diagsingleimgcropper.cpp \
    src/ui/dialognewsprite.cpp \
    src/scenerenderer.cpp \
    src/ui/dialogabout.cpp \
    src/ui/dialogprogressbar.cpp \
    src/ppmdu/fmts/compression_handler.cpp \
    src/extfmt/riff_palette.cpp \
    src/ppmdu/utils/gfileio.cpp \
    src/data/sprite/sprite_util_impl.cpp \
    src/ui/rendering/animated_sprite_item.cpp \
    src/ui/rendering/sprite_renderer.cpp \
    src/ui/rendering/sprite_scene.cpp \
    src/ui/tabs/basespritetab.cpp \
    src/ui/tabs/tabanimsequence.cpp \
    src/ui/tabs/tabanimtable.cpp \
    src/ui/tabs/tabeffects.cpp \
    src/ui/tabs/tabframes.cpp \
    src/ui/tabs/tabimages.cpp \
    src/ui/tabs/tabproperties.cpp \
    src/ui/tvspritescontextmenu.cpp \
    src/utility/threadedwriter.cpp

HEADERS  += \
    src/data/base_container.hpp \
    src/data/content_factory.hpp \
    src/data/content_manager.hpp \
    src/data/sprite/models/animframe_delegate.hpp \
    src/data/sprite/models/animframe_model.hpp \
    src/data/sprite/models/animgroup_delegate.hpp \
    src/data/sprite/models/animgroup_model.hpp \
    src/data/sprite/models/animsequences_list_model.hpp \
    src/data/sprite/models/animsequences_list_picker_model.hpp \
    src/data/sprite/models/animtable_delegate.hpp \
    src/data/sprite/models/animtable_model.hpp \
    src/data/sprite/models/effect_container_model.hpp \
    src/data/sprite/models/framepart_delegate.hpp \
    src/data/sprite/models/framepart_model.hpp \
    src/data/sprite/models/frames_list_model.hpp \
    src/data/sprite/models/images_list_model.hpp \
    src/data/sprite/models/palette_model.hpp \
    src/data/sprite/models/sprite_overview_model.hpp \
    src/data/sprite/models/sprite_props_delegate.hpp \
    src/data/sprite/models/sprite_props_handler.hpp \
    src/data/sprite/models/sprite_props_model.hpp \
    src/data/sprite/sprite_container.hpp \
    src/data/treeenodetypes.hpp \
    src/data/treenode.hpp \
    src/data/treenodemodel.hpp \
    src/data/treenodeterminal.hpp \
    src/data/treenodewithchilds.hpp \
    src/utility/baseqtexception.hpp \
    src/data/sprite/animframe.hpp \
    src/data/sprite/animgroup.hpp \
    src/data/sprite/animsequence.hpp \
    src/data/sprite/animsequences.hpp \
    src/data/sprite/animtable.hpp \
    src/data/sprite/effectoffsetcontainer.hpp \
    src/data/sprite/frame.hpp \
    src/data/sprite/framepart.hpp \
    src/data/sprite/framescontainer.hpp \
    src/data/sprite/image.hpp \
    src/data/sprite/imagescontainer.hpp \
    src/data/sprite/palettecontainer.hpp \
    src/utility/file_support.hpp \
    src/utility/randomgenhelper.hpp \
    src/data/sprite/sprite.hpp \
    src/data/sprite/spritemanager.hpp \
    src/ppmdu/fmts/sir0.hpp \
    src/ppmdu/utils/byteutils.hpp \
    src/ppmdu/fmts/packfile.hpp \
    src/ppmdu/utils/sequentialgenerator.hpp \
    src/ppmdu/fmts/integer_encoding.hpp \
    src/ppmdu/fmts/px_compression.hpp \
    src/ppmdu/fmts/compression_handler.hpp \
    src/ppmdu/fmts/wa_sprite.hpp \
    src/ppmdu/utils/algo.hpp \
    src/ppmdu/utils/imgutils.hpp \
    src/ui/diagsingleimgcropper.hpp \
    src/ui/dialognewsprite.hpp \
    src/scenerenderer.hpp \
    src/ui/dialogabout.hpp \
    src/ui/dialogprogressbar.hpp \
    src/extfmt/riff_palette.hpp \
    src/ppmdu/utils/gfileio.hpp \
    src/extfmt/text_palette.hpp \
    src/extfmt/gpl_palette.hpp \
    src/ppmdu/utils/color_utils.hpp \
    src/ui/editor/frame/frame_editor.hpp \
    src/ui/editor/frame/frame_editor_part.hpp \
    src/ui/editor/palette/paletteeditor.hpp \
    src/ui/errorhelper.hpp \
    src/ui/mainwindow.hpp \
    src/ui/processingconsole.hpp \
    src/ui/rendering/animated_sprite_item.hpp \
    src/ui/rendering/sprite_renderer.hpp \
    src/ui/rendering/sprite_scene.hpp \
    src/ui/tabs/basespritetab.hpp \
    src/ui/tabs/tabanimsequence.hpp \
    src/ui/tabs/tabanimtable.hpp \
    src/ui/tabs/tabeffects.hpp \
    src/ui/tabs/tabframes.hpp \
    src/ui/tabs/tabimages.hpp \
    src/ui/tabs/tabproperties.hpp \
    src/ui/tvspritescontextmenu.hpp \
    src/utility/threadedwriter.hpp \
    version.hpp

FORMS    += src/ui/mainwindow.ui \
    src/ui/editor/palette/paletteeditor.ui \
    src/ui/processingconsole.ui \
    src/ui/diagsingleimgcropper.ui \
    src/ui/dialognewsprite.ui \
    src/ui/dialogabout.ui \
    src/ui/dialogprogressbar.ui \
    src/ui/tabs/tabanimsequence.ui \
    src/ui/tabs/tabanimtable.ui \
    src/ui/tabs/tabeffects.ui \
    src/ui/tabs/tabframes.ui \
    src/ui/tabs/tabimages.ui \
    src/ui/tabs/tabproperties.ui

RESOURCES += \
    appres.qrc

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter #-Wno-reorder -Wno-extra
QMAKE_CXXFLAGS_WARN_OFF +=

DISTFILES += \
    readme.md \
    resources.rc \
    resources/app.ico
