#ifndef FRAMEPART_HPP
#define FRAMEPART_HPP
#include <src/data/treenodeterminal.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>

enum struct eFramePartColumnsType : int
{
    Preview = 0,                        //Preview image column
    ImgID,                              //Image index in the image data
    ImgSz,                              //Image resolution
    BlockRange,                         //Block range in use for the frame part
    PaletteID,                          //Palette in use by the part
    XOffset,                            //X offset of the part in screen coordinates
    YOffset,                            //Y offset of the part in screen coordinates
    VFlip,                              //Is the part vertically flipped
    HFlip,                              //Is the part horizontally flipped
    Priority,                           //Draw priority of the part in relation to the background
    LastColumnsBasicMode = Priority,    //So we don't count the nb columns

    //Advanced Controls
    ImgIsRef,                           //Convenience column meant to display if the part is a reference to something else. Hidden by default
    BlockNum,                           //Starting block number for the frame part
    Mosaic,                             //Whether mosaic mode is enabled for this part
    Mode,                               //Mode for displaying this part

    //Rotation and scaling
    RnS,                                //Whether rotation and scaling is enabled or not
    RnSParam,                           //Parameter for rotation and scaling
    RnSCanvasRot,                       //Whether rotations in rns mode are done to the whole canvas or to the part only. (AKA "double size flag")

    //Research stuff
    Unk0,

    //Always last
    NBColumns,
};

extern const std::map<eFramePartColumnsType, QString>   FramePartHeaderColumnNames;
extern const QString                ElemName_FramePart;
extern const QStringList            FRAME_PART_PRIORITY_NAMES;
extern const QStringList            FRAME_PART_MODE_NAMES;

//*******************************************************************
//  MFramePart
//*******************************************************************
// Represents data on a particular image referred in a MFrame.
class MFrame;
class MFramePart : public TreeNodeTerminal, public utils::BaseSequentialIDGen<MFramePart>
{
    friend class MFramePartModel;
    typedef TreeNodeTerminal partparent_t;
public:
    MFramePart(TreeNode * mframe);
    MFramePart(TreeNode * mframe, const fmt::step_t & part);
    MFramePart(const MFramePart& cp);
    MFramePart(MFramePart&& mv);
    virtual ~MFramePart();

    MFramePart &operator=(const MFramePart& cp);
    MFramePart& operator=(MFramePart&& mv);

    bool operator==( const MFramePart & other)const;
    bool operator!=( const MFramePart & other)const;

    TreeNode * clone()const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;

    virtual Qt::ItemFlags nodeFlags(int column = 0)const override;

    /*
     *  drawPart
     *      Draws an image from the part's data.
     *      Optimized for displaying the part, return an image in the ARGB32_premultiplied format.
     *      If the part is a -1 frame, returns a null image!
    */
    QImage drawPart(const Sprite * spr, bool transparencyenabled = false)const;

    QPixmap drawPartToPixmap(const Sprite * spr, bool transparencyenabled = false)const;

public:
    void        importPart(const fmt::step_t & part);
    fmt::step_t exportPart()const;

    fmt::step_t         & getPartData();
    const fmt::step_t   & getPartData()const;

    //Whether the part is a reference on another part, AKA a -1 frame
    bool isPartReference()const;
    void setIsReference(bool isref);

    //Returns the length in tiles this part is
    uint16_t getBlockLen()const;

    //Returns the right sub-palette from the src that matches the palette number of the framepart
    QVector<QRgb> getPartPalette(const QVector<QRgb> & src)const;

    //Wrapper methods
public:
    inline bool     isColorPal256()const            {return m_data.isColorPal256();}
    inline bool     isMosaicOn()const               {return m_data.isMosaicOn();}
    inline bool     isDisabled()const               {return m_data.isDisabled();}
    inline bool     isRnSRotCanvas()const           {return m_data.isRnSRotCanvas();}
    //inline bool     isDoubleSize()const             {return m_data.isDoubleSize();} //Misnomer from doc, actually rotates the "canvas" with the sprite instead of rotating the sprite in the "canvas"
    inline bool     isRotAndScalingOn()const        {return m_data.isRotAndScalingOn();}
    inline uint16_t getYOffset()const               {return m_data.getYOffset();}
    inline uint16_t getYWrappedOffset()const        {return (getYOffset() < 128)? getYOffset() + 255 : getYOffset();} //simulate wrap-around past 256 Y

    //Before checking VFlip and HFlip, make sure RnS isn't on!!!
    inline bool                     isVFlip()const          {return m_data.isVFlip();}
    inline bool                     isHFlip()const          {return m_data.isHFlip();}
    inline bool                     islast()const           {return m_data.islast();}
    inline fmt::step_t::eObjMode    getObjMode()const       {return m_data.getObjMode();}
    inline uint8_t                  getRnSParam()const      {return m_data.getRnSParam();}
    inline uint16_t                 getXOffset()const       {return m_data.getXOffset();}

    inline uint8_t          getPalNb()const                 {return m_data.getPalNb();}
    inline uint8_t          getPriority()const              {return m_data.getPriority();}
    inline uint16_t         getBlockNum()const              {return m_data.getBlockNum();}
    inline fmt::eFrameRes   getResolutionType()const        {return m_data.getResolutionType();}
    inline fmt::frmid_t     getFrameIndex()const            {return m_data.getFrameIndex();}
    inline std::pair<uint16_t,uint16_t> GetResolution()const {return m_data.GetResolution();}

    inline void setColorPal256      (bool bis256)           {m_data.setColorPal256(bis256);}
    inline void setMosaicOn         (bool bon)              {m_data.setMosaicOn(bon);}
    inline void setObjMode          (fmt::step_t::eObjMode mode){m_data.setObjMode(mode);}
    inline void setDisabled         (bool bon)              {m_data.setDisabled(bon);}
    inline void setRnSRotCanvas     (bool bon)              {m_data.setRnSCanvasRot(bon);}
    inline void setRotAndScaling    (bool bon)              {m_data.setRotAndScaling(bon);}
    inline void setYOffset          (uint16_t y)            {m_data.setYOffset(y);}

    inline void setVFlip            (bool bon)              {m_data.setVFlip(bon);}
    inline void setHFlip            (bool bon)              {m_data.setHFlip(bon);}
    inline void setLast             (bool blast)            {m_data.setLast(blast);}
    inline void setRnSParam         (uint8_t param)         {m_data.setRnSParam(param);}
    inline void setXOffset          (uint16_t x)            {m_data.setXOffset(x);}

    inline void setPalNb            (uint8_t palnb)         {m_data.setPalNb(palnb);}
    inline void setPriority         (uint8_t prio)          {m_data.setPriority(prio);}
    inline void setBlockNum          (uint16_t tnum)        {m_data.setBlockNum(tnum);}
    inline void setFrameIndex       (fmt::frmid_t id)       {m_data.setFrameIndex(id);}
    inline void setResolutionType   (fmt::eFrameRes res)    {m_data.setResolutionType(res);}
private:
    //Transform the given image according to the parameters stored in this class!
    void applyTransforms(QImage & srcimg)const;

    //Obtains the image data depending on the source used by the part, with the part's palette applied
    QImage getSrcImageData(const Sprite * spr)const;

private:
    fmt::step_t  m_data;
};

#endif // FRAMEPART_HPP
