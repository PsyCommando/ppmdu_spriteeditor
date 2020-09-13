#ifndef IMAGESCONTAINER_HPP
#define IMAGESCONTAINER_HPP
#include <src/data/treeelem.hpp>
#include <src/data/sprite/image.hpp>

extern const char * ElemName_Images;
//*******************************************************************
//  ImageContainer
//*******************************************************************
//Contains all the loaded images, and displays them in the appropriate view
class ImageContainer : public BaseTreeContainerChild<&ElemName_Images, Image>
{
public:
    ImageContainer(TreeElement *parent)
        :BaseTreeContainerChild(parent)
    {
        setNodeDataTy(eTreeElemDataType::images);
    }

    ImageContainer(const ImageContainer &cp)
        :BaseTreeContainerChild(cp)
    {}

    ImageContainer(ImageContainer &&mv)
        :BaseTreeContainerChild(mv)
    {}

    ~ImageContainer()
    {
        qDebug("ImageContainer::~ImageContainer()\n");
    }

    ImageContainer &operator=(const ImageContainer &cp)
    {
        BaseTreeContainerChild::operator=(cp);
        return *this;
    }

    ImageContainer &operator=(ImageContainer &&mv)
    {
        BaseTreeContainerChild::operator=(mv);
        return *this;
    }

    void clone(const TreeElement *other);

//
//Image container stuff
//
public:
    static const QString & ComboBoxStyleSheet();
    //lets make things shorter
    using imgtbl_t = fmt::ImageDB::imgtbl_t;
    using frmtbl_t = fmt::ImageDB::frmtbl_t;

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

    //ImageSelectorModel * getImageSelectModel() {return m_pimgselmodel.data();}

//
//BaseContainerChild overrides
//
public:
    QVariant nodeData(int column, int role) const override;
    Sprite * parentSprite();
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    //Whether the node should be movable
    bool        nodeIsMutable()const override           {return false;}
    virtual int nodeColumnCount() const override        {return static_cast<int>(Image::eColumnType::NbColumns);}

private:
    //QScopedPointer<ImageSelectorModel> m_pimgselmodel;
};

#endif // IMAGESCONTAINER_HPP
