#ifndef IMAGE_HPP
#define IMAGE_HPP
#include <QVector>
#include <QRgb>
#include <QImage>
#include <src/data/treenodeterminal.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

extern const QString ElemName_Image;
//*******************************************************************
//  Image
//*******************************************************************
//
class Image : public TreeNodeTerminal, public utils::BaseSequentialIDGen<TreeNodeTerminal, unsigned int>
{
    friend class ImageListModel;
public:
    enum struct eColumnType : int
    {
        Preview = 0,
        UID,
        Depth,
        Resolution,
        NbColumns [[maybe_unused]],

        direct_Unk2 [[maybe_unused]],    //Extra columns that don't count in the NbColumns
        direct_Unk14 [[maybe_unused]],
    };

public:
    Image(TreeNode * parent);
    Image(Image && mv);
    Image(const Image & cp);
    Image & operator=(Image && mv);
    Image & operator=(const Image & cp);
    ~Image();

    inline bool operator==( const Image & other)const  {return this == &other;}
    inline bool operator!=( const Image & other)const  {return !operator==(other);}

    //
    //Image stuff
    //
    void                importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h, bool isTiled);
    fmt::ImageDB::img_t exportImage4bpp(int & w, int & h, bool tiled)const;
    void                importImage8bpp(const fmt::ImageDB::img_t & img, int w, int h, bool isTiled);
    fmt::ImageDB::img_t exportImage8bpp(int & w, int & h, bool tiled)const;

    /*
        Generate a displayable QImage using the specified palette and the image data currently stored.
    */
    QImage makeImage(const QVector<QRgb> & palette)const;

    //Return bounding rectangle for the image contained
    inline QSize getImageSize()const        {return m_img.size();}
    inline int getImageOriginalDepth()const {return m_depth;}

    QString getImageDescription()const;

    //Reimplemented nodeData method specifically for displaying images in the image list table!
    //virtual QVariant imgData(int column, int role) const;
    //virtual bool     setImgData(int column, const QVariant &value, int role);
    //virtual QVariant imgDataCondensed(int role) const;

    //Reimplemented nodeColumnCount specifically for displaying images in the image list table!
    virtual int nbimgcolumns()const {return static_cast<int>(eColumnType::NbColumns);}

    //Returns the session unique id for this image
    inline id_t getImageUID()const {return getID();}

    //Unk values
    uint32_t getUnk2()const         {return m_unk2;}
    void     setUnk2(uint32_t val)  {m_unk2 = val;}
    uint16_t getUnk14()const        {return m_unk14;}
    void     setUnk14(uint16_t val) {m_unk14 = val;}

// TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;

protected:
    QImage                  m_img;
    std::vector<uint8_t>    m_raw;          //Need this because QImage doesn't own the raw img data buffer...
    int                     m_depth {0};    //Original image depth in bpp
    uint16_t                m_unk2  {0};
    uint16_t                m_unk14 {0};
};
#endif // IMAGE_HPP
