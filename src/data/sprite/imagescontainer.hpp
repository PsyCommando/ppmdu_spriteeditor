#ifndef IMAGESCONTAINER_HPP
#define IMAGESCONTAINER_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/sprite/image.hpp>

extern const QString ElemName_Images;
//*******************************************************************
//  ImageContainer
//*******************************************************************
//Contains all the loaded images, also act as a category node for images
class ImageContainer : public TreeNodeWithChilds<Image>
{
public:
//    ImageContainer(Sprite* sprite);
//    ImageContainer(const ImageContainer & cp);
//    ImageContainer(ImageContainer && mv);
//    ImageContainer &operator=(const ImageContainer & cp);
//    ImageContainer &operator=(ImageContainer && mv);
    ImageContainer(TreeNode* sprite)
        :TreeNodeWithChilds(sprite)
    {}

    ImageContainer(const ImageContainer & cp)
        :TreeNodeWithChilds<Image>(cp)
    {}

    ImageContainer(ImageContainer && mv)
        :TreeNodeWithChilds<Image>(mv)
    {}

    ~ImageContainer();

    ImageContainer &operator=(const ImageContainer &cp)
    {
        TreeNodeWithChilds<Image>::operator=(cp);
        return *this;
    }

    ImageContainer &operator=(ImageContainer &&mv)
    {
        TreeNodeWithChilds<Image>::operator=(mv);
        return *this;
    }

    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    bool nodeShowChildrenOnTreeView()const override {return false;}
//
//Image container stuff
//
public:
    static const QString & ComboBoxStyleSheet();
    //lets make things shorter
    using imgtbl_t = fmt::ImageDB::imgtbl_t;
    using frmtbl_t = fmt::ImageDB::frmtbl_t;

    Image * appendNewImage();

    /*
        importImages
            Imports a table of raw images into the container, and convert them to a displayable format!
    */
    void importImages(const imgtbl_t & imgs,
                      const frmtbl_t & frms);

    inline void importImages8bpp(const imgtbl_t & imgs, const frmtbl_t & frms) {importImages(imgs, frms);}
    inline void importImages4bpp(const imgtbl_t & imgs, const frmtbl_t & frms) {importImages(imgs, frms);}

    /*
        exportImages
            Exports the child images nodes back into their raw format!
    */
    imgtbl_t exportImages();
    imgtbl_t exportImages4bpp();
    imgtbl_t exportImages8bpp();

    /*
        getImage
            Helper method that casts the childs directly to the appropriate type.
    */
    inline Image        * getImage(fmt::frmid_t id)     { return static_cast<Image*>(nodeChild(id)); }
    inline const Image  * getImage(fmt::frmid_t id)const { return static_cast<Image*>(const_cast<ImageContainer*>(this)->nodeChild(id)); }

    /*
     * getTileData
     *      Allow access to all the image data as if it was in tile memory
    */
    QVector<uint8_t> getTileData(int id, int len)const;

    //Same as above, but starts to lookup at the specified image
    QVector<uint8_t> getTileDataFromImage(int imgidx, int id, int len)const;

    //Whether the node should be movable
    bool    nodeIsMutable()const override {return false;}
    QString nodeDisplayName() const override;


    void DumpAllImages(const QString & dirpath, const QVector<QRgb> & palette)const;
};

#endif // IMAGESCONTAINER_HPP
