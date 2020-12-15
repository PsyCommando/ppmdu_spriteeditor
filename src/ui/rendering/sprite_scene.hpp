#ifndef SPRITE_SCENE_HPP
#define SPRITE_SCENE_HPP
#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include <src/ui/rendering/animated_sprite_item.hpp>
#include <src/ui/rendering/sprite_renderer.hpp>

/*
 * Scene renderer for animated sprites
 * Basically an object that manages the preview scene, and can be plugged into one or more viewports!
 *
 * In order to use this, you wanna first install the anim preview onto a viewport. (make sure to uninstall after use)
 *
 * The structure is basically:
 * - A sprite scene creates and load an animated_sprite_item, which contains a cached animation sequence. And sends it ticks message so it updates what's painted in the viewport.
 * -
*/
class SpriteScene : public QObject
{
    Q_OBJECT
public:
    using animseqid_t = fmt::animseqid_t;

    explicit SpriteScene(QObject *parent = 0);
    virtual ~SpriteScene();

    /*
     * Removes the current anim sprite object from the scene, and stop playback.
    */
    void Clear();

    /*
     * Clears the current animsprite, stop playback, destroy the timer, and reset the ticks counter
    */
    void Reset();

    /*
     * Returns the internal QGraphicScene object managing the preview scene!
    */
    inline QGraphicsScene       & getAnimScene()        {return m_animScene;}
    inline const QGraphicsScene & getAnimScene()const   {return m_animScene;}

    /*
     * Returns a pointer to the animated sprite object if there's any.
    */
    inline AnimatedSpriteItem       * getAnimSprite()       {return m_animsprite.get();}
    inline const AnimatedSpriteItem * getAnimSprite()const  {return m_animsprite.get();}

    /*
     * Returns the "transparency" color for this sprite. The color usually used for the
     * sprite backround.
    */
    QColor getSpriteBGColor()const;

    /*
     * InstallAnimPreview
     * Renders the given animation for a given sprite into the specified viewport.
    */
    void InstallAnimPreview(QGraphicsView * pview, const Sprite * pspr, const AnimSequence * paniseq);

    /*
     * UninstallAnimPreview
     * Stops displaying in the specified viewport.
    */
    void UninstallAnimPreview(QGraphicsView * pview);

    /*
     * setSprite
     * Sets the sprite to render. Must also specify the animation sequence.
    */
    void setSprite(const Sprite * pspr, const AnimSequence * paniseq);
    void setSprite(const Sprite * pspr, fmt::animseqid_t seqid = 0);

    /*
     * setSequence
     * Sets the animation sequence to render.
    */
    void setSequence(const AnimSequence * paniseq);
    void setSequence(fmt::animseqid_t seqid);

    //For layering items over the preview
    void addGraphicsItem(QGraphicsItem * pitem);
    void removeGraphicsItem(QGraphicsItem * pitem);
    int getNbGraphicsItems()const;
    QGraphicsItem * getGraphicsItems(int idx);
    const QGraphicsItem * getGraphicsItems(int idx)const{return const_cast<SpriteScene*>(this)->getGraphicsItems(idx);}

    //Gets the current index of the frame in the animation being rendered!
    int getCurrentFrame()const;

    /*
     * Returns the duration in ticks of the currently loaded sequence
    */
    unsigned int getSequenceLengthInTicks()const;

    /*
     * Returns how many ticks of the duration of the sequence have elapsed
    */
    unsigned int getTimeElapsed()const;
//Interactions

    /*
     * Changes the currently displayed animation sequence to the one specified
    */
    void SetAnimationSequence(animseqid_t seq);

    /*
     * getAnimationLength
     * Returns the number of frames in the currently displayed animation sequence
     * 0 if empty.
    */
    size_t getAnimationLength()const;

    QVector<QImage> DumpSequence()const;

private:
    void ConnectSpriteSignals();
    void DisconnectSpriteSignals();
    void SetupSceneBackground();

signals:
    //Signals to external controls from the animated sprite
    void framechanged(int /*curframe*/, QRectF /*area*/);
    void rangechanged(int /*begining*/,int /*end*/);

    //Load handling
    void loadingBegin();
    void loadingEnd();

    //Signals sent to the animated sprite object
    void setCurFrm(int /*newcurframe*/);
    void tick(unsigned int /*curtick*/);

public slots:
    //Animation Slots
    void beginAnimationPlayback();      //Creates the timer and start playback
    void endAnimationPlayback();        //Stops animation playback, clears the timer
    void TimerTick();                   //Called by the timer every tick
    void setShouldLoop(bool state);     //Set whether the scene should keep ticking after completing a loop of the animation
    void loopComplete();                //When the animsprite completed its animation loop, it calls this.
    void setCurrentFrame(int frmid);    //Force set the frame the animsprite is currently displaying

    //Scene Loading
    void reloadAnim();
    void loadAnimation();

    //Scene change callbacks
    void OnSpriteDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()); //Called when the contained sprite or animation has had its data changed!
    void OnAnimDataChaged();
    void OnFrameChanged(int curfrm, QRectF area); //Called by the animated sprite object to tell us its displaying a new frame, and we gotta redraw the scene!
    //void OnRangeChanged( int min, int max );

private:
    static const int TICK_RATE = 24; //Default animation tickrate
    bool                   m_shouldLoop{false};         //Whether the animation should loop or anot
    QScopedPointer<QTimer> m_timer;                     //Timer for running the animation
    QGraphicsScene         m_animScene;                 //Graphic scene used to render the animation sequence
    const Sprite *         m_spr{nullptr};              //Current sprite object
    QScopedPointer<AnimatedSpriteItem> m_animsprite;    //Temporary object containing the currently displayed animated sequence
    unsigned int           m_ticks{0};                  //Animation ticks counter
    unsigned int           m_tickselapsed{0};           //The amount of ticks that are elapsed since the start of the animation sequence. Aka, the time between the first frame and the current frame plus the time spent on the current frame in ticks.
    fmt::animseqid_t        m_seqid{fmt::animseqid_t()}; //Currently displayed animation sequence id
    SpriteRenderer           m_renderer;                //Renderer for sprites

    //Concurent processing
    QFutureWatcher<void> m_seqloadwatch;                //Watcher for obtaining when the animation sequence loading is done
    QFuture<void>        m_seqloadupdate;               //Future that will contain the resulting loaded sequence after the loading is done!
    QMutex               m_mtxScene;                    //Mutex for locking modifications to the QGraphicScene

    QList<QGraphicsItem*> m_extraElements;     //Graphics item layered over the scene
};

#endif // SPRITE_SCENE_HPP
