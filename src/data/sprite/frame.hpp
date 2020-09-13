#ifndef FRAME_HPP
#define FRAME_HPP
#include <src/data/treeelem.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>

enum struct eFramesColumnsType : int
{
    Preview     = 0,
    ImgID,
    TileNum,
    PaletteID,
    Unk0,
    Offset,
    Flip,
    RotNScaling,
    Mosaic,
    Mode,
    Priority,
    HeaderNBColumns,


    direct_XOffset = HeaderNBColumns, //Extra column for accessing directly the x offset
    direct_YOffset, //Extra column for accessing directly the y offset
    direct_VFlip,   //Extra column for accessing directly the vflip boolean
    direct_HFlip,   //Extra column for accessing directly the hflip boolean
    NBColumns,
    INVALID,
};

extern const char *                 ElemName_Frame;
extern const size_t                 FramesHeaderNBColumns;
extern const std::vector<QString>   FramesHeaderColumnNames;

//*******************************************************************
//  MFrame
//*******************************************************************
// Represents an assembly of images.
class MFrameDelegate;
class MFrame : public BaseTreeContainerChild<&ElemName_Frame, MFramePart>
{
    friend class MFrameDelegate;
    //Dynamic property for edit controls, so we can keep track of which part they edit!
    static const char * PropPartID;
    typedef BaseTreeContainerChild<&ElemName_Frame,MFramePart> paren_t;

public:
    MFrame(TreeElement * parentNode);
    MFrame(const MFrame & cp);
    MFrame(MFrame      && mv);
    ~MFrame();
    MFrame &  operator=(const MFrame & cp);
    MFrame &  operator=(MFrame      && mv);

    void clone(const TreeElement *other);

    bool operator==( const MFrame & other)const;
    bool operator!=( const MFrame & other)const;

    void importFrame(const fmt::ImageDB::frm_t & frms);
    fmt::ImageDB::frm_t exportFrame()const;

    fmt::step_t         *getPart(int id);
    const fmt::step_t   *getPart(int id)const;
    inline int          getNbParts()const {return nodeChildCount();}


    QPixmap AssembleFrameToPixmap(int xoffset, int yoffset, QRect cropto, QRect * out_area = nullptr) const;
    //cropto: A rectangle whose width and height will be used to crop the assembled frame. Mainly useful to get even-sized frames.
    //        A default QRect is ignored and the image will be cropped to our discretion.
    QImage  AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect *out_area = nullptr, bool makebgtransparent = true)const;
    QRect   calcFrameBounds()const;

    //For now UID is index!
    inline int getFrameUID()const {return nodeIndex();}
    QVariant frameDataCondensed(int role)const;
    QVariant DataForAPart(int row, int column, int role) const;

    MFrameDelegate & itemDelegate();

public:
    Sprite * parentSprite()override;
    //Those can be re-implemented!
    virtual int         nodeColumnCount() const override;
    virtual QVariant    nodeData(int column, int role) const override;

    //data method of the model and childs!!!
    //We need this so that we can virtually display entries for each steps of the frame!
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //setData method of the model and childs!!!
    virtual bool setData(const QModelIndex  &index,
                         const QVariant     &value,
                         int                role = Qt::EditRole)override;

    virtual int columnCount(const QModelIndex &parentNode) const;
    virtual int rowCount(const QModelIndex &parent)const override;

private:
    QScopedPointer<MFrameDelegate> m_pdelegate;
};

#endif // FRAME_HPP
