#ifndef SPRITE_H
#define SPRITE_H
#include <QObject>
#include <QByteArray>
#include <QStack>
#include <QHash>
#include <QPixmap>
#include <QRgb>
#include <QImage>
#include <QVector>
#include <QAbstractItemModel>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QDebug>
#include <QGraphicsScene>
#include <QPointer>

#include <cstdint>
#include <list>
#include <functional>

#include <src/data/treenode.hpp>
#include <src/utility/baseqtexception.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>
#include <src/ppmdu/utils/imgutils.hpp>

//Backend
#include <src/ppmdu/fmts/compression_handler.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

//Other parts of the sprite
#include <src/data/sprite/effectoffsetcontainer.hpp>
#include <src/data/sprite/imagescontainer.hpp>
#include <src/data/sprite/framescontainer.hpp>
#include <src/data/sprite/palettecontainer.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/animtable.hpp>


//Since we have our own compression settings that differs
// from those in the compression_handler.hpp file, put them here!
enum struct eCompressionFmtOptions : int
{
    PKDPX = 0,
    AT4PX,
    NONE,
};
extern const QStringList CompressionFmtOptions; //names for the above options
filetypes::eCompressionFormats  CompOptionToCompFmt( eCompressionFmtOptions opt );
eCompressionFmtOptions          CompFmtToCompOption( filetypes::eCompressionFormats fmt );

enum struct eSpriteColorModes : int
{
    _16Colors = 0,
    _256Colors,
    _Bitmap,
    INVALID [[maybe_unused]],
};
extern const QStringList SpriteColorModes;
extern const QString ElemName_Sprite;

//Exceptions
class ExBadSpriteData :public BaseException {public: using BaseException::BaseException;};

//*******************************************************************
//  Sprite
//*******************************************************************
class Sprite : public TreeNode
{
    friend class SpritePropertiesHandler;
    void _ctor();
public:
    typedef std::vector<uint8_t> rawdat_t;

    Sprite(TreeNode * parent);
    Sprite(TreeNode * parent, rawdat_t && raw);
    Sprite(const Sprite & cp);
    Sprite(Sprite       && mv);
    Sprite & operator=(const Sprite & cp);
    Sprite & operator=(Sprite       && mv);
    ~Sprite();
    TreeNode* clone()const override;

    inline bool operator==( const Sprite & other)const  {return this == &other;}
    inline bool operator!=( const Sprite & other)const  {return !operator==(other);}

public:
    TreeNode *  nodeChild(int row) override;
    int         nodeChildCount() const override;
    bool        nodeCanFetchMore()const override;
    void        nodeFetchMore()override;
//    bool        canFetchMore(const QModelIndex &parent) const override;
//    void        fetchMore(const QModelIndex &parent) override;
    eTreeElemDataType nodeDataTy()const override;
    const QString&  nodeDataTypeName()const override;
    int indexOfChild(const TreeNode * ptr)const override;
    bool nodeShouldAutoExpand()const override {return true;}
    bool nodeAllowFetchMore()const override {return true;}
    QString nodeDisplayName()const override;

protected:
    bool _insertChildrenNode(TreeNode *node, int destrow) override;
    bool _insertChildrenNodes(int row, int count) override;
    bool _insertChildrenNodes(const QList<TreeNode*> & nodes, int destrow = -1) override;

    bool _removeChildrenNode(TreeNode *node) override;
    bool _removeChildrenNodes(int row, int count)override;
    bool _removeChildrenNodes(const QList<TreeNode*> & nodes)override;

    bool _deleteChildrenNode(TreeNode *node) override;
    bool _deleteChildrenNodes(int row, int count)override;
    bool _deleteChildrenNodes(const QList<TreeNode*> & nodes)override;

    bool _moveChildrenNodes(int row, int count, int destrow, TreeNode* destnode)override;
    bool _moveChildrenNodes(QModelIndexList &indices, int destrow, QModelIndex destparent) override;
    bool _moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent) override;

public:

    //Returns whether the sprite can be parsed, or not if there is something wrong with the raw data
    bool canParse()const;

    //Parse the data from the internal raw container
    void ParseSpriteData();

    //Turns the data from the data structure to its native format into the m_raw container!
    void CommitSpriteData();

    inline bool wasParsed()const    {return m_bparsed;}
    inline bool hasImageData()const {return m_imgcnt.nodeHasChildren();}
    inline bool hasEfxOffsets()const{return type() == fmt::eSpriteType::Character;}
    inline bool hasAnimGrps()const  {return type() == fmt::eSpriteType::Character;}

    QPixmap & MakePreviewPalette();
    QPixmap MakePreviewPalette()const; //Ignores cached palette, and makes one from scratch
    const QPixmap & getCachedPreviewPalette()const;

    QPixmap & MakePreviewFrame(bool transparency = false);
    QPixmap MakePreviewFrame(bool transparency = false)const; //Ignores cached image, and makes one from scratch
    const QPixmap & getCachedPreviewFrame()const;

    const QVector<QRgb> & getPalette()const     { return m_palcnt.getPalette(); }
    QVector<QRgb>       & getPalette()          { return m_palcnt.getPalette(); }
//    const PaletteModel * getPaletteModel()const { return m_palcnt.getModel(); }
//    PaletteModel       * getPaletteModel()      { return m_palcnt.getModel(); }

    inline void setPalette(const QVector<QRgb> & pal)   {m_palcnt.setPalette(pal);}

    inline AnimSequences        & getAnimSequences()                            {return m_seqcnt;}
    inline const AnimSequences  & getAnimSequences()const                       {return m_seqcnt;}
    inline AnimSequence         * getAnimSequence(fmt::AnimDB::animseqid_t id)  {return m_seqcnt.getSequenceByID(id);}
    inline const AnimSequence   * getAnimSequence(fmt::AnimDB::animseqid_t id)const  {return m_seqcnt.getSequenceByID(id);}

    inline AnimTable                & getAnimTable()                    {return m_anmtbl;}
    inline const AnimTable          & getAnimTable()const               {return m_anmtbl;}
    inline ImageContainer           & getImages()                       {return m_imgcnt;}
    inline const ImageContainer     & getImages()const                  {return m_imgcnt;}
    inline MFrame                   * getFrame( fmt::frmid_t id )       {return m_frmcnt.getFrame(id);}
    inline const MFrame             * getFrame( fmt::frmid_t id )const  {return m_frmcnt.getFrame(id);}
    inline FramesContainer          & getFrames()                       {return m_frmcnt;}
    inline const FramesContainer    & getFrames()const                  {return m_frmcnt;}
    Image                           * getImage(fmt::frmid_t idx);
    const Image                     * getImage(fmt::frmid_t idx)const;

    //Return binary version of the sprite
    inline rawdat_t             & getRawData()      {return m_raw;}
    inline const rawdat_t       & getRawData()const {return m_raw;}

    inline fmt::eSpriteType type()const    {return m_sprhndl.getSpriteType();}
    void                    convertSpriteToType(fmt::eSpriteType newty);

    //SPRITE PROPERTIES
//    SpritePropertiesHandler         * propHandler();
//    const SpritePropertiesHandler   * propHandler()const;

//    SpritePropertiesModel           * model();
//    const SpritePropertiesModel     * model()const;

//    SpriteOverviewModel             * overviewModel();
//    const SpriteOverviewModel       * overviewModel()const;

    inline bool is256Colors()const
    {
        return m_sprhndl.getImageFmtInfo().is256Colors();
    }
    inline void setIs256Colors( bool state )
    {
        m_sprhndl.getImageFmtInfo().setIs256Colors(state);
    }

    //0 == 2D mapping, 1 == 1D mapping
    inline bool     isTiled()const              {return unk13() == 0;}
    inline void     SetIsTiled(bool state)      {return unk13(state);}

    void                                    setTargetCompression(filetypes::eCompressionFormats fmt){ m_targetgompression = fmt;}
    inline filetypes::eCompressionFormats   getTargetCompression()const                             {return m_targetgompression;}

    inline uint16_t unk6()const         {return m_sprhndl.getAnimFmtInfo().unk6;}
    inline void     unk6(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk6 = v;}
    inline uint16_t unk7()const         {return m_sprhndl.getAnimFmtInfo().unk7;}
    inline void     unk7(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk7 = v;}
    inline uint16_t unk8()const         {return m_sprhndl.getAnimFmtInfo().unk8;}
    inline void     unk8(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk8 = v;}
    inline uint16_t unk9()const         {return m_sprhndl.getAnimFmtInfo().unk9;}
    inline void     unk9(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk9 = v;}
    inline uint16_t unk10()const        {return m_sprhndl.getAnimFmtInfo().unk10;}
    inline void     unk10(uint16_t v)   {m_sprhndl.getAnimFmtInfo().unk10 = v;}
    inline uint16_t unk11()const        {return m_sprhndl.getImageFmtInfo().unk11;}
    inline void     unk11(uint16_t v)   {m_sprhndl.getImageFmtInfo().unk11 = v;}
    inline uint16_t unk12()const        {return m_sprhndl.getUnk12();}
    inline void     unk12(uint16_t v)   {m_sprhndl.getUnk12() = v;}
    inline uint16_t unk13()const        {return m_sprhndl.getImageFmtInfo().unk13;}
    inline void     unk13(uint16_t v)   {m_sprhndl.getImageFmtInfo().unk13 = v;}

private:
    bool IsRawDataCompressed(filetypes::eCompressionFormats * outfmt = nullptr)const;
    void DecompressRawData();
    void CompressRawData(filetypes::eCompressionFormats cpfmt);

    TreeNode       * ElemPtr(int idx);
    TreeNode       * ElemPtrNoEfx(int idx);
    const TreeNode * ElemPtr(int idx)const;
    const TreeNode * ElemPtrNoEfx(int idx)const;
    int                 nbChildCat()const;

private:
    //Raw data buffer & handler
    std::vector<uint8_t>    m_raw;
    fmt::WA_SpriteHandler   m_sprhndl;

    //Converted data
    EffectOffsetContainer   m_efxcnt;
    PaletteContainer        m_palcnt;
    ImageContainer          m_imgcnt;
    FramesContainer         m_frmcnt;
    AnimSequences           m_seqcnt;
    AnimTable               m_anmtbl;
//    QScopedPointer<SpritePropertiesHandler> m_propshndlr;

    //Status / statistics
    bool                                    m_bparsed;          //Whether the sprite's raw has been parsed to be displayed yet or not!
    bool                                    m_bhasimagedata;    //Whether the sprite can be displayed or not!
    filetypes::eCompressionFormats          m_targetgompression;
//    QScopedPointer<SpriteOverviewModel>     m_overmodel;

    //UI data
    QPixmap m_previewImg;       //Cached image preview
    QPixmap m_previewPal;       //Cached palette preview

};

#endif // SPRITE_H
