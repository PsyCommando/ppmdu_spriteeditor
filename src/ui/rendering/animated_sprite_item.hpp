#ifndef ANIMATEDSPRITEITEM_HPP
#define ANIMATEDSPRITEITEM_HPP

#include <QGraphicsItem>
#include <QObject>
#include <QWidget>
#include <src/sprite.hpp>

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
    using animseqid_t = fmt::AnimDB::animseqid_t;

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
     *
    */
    QVector<QImage> DumpFrames()const;

    inline size_t getNbFrames()const{return m_cachedframes.size();}

    inline int getCurrentFrame()const{return m_curfrm;}

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
    int                     m_curfrm{0};        //The frame the sprite is currently set to paint
    unsigned int            m_ticksnextfrm{0};  //Amount of ticks to wait for before changing the frame again!

signals:
    void frame_changed(int /*frmid*/, QRectF /*boundingbox*/); //Emited when the sprite's current painted frame changed
    void playback_complete();   //Emited when the sprite has played through all its frames

public slots:
    void tick(unsigned int curticks);   //Called every ticks, the sprite decides if it needs to do anything
    void setCurFrame(int frmid);        //Force set the current frame to be drawn on the next paint call + reset the frame duration!
};

#endif // ANIMATEDSPRITEITEM_HPP
