#ifndef ANIMATEDSPRITEITEM_HPP
#define ANIMATEDSPRITEITEM_HPP

#include <QGraphicsItem>
#include <QObject>
#include <QWidget>
#include <src/data/sprite/sprite.hpp>

/*
 * Struct to store a cached animation frame.
 * Contains the actual final image to be rendered, and various position data
*/
struct cachedframe
{
    QPixmap img;            //Image data of the frame
    QRect   area;           //Frame bounding box
    int     duration = 0;   //Display duration of the frame in the sequence
    int     shadowx = 0;    //Shadow blob X offset
    int     shadowy = 0;    //Shadow blob Y offset
    int     offsetx = 0;    //Sprite center X offset
    int     offsety = 0;    //Sprite center Y offset
};

/*
 * AnimatedSpriteItem
 *  Update the current frame from the exterior to get the
 *  sprite to change its currently displayed frame.
*/
class AnimatedSpriteItem : public QGraphicsObject
{
    Q_OBJECT
public:
    using animseqid_t = fmt::animseqid_t;

    AnimatedSpriteItem();
    AnimatedSpriteItem(QVector<cachedframe> && frames, QVector<QColor> && palette);
    virtual ~AnimatedSpriteItem();

    /*
     * Reset the sprite state
    */
    void Reset();

    /*
     * Sets the cached frames and their palette
    */
    void setCachedFrames(QVector<cachedframe> && frames, QVector<QColor> && palette);

    /*
     * Dumps all the rendered frames of the animation to a vector of images.
    */
    QVector<QImage> DumpFrames()const;

    //Get the total amount of frames this item's animation has
    inline size_t getNbFrames()const{return m_cachedframes.size();}

    //Get the frame this item is currently displaying
    inline int getCurrentFrame()const{return m_curfrm;}

    //Get the total duration it takes for the animation of this item to play
    inline unsigned int getDuration()const{return m_cachedDuration;}

    //Get the amount of ticks elapsed between the currently displayed frame and the start of the animation
    inline unsigned int getTimeElapsed()const{return m_tickselapsed;}

    //Get the area within which the frames for this animation move/appear. Useful for resizing the viewport and having the whole thing fit in.
    inline QRect getTravelRange()const {return m_frameTraverse;}

// QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    /*
     * Change the frame currently set to paint, depending on the current time in ticks
     * Called in tick
    */
    void UpdateFrame();

private:
    QVector<cachedframe>    m_cachedframes;     //Cached animation frames
    QVector<QColor>         m_cachedpal;        //The cached color palette for the animation sequence
    QRect                   m_biggestFrame;     //The largest frame of all the cached frames. Used to calculate the center and etc
    QRect                   m_frameTraverse;    //The area where the frames in this animation will move into
    int                     m_curfrm{0};        //The frame the sprite is currently set to paint
    unsigned int            m_ticksnextfrm{0};  //Amount of ticks to wait for before changing the frame again!
    unsigned int            m_cachedDuration{0};//Amount of ticks the animation lasts for
    unsigned int            m_tickselapsed{0};  //Ticks elapsed from the first frame of the animation sequence, to the end of the animation sequence. Used to track playback

signals:
    void frame_changed(int /*frmid*/, QRectF /*boundingbox*/); //Emited when the sprite's current painted frame changed
    void playback_complete();   //Emited when the sprite has played through all its frames

public slots:
    void tick(unsigned int curticks);   //Called every ticks, the sprite decides if it needs to do anything
    void setCurFrame(int frmid);        //Force set the current frame to be drawn on the next paint call + reset the frame duration!
};

#endif // ANIMATEDSPRITEITEM_HPP
