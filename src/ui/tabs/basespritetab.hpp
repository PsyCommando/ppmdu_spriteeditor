#ifndef BASESPRITETAB_HPP
#define BASESPRITETAB_HPP
/*
BaseSpriteTab
    Base class for tabs meant to be used for editing sprites data
*/
#include <QUndoCommand>
#include <QWidget>
#include <src/data/treeenodetypes.hpp>

class Sprite;
class MFrame;
class Image;
class AnimSequence;
class MFramePart;
class MainWindow;


class BaseSpriteTab : public QWidget
{
    Q_OBJECT
public:
    explicit BaseSpriteTab(QWidget *parent = nullptr);
    virtual ~BaseSpriteTab();

    /*
     * Sets the main windoe
    */
    virtual void setMainWindow(MainWindow * parentwindow);

    virtual MainWindow * getMainWindow();

    /*
     * When the tab is about to be displayed, pass the current sprite and
     * the currently selected element's modelindex.
    */
    virtual void OnShowTab(QPersistentModelIndex element);

    /*
     * Called when the tab is about to be hidden
    */
    virtual void OnHideTab();

    /*
     * Called before the destructor when the application is shutting down.
     * Meant to be used for clearing references to shared resources.
    */
    virtual void OnDestruction();

    /*
     * Called when an item is removed from the data.
    */
    virtual void OnItemRemoval(const QModelIndex & item);

    /*
    * Called when we're about to load a new container
    */
    virtual void PrepareForNewContainer()=0;

    /*
     * Whether this tab should be displayed for a given element type
    */
    virtual bool canAcceptContentType(const QString & contenttype)const;

    /*
     * Helpers Main Window procs
    */
    void ShowStatusErrorMessage(QString msg);
    void ShowStatusMessage(QString msg);
    void Warn(const QString & title, const QString & text);

    /*
     * Helper Data access
    */
    Sprite * currentSprite();
    MFrame * currentFrame();
    Image * currentImage();
    AnimSequence * currentAnimSequence();
    eTreeElemDataType currentEntryType();
//    MFramePart * currentTblFrameParts();
//    Image * currentTblImages();

signals:
protected:
    MainWindow * m_pmainwindow;
};

#endif // BASESPRITETAB_HPP

