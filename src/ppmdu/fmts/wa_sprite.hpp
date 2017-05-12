#ifndef WA_SPRITE_HPP
#define WA_SPRITE_HPP
/*
wa_sprite.hpp
2017/05/05
psycommando@gmail.com
Description: Utilities for reading and writing WAN sprites, WAT sprites, and etc..
*/
#include <cstdint>
#include <vector>
#include <algorithm>
#include <forward_list>
#include <list>
#include <unordered_map>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/sir0.hpp>

namespace fmt
{
    //Palette colors
    typedef uint32_t            rgbx_t;        //Data type for storing a color's 4 bytes data under the format RRGGBBXX (XX is unused)
    typedef int16_t             frmid_t;       //-1 is a nodraw frame
    typedef std::vector<rgbx_t> rbgx24pal_t;

    //Frame resolutions
    extern const std::array<std::pair<uint16_t,uint16_t>, 12> FrameResValues;
    //Possible resolutions for individual parts of an assembled frame.
    //The value of the bits matches the 2 first bits of attr1 and attr2 respectively for representing the resolution!
    enum struct eFrameRes : uint16_t
    {
        Square_8x8   = 0,
        Square_16x16 = 1,
        Square_32x32 = 2,
        Square_64x64 = 3,

        Wider_16x8   = 4,
        Wider_32x8   = 5,
        Wider_32x16  = 6,
        Wider_64x32  = 7,

        Higher_8x16  = 8,
        Higher_8x32  = 9,
        Higher_16x32 = 10,
        Higher_32x64 = 11,
        Invalid,
    };

    //Sprite formats variants
    enum struct eSpriteType : uint16_t
    {
        Prop        = 0,
        Character   = 1,
        Effect      = 2,
        WAT         = 3,
        INVALID,
    };

//-----------------------------------------------------------------------------
//  Data
//-----------------------------------------------------------------------------
    /**********************************************************************
     * imgfmtinfo
     *      Chunk of the header containing information on the image format and data.
    **********************************************************************/
    struct imgfmtinfo
    {
        uint32_t ptrimgtable;   //pointer to the image data pointer table
        uint32_t ptrpal;
        uint16_t unk13;         //tiling?
        uint16_t colordepth;    //1 == 256 colors, 0 == 16 colors
        uint16_t unk11;         //palette slot?
        uint16_t nbimgs;        //nb images in the img data table

        imgfmtinfo()
            :ptrimgtable(0), ptrpal(0), unk13(0), colordepth(0), unk11(0), nbimgs(0)
        {}

        template<class _init>
        _init read( _init beg, _init end )
        {
            beg = utils::readBytesAs( beg, end, ptrimgtable );
            beg = utils::readBytesAs( beg, end, ptrpal );
            beg = utils::readBytesAs( beg, end, unk13 );
            beg = utils::readBytesAs( beg, end, colordepth );
            beg = utils::readBytesAs( beg, end, unk11 );
            beg = utils::readBytesAs( beg, end, nbimgs );
            return beg;
        }

        template<class _outit>
        _outit write( _outit where )
        {
            where = utils::writeBytesFrom( ptrimgtable,    where);
            where = utils::writeBytesFrom( ptrpal,         where);
            where = utils::writeBytesFrom( unk13,          where);
            where = utils::writeBytesFrom( colordepth,     where);
            where = utils::writeBytesFrom( unk11,          where);
            where = utils::writeBytesFrom( nbimgs,         where);
            return where;
        }

        //Utility function for pushing the pointer offsets of the pointers in this struct into the specified container
        //Used for building the SIR0 wrapper
        template<class _STDCNT>
            uint32_t MarkPointers( uint32_t imgfmtbegoffset, _STDCNT ptrcnt )
        {
            ptrcnt.push_back(imgfmtbegoffset); //ptrimgtable
            ptrcnt.push_back(imgfmtbegoffset += sizeof(uint32_t)); //ptrpal
            return imgfmtbegoffset;
        }

        inline bool is256Colors()const {return colordepth != 0;}
    };

    /**********************************************************************
     * animfmtinfo
     *      Chunk of the header containing informaton on how to handle animations.
    **********************************************************************/
    struct animfmtinfo
    {
        uint32_t ptroamtbl;     //pointer to the frame assembly table/oam data
        uint32_t ptrefxtbl;     //pointer to the effect offsets table
        uint32_t ptranimtbl;    //pointer to the animation table
        uint16_t nbanims;       //nb of animations in the animation table
        uint16_t unk6;          //block size?
        uint16_t unk7;
        uint16_t unk8;
        uint16_t unk9;          //possibly boolean
        uint16_t unk10;

        animfmtinfo()
            :ptroamtbl(0), ptrefxtbl(0),ptranimtbl(0), nbanims(0),unk6(0), unk7(0), unk8(0), unk9(0), unk10(0)
        {}

        template<class _init>
        _init read( _init beg, _init end )
        {
            beg = utils::readBytesAs( beg, end, ptroamtbl );
            beg = utils::readBytesAs( beg, end, ptrefxtbl );
            beg = utils::readBytesAs( beg, end, ptranimtbl );
            beg = utils::readBytesAs( beg, end, nbanims );
            beg = utils::readBytesAs( beg, end, unk6 );
            beg = utils::readBytesAs( beg, end, unk7 );
            beg = utils::readBytesAs( beg, end, unk8 );
            beg = utils::readBytesAs( beg, end, unk9 );
            beg = utils::readBytesAs( beg, end, unk10 );
            return beg;
        }


        //Utility function for pushing the pointer offsets of the pointers in this struct into the specified container
        //Used for building the SIR0 wrapper
        template<class _STDCNT>
            uint32_t MarkPointers( uint32_t animfmtinfobegoffset, _STDCNT ptrcnt )
        {
            ptrcnt.push_back(animfmtinfobegoffset); //ptroamtbl
            ptrcnt.push_back(animfmtinfobegoffset += sizeof(uint32_t)); //ptrefxtbl
            ptrcnt.push_back(animfmtinfobegoffset += sizeof(uint32_t)); //ptranimtbl
            return animfmtinfobegoffset;
        }
    };


    /**********************************************************************
     * effectoffset
     *      An offset from the effect offset table.
    **********************************************************************/
    struct effectoffset
    {
        uint16_t xoff = 0;
        uint16_t yoff = 0;
    };


    /**********************************************************************
     * palettedata
     *      Header chunk for the color palette. Contains data on how to
     *      parse the colors.
    **********************************************************************/
    struct palettedata
    {
        rbgx24pal_t colors;
        //uint32_t ptrpalbeg; //
        uint16_t unk3;      //force4bb
        uint16_t nbcol;
        uint16_t unk4;
        uint16_t unk5;
        //4bytes of zeroes here
    };

    /**********************************************************************
     * step_t
     *      Part of an assembled frame. Contains details on where to place
     *      the part and if any special operation is ran on it.
     *      Contains OAM attribute data as it is stored in NDS memory.
    **********************************************************************/
    struct step_t
    {
        static const uint16_t ATTR0_FlagBitsMask = 0xFF00;   //1111 1111 0000 0000
        static const uint16_t ATTR1_FlagBitsMask = 0xFE00;   //1111 1110 0000 0000
        static const uint16_t ATTR2_FlagBitsMask = 0xFC00;   //1111 1100 0000 0000

        static const uint16_t ATTR0_ColPalMask   = 0x2000;   //0010 0000 0000 0000
        static const uint16_t ATTR0_MosaicMask   = 0x1000;   //0001 0000 0000 0000
        static const uint16_t ATTR0_ObjModeMask  = 0x0C00;   //0000 1100 0000 0000
        static const uint16_t ATTR0_DblSzDisabled= 0x0200;   //0000 0010 0000 0000 (Whether the obj is disabled if rot&scaling is off, or if double sized when rot&scaling is on!)
        static const uint16_t ATTR0_RotNScaleMask= 0x0100;   //0000 0001 0000 0000
        static const uint16_t ATTR0_YOffsetMask  = ~ATTR0_FlagBitsMask;   //0000 0000 1111 1111

        static const uint16_t ATTR1_VFlipMask    = 0x2000;   //0010 0000 0000 0000
        static const uint16_t ATTR1_HFlipMask    = 0x1000;   //0001 0000 0000 0000
        static const uint16_t ATTR1_IsLastMask   = 0x0800;   //0000 1000 0000 0000
        static const uint16_t ATTR1_RotNScalePrm = 0x3E00;   //0011 1110 0000 0000
        static const uint16_t ATTR1_XOffsetMask  = ~ATTR1_FlagBitsMask;   //0000 0001 1111 1111

        static const uint16_t ATTR2_PalNumberMask= 0xF000;   //1111 0000 0000 0000
        static const uint16_t ATTR2_PriorityMask = 0x0C00;   //0000 1100 0000 0000
        static const uint16_t ATTR2_TileNumMask  = 0x03FF;   //0000 0011 1111 1111

        static const uint16_t  ATTR01_ResMask    = 0xC000;   //1100 0000 0000 0000

        /*
         * eObjMode
         *
        */
        enum struct eObjMode: uint16_t
        {
            Normal      = 0,
            SemiTransp  = 0x400,
            Window      = 0x800,
            Invalid,
        };

        frmid_t  frmidx;
        uint16_t unk0;
        uint16_t attr0;
        uint16_t attr1;
        uint16_t attr2;

        template<class _init>
            _init read(_init beg, _init end)
        {
            beg = utils::readBytesAs( beg, end, frmidx );
            beg = utils::readBytesAs( beg, end, unk0 );
            beg = utils::readBytesAs( beg, end, attr0 );
            beg = utils::readBytesAs( beg, end, attr1 );
            beg = utils::readBytesAs( beg, end, attr2 );
            return beg;
        }

        template<class _outit>
            _outit write(_outit where)
        {
            where = utils::writeBytesFrom( frmidx,  where);
            where = utils::writeBytesFrom( unk0,    where);
            where = utils::writeBytesFrom( attr0,   where);
            where = utils::writeBytesFrom( attr1,   where);
            where = utils::writeBytesFrom( attr2,   where);
            return where;
        }


        inline bool isColorPal256()const            {return (ATTR0_ColPalMask & attr0) != 0;}
        inline bool isMosaicOn()const               {return (ATTR0_MosaicMask & attr0) != 0;}
        inline eObjMode ObjMode()const              {return static_cast<eObjMode>(attr0 & ATTR0_ObjModeMask);}
        inline bool isDisabled()const               {return !isRotAndScalingOn() && ((ATTR0_DblSzDisabled & attr0) != 0);}
        inline bool isDoubleSize()const             {return isRotAndScalingOn() && ((ATTR0_DblSzDisabled & attr0) != 0);}
        inline bool isRotAndScalingOn()const        {return (ATTR0_RotNScaleMask & attr0) != 0;}
        inline uint16_t getYOffset()const           {return (ATTR0_YOffsetMask & attr0);}

        inline bool isVFlip()const                  {return (ATTR1_VFlipMask & attr1) != 0;}
        inline bool isHFlip()const                  {return (ATTR1_HFlipMask & attr1) != 0;}
        inline bool islast()const                   {return (ATTR1_IsLastMask & attr1) != 0;}
        inline uint8_t getRnSParam()const           {return static_cast<uint8_t>((ATTR1_RotNScalePrm & attr1) >> 9);}
        inline uint16_t getXOffset()const           {return (ATTR1_XOffsetMask & attr1);}

        inline uint8_t getPalNb()const              {return static_cast<uint8_t>((ATTR2_PalNumberMask & attr2) >> 12);}
        inline uint8_t getPriority()const           {return static_cast<uint8_t>((ATTR2_PriorityMask  & attr2) >> 10);}
        inline uint16_t getTileNum()const           {return (ATTR2_TileNumMask & attr2);}
        inline eFrameRes getResolutionType()const   { return static_cast<eFrameRes>( ( (attr1 & ATTR01_ResMask) >> 14) | ( (attr0 & ATTR01_ResMask) >> 12 ) ); }

        inline std::pair<uint16_t,uint16_t> GetResolution()const
        {
            uint8_t  flagresval = ((attr1 & ATTR01_ResMask) >> 14) | ( (attr0 & ATTR01_ResMask) >> 12 ) ; //Combine both into a number
            assert(flagresval < 12);
            return FrameResValues[flagresval];
        }


        inline void setColorPal256  (bool bis256)   {attr0 = (bis256)? (ATTR0_ColPalMask | attr0) : (attr0 ^ ATTR0_ColPalMask);}
        inline void setMosaicOn     (bool bon)      {attr0 = (bon)? (ATTR0_MosaicMask | attr0) : (attr0 ^ ATTR0_MosaicMask);}
        inline void setObjMode      (eObjMode mode) {attr0 = ((static_cast<uint16_t>(mode) | attr0) & ATTR0_ObjModeMask);}
        inline void setDisabled     (bool bon)      {attr0 = (bon)? (ATTR0_DblSzDisabled | attr0) : (attr0 ^ ATTR0_DblSzDisabled);}
        inline void setDoubleSize   (bool bon)      {setDisabled(bon);}
        inline void setRotAndScaling(bool bon)      {attr0 = (bon)? (ATTR0_RotNScaleMask | attr0) : (attr0 ^ ATTR0_RotNScaleMask);}
        inline void setYOffset      (uint16_t y)    {attr0 = (attr0 & ATTR0_FlagBitsMask) | (ATTR0_YOffsetMask & y);}

        inline void setVFlip        (bool bon)      {attr1 = (bon)? (ATTR1_VFlipMask | attr1) : (attr1 ^ ATTR1_VFlipMask);}
        inline void setHFlip        (bool bon)      {attr1 = (bon)? (ATTR1_HFlipMask | attr1) : (attr1 ^ ATTR1_HFlipMask);}
        inline void setLast         (bool blast)    {attr1 = (blast)? (ATTR1_IsLastMask | attr1) : (attr1 ^ ATTR1_IsLastMask);}
        inline void setRnSParam     (uint8_t param) {attr1 = (attr1 & ~ATTR1_RotNScalePrm) | ((param << 9) & ATTR1_RotNScalePrm);}
        inline void setXOffset      (uint16_t x)    {attr1 = (attr1 & ATTR1_FlagBitsMask) | (ATTR1_XOffsetMask & x);}

        inline void setPalNb        (uint8_t palnb) {attr2 = (attr2 & ~ATTR2_PalNumberMask) | (ATTR2_PalNumberMask & palnb);}
        inline void setPriority     (uint8_t prio)  {attr2 = (attr2 & ~ATTR2_PriorityMask) | (ATTR2_PriorityMask & prio);}
        inline void setTileNum      (uint16_t tnum) {attr2 = (attr2 & ~ATTR2_TileNumMask) | (ATTR2_TileNumMask & tnum);}

        inline void setResolutionType(eFrameRes res)
        {
            uint16_t flagval = static_cast<uint16_t>(res);
            attr1 = (attr1 & ~ATTR01_ResMask) | ((flagval << 14) & ATTR01_ResMask);
            attr0 = (attr0 & ~ATTR01_ResMask) | ((flagval << 12) & ATTR01_ResMask);
        }
    };

    /**********************************************************************
     * animfrm_t
     *      Contains data on how a frame is used in an animation sequence.
     *      Makes up one animation frame of a sequence.
    **********************************************************************/
    struct animfrm_t
    {
        uint8_t duration;
        uint8_t flag;
        int16_t frmidx;
        int16_t xoffs;
        int16_t yoffs;
        int16_t shadowxoffs;
        int16_t shadowyoffs;

        bool isnull()const
        {
            return !duration && !flag && !frmidx && !xoffs && !yoffs && !shadowxoffs && !shadowyoffs;
        }

        template<class _init>
            _init read(_init beg, _init end)
        {
            beg = utils::readBytesAs(beg, end, duration);
            beg = utils::readBytesAs(beg, end, flag);
            beg = utils::readBytesAs(beg, end, frmidx);
            beg = utils::readBytesAs(beg, end, xoffs);
            beg = utils::readBytesAs(beg, end, yoffs);
            beg = utils::readBytesAs(beg, end, shadowxoffs);
            beg = utils::readBytesAs(beg, end, shadowyoffs);
            return beg;
        }

        template<class _outit>
            _outit write(_outit where)
        {
            where = utils::writeBytesFrom(duration,    where);
            where = utils::writeBytesFrom(flag,        where);
            where = utils::writeBytesFrom(frmidx,      where);
            where = utils::writeBytesFrom(xoffs,       where);
            where = utils::writeBytesFrom(yoffs,       where);
            where = utils::writeBytesFrom(shadowxoffs, where);
            where = utils::writeBytesFrom(shadowyoffs, where);
            return where;
        }
    };

//-----------------------------------------------------------------------------
//  ImageDB
//-----------------------------------------------------------------------------
    /**********************************************************************
     * Handles parsing/writing/containing everything related to image data
     * from a sprite!
    **********************************************************************/
    struct ImageDB
    {
        typedef std::vector<uint8_t>    img_t;
        typedef std::list<step_t>       frm_t;
        typedef std::vector<img_t>      imgtbl_t;
        typedef std::vector<frm_t>      frmtbl_t;
        imgtbl_t    m_images;    //contains decoded raw linear images
        frmtbl_t    m_frames;    //References on how to assemble raw images into individual animation frames.
        palettedata m_pal;

        template<class _init>
        void ParsePalette(_init itsrcbeg, _init itsrcend, uint32_t paloff)
        {
            using namespace std;
            if(paloff == 0)
                return;

            auto itpaldata = next(itsrcbeg, paloff);
            uint32_t ptrbegpal = utils::readBytesAs<uint32_t>( itpaldata, itsrcend );
            itpaldata = utils::readBytesAs( itpaldata, itsrcend, m_pal.unk3 );
            itpaldata = utils::readBytesAs( itpaldata, itsrcend, m_pal.nbcol);
            itpaldata = utils::readBytesAs( itpaldata, itsrcend, m_pal.unk4 );
            itpaldata = utils::readBytesAs( itpaldata, itsrcend, m_pal.unk5 );

            if(itpaldata == 0)
                return;

            //grab colors!
            auto itpalcolor = next(itsrcbeg, ptrbegpal);
            size_t pallen = std::abs(paloff - ptrbegpal) / sizeof(uint32_t);
            for( size_t cntcol = 0; cntcol < pallen; ++cntcol )
            {
                rgbx_t curcol;
                itpalcolor = utils::readBytesAs( itpalcolor, itsrcend, curcol, false );
                m_pal.colors.push_back( curcol );
            }
        }

        template<class _init>
            void ParseAFrame(_init itsrcbeg, _init itsrcend, uint32_t frmoff)
        {
            using namespace std;
            if(frmoff == 0)
                return;

            //A frame is made up of several sub-entry with details on how to assemble the final frame.
            auto itstep = next(itsrcbeg, frmoff);
            frm_t curfrm;
            step_t curstep;
            do
            {
                itstep = curstep.read( itstep, itsrcend);
                curfrm.push_back(move(curstep));
            }while( !curstep.islast() );
            m_frames.push_back(std::move(curfrm));
        }

        template<class _init>
            void ParseAnImage(_init itsrcbeg, _init itsrcend, uint32_t imgoff)
        {
            using namespace std;

            if(imgoff == 0)
                return;

            //Read comp table
            auto itcmptable = next(itsrcbeg, imgoff);
            img_t curimg;
            uint16_t len = 0;
            do
            {
                uint32_t pixsrc = utils::readBytesAs<uint32_t>(itcmptable, itsrcend);
                len             = utils::readBytesAs<uint16_t>(itcmptable, itsrcend);
                uint16_t unk14  = utils::readBytesAs<uint16_t>(itcmptable, itsrcend);
                uint32_t unk2   = utils::readBytesAs<uint32_t>(itcmptable, itsrcend);

                if( pixsrc == 0 )
                    fill_n( back_inserter(curimg), len, 0 );
                else
                {
                    auto itimg = next(itsrcbeg, pixsrc);
                    copy( itimg, next(itimg,len), back_inserter(curimg) );
                }

            }while( len != 0 ); //null entry is last
            m_images.push_back(move(curimg));
        }

        template<class _init>
            void ParseImgData( _init itsrcbeg, _init itsrcend, const imgfmtinfo & imginf, const animfmtinfo & animinf  )
        {
            using namespace std;

            if( imginf.ptrimgtable != 0 )
            {
                //Grab the images
                auto itimgtbl = next( itsrcbeg, imginf.ptrimgtable );

                for( size_t cntimg = 0; cntimg < imginf.nbimgs; ++cntimg )
                {
                    uint32_t curptr = utils::readBytesAs<uint32_t>(itimgtbl, itsrcend);
                    if( curptr != 0 )
                        ParseAnImage(itsrcbeg, itsrcend, curptr);
                }
            }

            if( imginf.ptrpal != 0 )
            {
                //Grab the palette
                ParsePalette(itsrcbeg, itsrcend, imginf.ptrpal);
            }

            if( animinf.ptroamtbl != 0 )
            {
                //Grab the frames
                auto itfrmtbl = next( itsrcbeg, animinf.ptroamtbl );
                const size_t nbfrm  = (animinf.ptrefxtbl - animinf.ptroamtbl) / sizeof(uint32_t);

                for( size_t cntfrm = 0; cntfrm < nbfrm && itfrmtbl != itsrcend; ++cntfrm )
                {
                    uint32_t curptr = utils::readBytesAs<uint32_t>(itfrmtbl, itsrcend);
                    if( curptr != 0 )
                        ParseAFrame(itsrcbeg, itsrcend, curptr);
                }
            }
        }

    };

//-----------------------------------------------------------------------------
//  AnimDB
//-----------------------------------------------------------------------------
    /**********************************************************************
     * Sub component of a sprite for handling animation data.
    **********************************************************************/
    struct AnimDB
    {
        //A single sequence of animated frames
        typedef int                                         animseqid_t;   //Id for an animation sequence
        typedef int                                         animgrpid_t;   //Id for an animation group
        typedef std::list<animfrm_t>                        animseq_t;
        typedef std::vector<animgrpid_t>                    animtbl_t;
        /**********************************************************************
         * A group is a set of animation sequences
        **********************************************************************/
        struct animgrp_t
        {
            std::vector<animseqid_t> seqs;
            uint16_t                 unk16;
        };
        typedef std::unordered_map<animgrpid_t, animgrp_t>  animgrptbl_t;
        typedef std::unordered_map<animseqid_t, animseq_t>  animseqtbl_t;



        static const animseqid_t NullSeq = -1;  //Represents an entry not refering to any sequence
        static const animgrpid_t NullGrp = -1;  //Represents an entry not refering to any group

        animseqtbl_t  m_animsequences;    //A table containing all uniques animation sequences
        animgrptbl_t  m_animgrps;         //A table of all the unique animation groups
        animtbl_t     m_animtbl;          //A table of pointers to the animation group associated to an animation

        template<class _init>
            animseqid_t ParseAnimSeq( std::unordered_map<uint32_t,animgrpid_t> & animseqreftable,
                                      _init itsrcbeg,
                                      _init itsrcend,
                                      uint32_t seqoffset )
        {
            using namespace std;
            if(seqoffset == 0)
                return NullSeq;

            //check if we have it already
            auto itf = animseqreftable.find(seqoffset);
            if(itf != animseqreftable.end() )
                return itf->second; //set existing id! No need to add anything to the sequence table

            //Parse the sequence!
            auto itseq = next(itsrcbeg, seqoffset);
            animfrm_t curfrm;
            do
            {
                itseq = curfrm.read(itseq,itsrcend);
                if( !curfrm.isnull() )
                    m_animsequences[curseqid].push_back(curfrm);
            }while( !curfrm.isnull() && itseq != itsrcend );

            //Add seq to the ref table
            animseqreftable.insert( {seqoffset, curseqid} );
            ++curseqid;

            return (curseqid - 1); //return our id!
        }


        template<class _init>
            animgrpid_t ParseAnimGroup( std::unordered_map<uint32_t,animgrpid_t> & animgrpreftable,
                                        std::unordered_map<uint32_t,animgrpid_t> & animseqreftable,
                                        _init itsrcbeg,
                                        _init itsrcend,
                                        uint32_t offanimgrp,
                                        uint16_t grplen,
                                        uint16_t unk16 )
        {
            using namespace std;
            if( offanimgrp == 0 )
                return NullGrp;

            //check if we have it already
            auto itf = animgrpreftable.find(offanimgrp);
            if(itf != animgrpreftable.end() )
                return itf->second; //set existing id! No need to add anything to the group table

            //Parse the group!
            auto itgrp = next(itsrcbeg, offanimgrp);
            m_animgrps[curgrpid].unk16 = unk16;
            for( size_t cntseq = 0; cntseq < grplen && itgrp != itsrcend; ++cntseq )
            {
                uint32_t curptr = 0;
                itgrp = utils::readBytesAs( itgrp, itsrcend, curptr );
                m_animgrps[curgrpid].seqs.push_back( ParseAnimSeq(animseqreftable, itsrcbeg, itsrcend, curptr) );
            }


            //Add group to the ref table
            animgrpreftable.insert( {offanimgrp, curgrpid} );
            ++curgrpid;

            return (curgrpid - 1); //return our id!
        }

        template<class _init>
            void ParseAnimTbl(_init itsrcbeg, _init itsrcend, const animfmtinfo & animinf )
        {
            using namespace std;
            curgrpid = 0;
            curseqid = 0;
            unordered_map<uint32_t,animgrpid_t> animgrpreftable; //(pointer, assignedid) used to avoid duplicating groups when the same group is refered to in multiple locations
            unordered_map<uint32_t,animgrpid_t> animseqreftable; //(pointer, assignedid) used to avoid duplicate sequences

            //Parse anim table first!
            if( animinf.ptranimtbl != 0 )
            {
                auto animtblbeg = std::next( itsrcbeg, animinf.ptranimtbl);
                m_animtbl.resize( animinf.nbanims );
                for( size_t cntptr = 0; cntptr < m_animtbl.size() && animtblbeg != itsrcend; ++cntptr )
                {
                    //Read current entry
                    uint32_t curptr = 0;
                    uint16_t grplen = 0;
                    uint16_t unk16  = 0;
                    animtblbeg = utils::readBytesAs( animtblbeg, itsrcend, curptr );
                    animtblbeg = utils::readBytesAs( animtblbeg, itsrcend, grplen );
                    animtblbeg = utils::readBytesAs( animtblbeg, itsrcend, unk16 );
                    //Parse it
                    m_animtbl[cntptr] = ParseAnimGroup( animgrpreftable, animseqreftable, itsrcbeg, itsrcend, curptr, grplen, unk16 );
                }
            }
        }

    private:
        animgrpid_t curgrpid;
        animseqid_t curseqid;
    };

//-----------------------------------------------------------------------------
//  WA_SpriteHandler
//-----------------------------------------------------------------------------

    /**********************************************************************
     *
     * This class is used as a sort of in-between between the file representation and in-memory representation of a wa_ sprite.
     * Basically it contains the raw data of a sprite, and provides ways to translates those to and from.
    **********************************************************************/
    class WA_SpriteHandler
    {
    public:
        typedef std::vector<effectoffset> OffsetsDB;

        template<class _init>
            void Parse( _init itbeg, _init itend )
        {
            auto itsrcbeg = itbeg;

            ParseHeaders(itbeg, itend);
            ParseImageInfo(itbeg, itend);
            ParseAnimInfoAndData(itbeg, itend);
        }

        template<class _outit>
            _outit Write( _outit itout )
        {
            assert(false);
            //Generate and track SIR0 pointer list!!!
            return itout;
        }

        // ----------- Data Access -----------
        inline ImageDB::frm_t      & getFrame(frmid_t frmid)                   {return m_images.m_frames.at(frmid);}
        inline ImageDB::frmtbl_t   & getFrames()                               {return m_images.m_frames;}
        inline ImageDB::img_t      & getImage(size_t imgidx)                   {return m_images.m_images.at(imgidx);}
        inline ImageDB::imgtbl_t   & getImages()                               {return m_images.m_images;}
        inline rbgx24pal_t         & getPalette()                              {return m_images.m_pal.colors;}
        inline palettedata         & getPaletteData()                          {return m_images.m_pal;}
        inline AnimDB::animtbl_t   & getAnimationTable()                       {return m_animtions.m_animtbl;}
        inline AnimDB::animgrp_t   & getAnimGroup(AnimDB::animgrpid_t id)      {return m_animtions.m_animgrps[id];}
        inline AnimDB::animgrptbl_t& getAnimGroups()                           {return m_animtions.m_animgrps; }
        inline AnimDB::animseq_t   & getAnimSeq(AnimDB::animseqid_t id)        {return m_animtions.m_animsequences[id];}
        inline AnimDB::animseqtbl_t& getAnimSeqs()                             {return m_animtions.m_animsequences;}
        inline OffsetsDB           & getEffectOffset()                         {return m_efxoffsets;}

        inline const animfmtinfo   & getAnimFmtInfo()const                     {return m_animfmt;}
        inline const imgfmtinfo    & getImageFmtInfo()const                    {return m_imgfmt;}

    private:

        //pointer to the sprite header
        //uint32_t    m_ptrsprhdr;

        //Sprite Header
        uint32_t    m_offsetAnimInfo;
        uint32_t    m_offsetImgInfo;
        eSpriteType m_sprty;
        uint16_t    m_unk12;

        //
        imgfmtinfo  m_imgfmt;
        animfmtinfo m_animfmt;

        //Content
        ImageDB     m_images;
        AnimDB      m_animtions;
        OffsetsDB   m_efxoffsets;

    private:
        template<class _init>
            void ParseHeaders(_init itbeg, _init itend )
        {
            auto itsrcbeg = itbeg;
            //#1. Parse SIR0 hdr
            fmt::SIR0hdr hdr;
            itbeg = hdr.Read(itbeg, itend);

            //#2. Parse Sprite hdr
            itbeg = std::next( itsrcbeg, hdr.ptrsub );
            itbeg = utils::readBytesAs( itbeg, itend, m_offsetAnimInfo );
            itbeg  = utils::readBytesAs( itbeg, itend, m_offsetImgInfo );
            m_sprty = static_cast<eSpriteType>(utils::readBytesAs<uint16_t>( itbeg, itend ));
            itbeg = utils::readBytesAs( itbeg, itend, m_unk12 );

            if( m_sprty >= eSpriteType::INVALID )
                throw std::runtime_error("WA_SpriteHandler::Parse(): Invalid sprite type!!"); //STOP HERE!!
            assert( m_sprty < eSpriteType::INVALID );

            if( m_offsetAnimInfo != 0 )
                m_animfmt.read(std::next( itsrcbeg, m_offsetAnimInfo ), itend); //Parse anim info chunk

            if( m_offsetImgInfo != 0 )
                m_imgfmt.read(std::next( itsrcbeg, m_offsetImgInfo ), itend);
        }


        template<class _init>
            void ParseImageInfo(_init itbeg, _init itend )
        {
            m_images.ParseImgData(itbeg, itend, m_imgfmt, m_animfmt);
        }

        template<class _init>
            void ParseAnimInfoAndData(_init itbeg, _init itend )
        {
            //Parse animations
            m_animtions.ParseAnimTbl(itbeg, itend, m_animfmt);
            if(m_animfmt.ptrefxtbl != 0)
            {
                //1. Calculate effect table length/end
                //#TODO
            }
        }
    };
};

#endif // WA_SPRITE_HPP
