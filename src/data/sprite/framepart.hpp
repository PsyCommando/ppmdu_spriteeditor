#ifndef FRAMEPART_HPP
#define FRAMEPART_HPP
#include <src/data/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>


extern const char * ElemName_FramePart;
//*******************************************************************
//  MFramePart
//*******************************************************************
// Represents data on a particular image referred in a MFrame.
class MFramePart : public BaseTreeTerminalChild<&ElemName_FramePart>
{
    typedef BaseTreeTerminalChild<&ElemName_FramePart> partparent_t;
public:
    MFramePart(TreeElement * parent);
    MFramePart(TreeElement * parent, const fmt::step_t & part);
    virtual ~MFramePart();

    bool operator==( const MFramePart & other)const;
    bool operator!=( const MFramePart & other)const;

    void clone(const TreeElement *other)
    {
        const MFramePart * ptr = static_cast<const MFramePart*>(other);
        if(!ptr)
            throw std::runtime_error("MFramePart::clone(): other is not a MFramePart!");
        (*this) = *ptr;
    }

    Sprite          * parentSprite();
    const Sprite    * parentSprite()const;

public:
    virtual int             nodeColumnCount() const;
    virtual Qt::ItemFlags   nodeFlags(int column = 0)const override;
    virtual QVariant        nodeData(int column, int role) const override;


    /*
     *  drawPart
     *      Draws an image from the part's data.
     *      Optimized for displaying the part, return an image in the ARGB32_premultiplied format.
     *      If the part is a -1 frame, returns a null image!
    */
    QImage drawPart(bool transparencyenabled = false)const;

public:
    void        importPart(const fmt::step_t & part);
    fmt::step_t exportPart()const;

    fmt::step_t         & getPartData();
    const fmt::step_t   & getPartData()const;

private:
    static QSize calcTextSize( const QString & str);

    QVariant dataImgPreview     (int role) const;
    QVariant dataImgId          (int role)const;
    QVariant dataUnk0           (int role)const;
    QVariant dataOffset         (int role)const;
    QVariant dataDirectXOffset  (int role)const;
    QVariant dataDirectYOffset  (int role)const;
    QVariant dataFlip           (int role)const;
    QVariant dataDirectVFlip    (int role)const;
    QVariant dataDirectHFlip    (int role)const;
    QVariant dataRotNScaling    (int role)const;
    QVariant dataPaletteID      (int role)const;
    QVariant dataPriority       (int role)const;
    QVariant dataTileNum        (int role)const;
    QVariant dataMosaic         (int role)const;
    QVariant dataMode           (int role)const;

    //Transform the given image according to the parameters stored in this class!
    void applyTransforms(QImage & srcimg)const;

private:
    fmt::step_t  m_data;
    fmt::frmid_t m_imgUID;
};

#endif // FRAMEPART_HPP
