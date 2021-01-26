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
#include <src/data/sprite/animgroups_container.hpp>


//Since we have our own compression settings that differs
// from those in the compression_handler.hpp file, put them here!
enum struct eCompressionFmtOptions : int
{
    PKDPX = 0,
    AT4PX,
    AT4PN,
    NONE,

    NbOptions,
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
    TreeNode *          nodeChild(int row) override;
    int                 nodeChildCount() const override;
    bool                nodeCanFetchMore()const override;
    void                nodeFetchMore()override;
    eTreeElemDataType   nodeDataTy()const override;
    const QString&      nodeDataTypeName()const override;
    int                 indexOfChild(const TreeNode * ptr)const override;
    bool                nodeShouldAutoExpand()const override                {return true;}
    bool                nodeAllowFetchMore()const override                  {return true;}
    QString             nodeDisplayName()const override;
    QVariant            nodeDecoration()const override;

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
    bool _moveChildrenNodes(const QModelIndexList &indices, int destrow, QModelIndex destparent) override;
    bool _moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent) override;

public:

    //Returns whether the sprite can be parsed, or not if there is something wrong with the raw data
    bool canParse()const;

    //Parse the data from the internal raw container
    void ParseSpriteData();

    //Turns the data from the data structure to its native format into the m_raw container!
    void CommitSpriteData();

    //Whether there's uncommited data for this sprite in its raw data container
    bool hasUnsavedChanges()const;

    //Dumping
    void DumpSpriteToStream(QDataStream & outstr);
    void DumpSpriteToFile(const QString & fpath);
    void DumpSpriteToXML(const QString & fpath);

    inline bool hasRawData()const   {return !m_raw.empty();}
    inline bool wasParsed()const    {return m_bparsed;}
    inline bool hasImageData()const {return m_imgcnt.nodeHasChildren();}
    inline bool hasEfxOffsets()const{return type() == fmt::eSpriteType::Character;}
    inline bool hasAnimGrps()const  {return type() == fmt::eSpriteType::Character;}


    QPixmap &       MakePreviewPalette();
    QPixmap         MakePreviewPalette()const; //Ignores cached palette, and makes one from scratch
    //Generate a preview of a 16 color sub-palette
    QPixmap         MakePreviewSubPalette(int subpalid)const;
    const QPixmap & getCachedPreviewPalette()const;

    QPixmap &       MakePreviewFrame(bool transparency = false);
    QPixmap         MakePreviewFrame(bool transparency = false)const; //Ignores cached image, and makes one from scratch
    const QPixmap & getCachedPreviewFrame()const;

    const QVector<QRgb> &   getPalette()const                       { return m_palcnt.getPalette(); }
    QVector<QRgb>       &   getPalette()                            { return m_palcnt.getPalette(); }
    inline void             setPalette(const QVector<QRgb> & pal)   {m_palcnt.setPalette(pal);}

    inline AnimSequences        & getAnimSequences()                                {return m_seqcnt;}
    inline const AnimSequences  & getAnimSequences()const                           {return m_seqcnt;}
    inline AnimSequence         * getAnimSequence(fmt::animseqid_t id)      {return m_seqcnt.getSequenceByID(id);}
    inline const AnimSequence   * getAnimSequence(fmt::animseqid_t id)const {return m_seqcnt.getSequenceByID(id);}

    inline AnimGroups           & getAnimGroups()                               {return m_anmgrp;}
    inline const AnimGroups     & getAnimGroups()const                          {return m_anmgrp;}
    inline AnimGroup            * getAnimGroup(fmt::animgrpid_t id)     {return m_anmgrp.getGroup(id);}
    inline const AnimGroup      * getAnimGroup(fmt::animgrpid_t id)const{return m_anmgrp.getGroup(id);}

    inline AnimTable            & getAnimTable()        {return m_anmtbl;}
    inline const AnimTable      & getAnimTable()const   {return m_anmtbl;}

    inline ImageContainer       & getImages()                       {return m_imgcnt;}
    inline const ImageContainer & getImages()const                  {return m_imgcnt;}
    Image                       * getImage(fmt::frmid_t idx);
    const Image                 * getImage(fmt::frmid_t idx)const;


    inline MFrame               * getFrame( fmt::frmid_t id )       {return m_frmcnt.getFrame(id);}
    inline const MFrame         * getFrame( fmt::frmid_t id )const  {return m_frmcnt.getFrame(id);}
    inline FramesContainer      & getFrames()                       {return m_frmcnt;}
    inline const FramesContainer& getFrames()const                  {return m_frmcnt;}

    const EffectOffsetSet       * getAttachMarkers(fmt::frmid_t frmidx)const; //returns attachment markers for the given mframe index
    EffectOffsetSet             * getAttachMarkers(fmt::frmid_t frmidx); //returns attachment markers for the given mframe index

    //Return binary version of the sprite
    inline rawdat_t             & getRawData()      {return m_raw;}
    inline const rawdat_t       & getRawData()const {return m_raw;}

    inline fmt::eSpriteType type()const    {return m_sprhndl.getSpriteType();}
    void                    convertSpriteToType(fmt::eSpriteType newty);

    //SPRITE PROPERTIES
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

    uint16_t getMaxTileUsage()const;

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

    //Utilities
public:
    typedef std::pair<QRect, QImage>    imgpart_t; //Single chunk of an image with its position from the original image
    typedef QVector<imgpart_t>          imgparts_t; //All the parts of a single image
    typedef QVector<imgparts_t>         imgseq_t; //All the images part of a sequence
    typedef QVector<imgseq_t>           imgseqs_t; //A set of sequences

    fmt::frmid_t importImageParts(const imgparts_t & img); //returns img ids of the imported parts in the same order!
    fmt::animseqid_t importImageSequence(const imgseq_t & seq, uint8_t frmduration = 1);
    void importImageSequences(const imgseqs_t & sequences, uint8_t frmduration = 1);


private:
    bool IsRawDataCompressed(filetypes::eCompressionFormats * outfmt = nullptr)const;
    void DecompressRawData();
    void CompressRawData(filetypes::eCompressionFormats cpfmt);

    void ResizeAnimGroupSlots(int newsz);

    void ClearAttachPoints();
    void CreateAttachPoints();

    TreeNode       * ElemPtr(int idx);
    const TreeNode * ElemPtr(int idx)const;
    int              nbChildCat()const;

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
    AnimGroups              m_anmgrp;

    //Status / statistics
    bool                                    m_bparsed{false};       //Whether the sprite's raw has been parsed to be displayed yet or not!
    bool                                    m_bhasimagedata{false}; //Whether the sprite can be displayed or not!
    filetypes::eCompressionFormats          m_targetgompression{filetypes::eCompressionFormats::NONE};

    //UI data
    QPixmap m_previewImg;       //Cached image preview
    QPixmap m_previewPal;       //Cached palette preview

};

#endif // SPRITE_H
