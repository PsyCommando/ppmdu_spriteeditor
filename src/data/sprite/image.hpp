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
    Image(TreeNode * parent);
    Image(Image && mv);
    Image(const Image & cp);
    Image & operator=(Image && mv);
    Image & operator=(const Image & cp);
    ~Image();

    inline bool operator==( const Image & other)const  {return this == &other;}
    inline bool operator!=( const Image & other)const  {return !operator==(other);}

    //
    //  Import/Export
    //
    //Import an image that doesn't fit the expected format, mostly for research
    void                importBrokenImage(const fmt::ImageDB::img_t & img);

    void                importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h);
    void                importImage8bpp(const fmt::ImageDB::img_t & img, int w, int h);
    fmt::ImageDB::img_t exportImage4bpp(int & w, int & h)const;
    fmt::ImageDB::img_t exportImage8bpp(int & w, int & h)const;

    //
    //  Preview
    //
    //Replace the current cached QImage with a fresh one made from the raw data
    void generateCachedImage(int w, int h);

    //Generate a displayable QImage using the specified palette and the cached image data.
    QImage makeImage(const QVector<QRgb> & palette)const;
    QPixmap makePixmap(const QVector<QRgb> & palette)const;

    //Returns the contained image as-is with no palette
    const QImage & getImage()const {return m_img;}

    //Return bounding rectangle for the cached preview image
    inline QSize getImageSize()const        {return m_img.size();}

    //Returns a short description for the image to be displayed in the model
    QString getImageDescription()const;

    //
    //Properties
    //
    //Returns the original depth of the image as it was in the file
    inline fmt::eColorDepths getImageOriginalDepth()const {return m_depth;}

    //Returns the raw size of the image's data
    int getByteSize()const;

    //Returns the raw bytes of the image
    inline const std::vector<uint8_t> & getRawPixels()const{return m_rawPixels;}

    //Returns the size of the image in tiles
    int getTileSize()const;
    int getBlockLen()const;

    //Access a particular block in the raw image data
    std::vector<uint8_t>                    getBlock   (int blocknum)const;
    std::vector<uint8_t>::const_iterator    getBlockBeg(int blocknum)const;
    std::vector<uint8_t>::const_iterator    getBlockEnd(int blocknum)const;

    //Returns the raw data for a  specific tile from the image
    std::vector<uint8_t>                    getTile   (int tnum)const;
    std::vector<uint8_t>::const_iterator    getTileBeg(int tnum)const;
    std::vector<uint8_t>::const_iterator    getTileEnd(int tnum)const;

    //Returns the session unique id for this image
    inline id_t getImageUID()const {return getID();}

    //Unk values
    uint32_t getUnk2()const         {return m_unk2;}
    void     setUnk2(uint32_t val)  {m_unk2 = val;}
    uint16_t getUnk14()const        {return m_unk14;}
    void     setUnk14(uint16_t val) {m_unk14 = val;}

// TreeNode interface
public:
    TreeNode *          clone() const override;
    eTreeElemDataType   nodeDataTy() const override;
    const QString &     nodeDataTypeName() const override;

protected:
    QImage                  m_img;                                  //8bpp image, can be replaced by users during use of the program
    std::vector<uint8_t>    m_rawPixels;                            //Pixels in their original tiled format, 4bbp or 8bpp, uncompressed!
    fmt::eColorDepths       m_depth {fmt::eColorDepths::Invalid};   //Original image bit depth in bpp, either 4 or 8
    uint16_t                m_unk2  {0};                            //unknown field
    uint16_t                m_unk14 {0};                            //unknown field
    bool                    m_broken{false};                        //Whether the image is "broken" or not. If broken its unusable, but we keep the data anyways for research
};
#endif // IMAGE_HPP
