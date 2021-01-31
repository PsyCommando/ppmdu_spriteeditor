#ifndef RAW_SPRITE_DATA_HPP
#define RAW_SPRITE_DATA_HPP
#include <cstdint>
#include <vector>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/sir0.hpp>
#include <src/ppmdu/fmts/sprite/shared_sprite_constants.hpp>

namespace fmt
{
//-----------------------------------------------------------------------------
//  Headers
//-----------------------------------------------------------------------------
    /**********************************************************************
     * hdr_wan
     *  Sub header for the wan/wat file format
    **********************************************************************/
    struct hdr_wan
    {
        static const uint32_t LEN   {12}; //bytes
        uint32_t ptraniminfo        {0};
        uint32_t ptrimgdatinfo      {0};
        uint16_t spritety           {0};
        uint16_t unk12              {0};

        template<class _init>
            _init read( _init beg, _init end )
        {
            beg = utils::readBytesAs( beg, end, ptraniminfo );
            beg = utils::readBytesAs( beg, end, ptrimgdatinfo );
            beg = utils::readBytesAs( beg, end, spritety );
            beg = utils::readBytesAs( beg, end, unk12 );
            return beg;
        }

        template<class _writerhelper_t>
            _writerhelper_t & write( _writerhelper_t & sir0hlpr)const
        {
            sir0hlpr.writePtr(ptraniminfo);
            sir0hlpr.writePtr(ptrimgdatinfo);
            sir0hlpr.writeVal(spritety);
            sir0hlpr.writeVal(unk12);
            return sir0hlpr;
        }
    };

    /**********************************************************************
     * hdr_imgfmtinfo
     *      Chunk of the header containing information on the image format and data.
    **********************************************************************/
    struct hdr_imgfmtinfo
    {
        uint32_t ptrimgtable    {0};    //pointer to the image data pointer table
        uint32_t ptrpal         {0};    //pointer to palette data
        uint16_t mappingmode    {0};    //VRAM Character Mapping. 0 = 2D Mapping(Tiles placed on a 32x32 matrix), 1 = 1D Mapping(Tiles loaded one after another).
        uint16_t colordepth     {0};    //1 == 256 colors, 0 == 16 colors
        uint16_t unk11          {0};    //palette slot?
        uint16_t nbimgs         {0};    //nb images in the img data table

        template<class _init>
        _init read( _init beg, _init end )
        {
            beg = utils::readBytesAs( beg, end, ptrimgtable );
            beg = utils::readBytesAs( beg, end, ptrpal );
            beg = utils::readBytesAs( beg, end, mappingmode );
            beg = utils::readBytesAs( beg, end, colordepth );
            beg = utils::readBytesAs( beg, end, unk11 );
            beg = utils::readBytesAs( beg, end, nbimgs );
            return beg;
        }

        template<class _writerhelper_t>
            _writerhelper_t & write( _writerhelper_t & sir0hlpr)const
        {
            sir0hlpr.writePtr(ptrimgtable);
            sir0hlpr.writePtr(ptrpal);
            sir0hlpr.writeVal(mappingmode);
            sir0hlpr.writeVal(colordepth);
            sir0hlpr.writeVal(unk11);
            sir0hlpr.writeVal(nbimgs);
            return sir0hlpr;
        }

        inline bool is256Colors()const {return colordepth != 0;}
        inline void setIs256Colors(bool v) {colordepth = (v)? 1 : 0;}
    };

    /**********************************************************************
     * hdr_animfmtinfo
     *      Chunk of the header containing informaton on how to handle animations.
    **********************************************************************/
    struct hdr_animfmtinfo
    {
        uint32_t ptroamtbl;     //pointer to the frame assembly table/oam data
        uint32_t ptrattachtbl;  //pointer to the attachment points table
        uint32_t ptranimtbl;    //pointer to the animation table
        uint16_t nbanims;       //nb of animations in the animation table
        uint16_t maxnbusedblocks;//The number of tiles that the bigest assembled frame takes in memory
        uint16_t unk7;
        uint16_t unk8;
        uint16_t unk9;          //possibly boolean
        uint16_t unk10;

        hdr_animfmtinfo()
            :ptroamtbl(0), ptrattachtbl(0),ptranimtbl(0), nbanims(0),maxnbusedblocks(0), unk7(0), unk8(0), unk9(0), unk10(0)
        {}

        template<class _init>
        _init read( _init beg, _init end )
        {
            beg = utils::readBytesAs( beg, end, ptroamtbl );
            beg = utils::readBytesAs( beg, end, ptrattachtbl );
            beg = utils::readBytesAs( beg, end, ptranimtbl );
            beg = utils::readBytesAs( beg, end, nbanims );
            beg = utils::readBytesAs( beg, end, maxnbusedblocks );
            beg = utils::readBytesAs( beg, end, unk7 );
            beg = utils::readBytesAs( beg, end, unk8 );
            beg = utils::readBytesAs( beg, end, unk9 );
            beg = utils::readBytesAs( beg, end, unk10 );
            return beg;
        }

        template<class _writerhelper_t>
            _writerhelper_t & write( _writerhelper_t & sir0hlpr )const
        {
            sir0hlpr.writePtr(ptroamtbl);
            sir0hlpr.writePtr(ptrattachtbl);
            sir0hlpr.writePtr(ptranimtbl);
            sir0hlpr.writeVal(nbanims);
            sir0hlpr.writeVal(maxnbusedblocks);
            sir0hlpr.writeVal(unk7);
            sir0hlpr.writeVal(unk8);
            sir0hlpr.writeVal(unk9);
            sir0hlpr.writeVal(unk10);
            return sir0hlpr;
        }

        bool isNull()const
        {
            return ptroamtbl == 0 &&
                   ptrattachtbl == 0 &&
                   ptranimtbl == 0 &&
                   nbanims == 0 &&
                   maxnbusedblocks == 0 &&
                   unk7 == 0 &&
                   unk8 == 0 &&
                   unk9 == 0 &&
                   unk10 == 0;
        }
    };

    /**********************************************************************
     * hdr_palettedata
     *      Header chunk for the color palette. Contains data on how to
     *      parse the colors.
    **********************************************************************/
    struct hdr_palettedata
    {
        static const size_t LEN = 16; //Size of the structure in the file

        rbgx24pal_t colors;
        //uint32_t ptrpalbeg; //
        uint16_t unk3;      //force4bb
        uint16_t nbcol;
        uint16_t unk4;
        uint16_t unk5;
        //4bytes of zeroes here
    };

//-----------------------------------------------------------------------------
//  Internal Data Format
//-----------------------------------------------------------------------------
    /**********************************************************************
     * offset_t
     *      An offset from the effect offset table.
    **********************************************************************/
    struct offset_t
    {
        static const uint32_t LEN {4}; //in bytes
        int16_t xoff = 0;
        int16_t yoff = 0;

        template<class _init>
        _init read( _init beg, _init end )
        {
            assert(beg != end);
            beg = utils::readBytesAs( beg, end, xoff );
            beg = utils::readBytesAs( beg, end, yoff );
            return beg;
        }

        template<class _writerhelper_t>
            _writerhelper_t & write( _writerhelper_t & sir0hlpr )const
        {
            sir0hlpr.writeVal(xoff);
            sir0hlpr.writeVal(yoff);
            return sir0hlpr;
        }
    };

    /**********************************************************************
     * frameoffsets_t
     *      The 4 offsets for placing effects sprite on a given frame.
     *      Basically the contents of the efx table
    **********************************************************************/
    struct frameoffsets_t
    {
        static const uint32_t LEN {offset_t::LEN * 4};
        offset_t head;
        offset_t rhand;
        offset_t lhand;
        offset_t center;

        frameoffsets_t(){}

        frameoffsets_t(const offset_t & hd, const offset_t & rhnd, const offset_t & lhnd, const offset_t & cntr)
        {
            head = hd;
            rhand = rhnd;
            lhand = lhnd;
            center = cntr;
        }

        frameoffsets_t(const frameoffsets_t & cp)
        {
            operator=(cp);
        }

        frameoffsets_t & operator=(const frameoffsets_t & cp)
        {
            head = cp.head;
            rhand = cp.rhand;
            lhand = cp.lhand;
            center = cp.center;
            return *this;
        }

        //Read from raw bytes
        template<typename _init>
            _init Read(_init itbeg, _init itend)
        {
            itbeg = head.read(itbeg, itend);
            itbeg = rhand.read(itbeg, itend);
            itbeg = lhand.read(itbeg, itend);
            itbeg = center.read(itbeg, itend);
            return itbeg;
        }

        template<typename _writerhelper_t>
            void Write(_writerhelper_t & sir0hlpr)const
        {
            head.write(sir0hlpr);
            rhand.write(sir0hlpr);
            lhand.write(sir0hlpr);
            center.write(sir0hlpr);
        }
    };

    /**********************************************************************
     * step_t (frame part)
     *      Part of an assembled frame. Contains details on where to place
     *      the part and if any special operation is ran on it.
     *      Contains OAM attribute data as it is stored in NDS memory.
    **********************************************************************/
    struct step_t
    {
        static const uint32_t FRAME_LEN          = 10;       //bytes, size of the structure in the file
        static const uint16_t ATTR0_FlagBitsMask = 0xFF00;   //1111 1111 0000 0000
        static const uint16_t ATTR1_FlagBitsMask = 0xFE00;   //1111 1110 0000 0000
        static const uint16_t ATTR2_FlagBitsMask = 0xFC00;   //1111 1100 0000 0000

        static const uint16_t ATTR0_ColPalMask   = 0x2000;   //0010 0000 0000 0000
        static const uint16_t ATTR0_MosaicMask   = 0x1000;   //0001 0000 0000 0000
        static const uint16_t ATTR0_ObjModeMask  = 0x0C00;   //0000 1100 0000 0000
        static const uint16_t ATTR0_DblSzDisabled= 0x0200;   //0000 0010 0000 0000 (Whether the obj is disabled if rot&scaling is off, or if double sized when rot&scaling is on!)
        static const uint16_t ATTR0_RotNScaleMask= 0x0100;   //0000 0001 0000 0000
        static const uint16_t ATTR0_YOffsetMask  = static_cast<uint16_t>(~ATTR0_FlagBitsMask);   //0000 0000 1111 1111

        static const uint16_t ATTR1_VFlipMask    = 0x2000;   //0010 0000 0000 0000
        static const uint16_t ATTR1_HFlipMask    = 0x1000;   //0001 0000 0000 0000
        static const uint16_t ATTR1_IsLastMask   = 0x0800;   //0000 1000 0000 0000
        static const uint16_t ATTR1_RotNScalePrm = 0x3E00;   //0011 1110 0000 0000
        static const uint16_t ATTR1_XOffsetMask  = static_cast<uint16_t>(~ATTR1_FlagBitsMask);   //0000 0001 1111 1111

        static const uint16_t ATTR2_PalNumberMask= 0xF000;   //1111 0000 0000 0000 //
        static const uint16_t ATTR2_PriorityMask = 0x0C00;   //0000 1100 0000 0000 //
        static const uint16_t ATTR2_TileNumMask  = 0x03FF;   //0000 0011 1111 1111 //Image tile id mask

        static const uint16_t  ATTR01_ResMask    = 0xC000;   //1100 0000 0000 0000 //Resolution enum value mask

        static const uint16_t  YOFFSET_MAX       = 255;     //256 possible values (-127 to 127)
        static const uint16_t  XOFFSET_MAX       = 511;     //512 possible values (-255 to 255)
        static const uint16_t  TILENUM_MAX       = 1023;    //1024 possible values
        static const uint8_t   PALID_MAX         = 15;      //16 palettes
        static const uint8_t   MAX_NB_PAL        = 16;      //16 palettes maximum
        static const uint8_t   NB_RNS_PARAM      = 32;      //32 possible values for the RnS param

        /*
         * eObjMode
        */
        //
        enum struct eObjMode: uint16_t
        {
            Normal      = 0,
            SemiTransp,
            Window,
            Bitmap,
            Invalid,
        };

        /*
         * ePriority
        */
        //Draw priority for the frame part
        enum struct ePriority: uint8_t
        {
            Highest = 0,
            High    = 1,
            Low     = 2,
            Lowest  = 3,
            NbLevels,
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
            _outit write(_outit where)const
        {
            where = utils::writeBytesFrom( frmidx,  where);
            where = utils::writeBytesFrom( unk0,    where);
            where = utils::writeBytesFrom( attr0,   where);
            where = utils::writeBytesFrom( attr1,   where);
            where = utils::writeBytesFrom( attr2,   where);
            return where;
        }

        /*
            helpr_t is a variant of SIR0_WriterHelper
        */
        template<class helpr_t>
            helpr_t & writeWHelpr(helpr_t & helpr)const
        {
            helpr.writeVal(frmidx);
            helpr.writeVal(unk0);
            helpr.writeVal(attr0);
            helpr.writeVal(attr1);
            helpr.writeVal(attr2);
            return helpr;
        }

        inline bool isReference()const              {return frmidx == static_cast<frmid_t>(-1);}
        inline bool isColorPal256()const            {return (ATTR0_ColPalMask & attr0) != 0;}
        inline bool isMosaicOn()const               {return (ATTR0_MosaicMask & attr0) != 0;}
        inline bool isDisabled()const               {return !isRotAndScalingOn() && ((ATTR0_DblSzDisabled & attr0) != 0);}
        inline bool isRnSRotCanvas()const             {return isRotAndScalingOn() && ((ATTR0_DblSzDisabled & attr0) != 0);} //Previously isDoubleSize
        inline bool isRotAndScalingOn()const        {return (ATTR0_RotNScaleMask & attr0) != 0;}
        inline uint16_t getYOffset()const           {return (ATTR0_YOffsetMask & attr0);}

        //Before checking VFlip and HFlip, make sure RnS isn't on!!!
        inline bool isVFlip()const                  {return (ATTR1_VFlipMask & attr1) != 0;}
        inline bool isHFlip()const                  {return (ATTR1_HFlipMask & attr1) != 0;}
        inline bool islast()const                   {return (ATTR1_IsLastMask & attr1) != 0;}

        inline eObjMode getObjMode()const           {return static_cast<eObjMode>((attr0 & ATTR0_ObjModeMask) >> 10);}
        inline uint8_t getRnSParam()const           {return static_cast<uint8_t>((ATTR1_RotNScalePrm & attr1) >> 9);}
        inline uint16_t getXOffset()const           {return (ATTR1_XOffsetMask & attr1);}

        inline uint8_t getPalNb()const              {return static_cast<uint8_t>((ATTR2_PalNumberMask & attr2) >> 12);}
        inline uint8_t getPriority()const           {return static_cast<uint8_t>((ATTR2_PriorityMask  & attr2) >> 10);}
        inline uint16_t getCharBlockNum()const           {return (ATTR2_TileNumMask & attr2);}
        inline eFrameRes getResolutionType()const   {return static_cast<eFrameRes>( ( (attr1 & ATTR01_ResMask) >> 14) | ( (attr0 & ATTR01_ResMask) >> 12 ) ); }

        inline frmid_t getFrameIndex()const         {return frmidx;}

        inline std::pair<uint16_t,uint16_t> GetResolution()const
        {
            uint8_t  flagresval = ((attr1 & ATTR01_ResMask) >> 14) | ( (attr0 & ATTR01_ResMask) >> 12 ) ; //Combine both into a number
            assert(flagresval < FrameResValues.size());

//            if(isDoubleSize())
//            {
//                auto res = FrameResValues[flagresval];
//                res.first  *= 2;
//                res.second *= 2;
//                return res;
//            }
//            else
                return FrameResValues[flagresval];
        }


        inline void setColorPal256  (bool bis256)   {attr0 = (bis256)? (ATTR0_ColPalMask | attr0) : (attr0 & ~ATTR0_ColPalMask);}
        inline void setMosaicOn     (bool bon)      {attr0 = (bon)? (ATTR0_MosaicMask | attr0) : (attr0 & ~ATTR0_MosaicMask);}
        inline void setObjMode      (eObjMode mode) {attr0 = (attr0 & ~ATTR0_ObjModeMask) | ((static_cast<uint16_t>(mode) & 0x3 ) << 10);}
        inline void setDisabled     (bool bon)      {attr0 = (bon)? (ATTR0_DblSzDisabled | attr0) : (attr0 & ~ATTR0_DblSzDisabled);}
        inline void setRnSCanvasRot (bool bon)      {setDisabled(bon);}
        inline void setRotAndScaling(bool bon)      {attr0 = (bon)? (ATTR0_RotNScaleMask | attr0) : (attr0 & ~ATTR0_RotNScaleMask);}
        inline void setYOffset      (uint16_t y)    {attr0 = (attr0 & ATTR0_FlagBitsMask) | (ATTR0_YOffsetMask & y);}

        inline void setVFlip        (bool bon)      {attr1 = (bon)? (ATTR1_VFlipMask | attr1) : (attr1 & ~ATTR1_VFlipMask);}
        inline void setHFlip        (bool bon)      {attr1 = (bon)? (ATTR1_HFlipMask | attr1) : (attr1 & ~ATTR1_HFlipMask);}
        inline void setLast         (bool blast)    {attr1 = (blast)? (ATTR1_IsLastMask | attr1) : (attr1 & ~ATTR1_IsLastMask);}
        inline void setRnSParam     (uint8_t param) {attr1 = (attr1 & ~ATTR1_RotNScalePrm) | ((param << 9) & ATTR1_RotNScalePrm);}
        inline void setXOffset      (uint16_t x)    {attr1 = (attr1 & ATTR1_FlagBitsMask) | (ATTR1_XOffsetMask & x);}

        inline void setPalNb        (uint8_t palnb) {attr2 = (attr2 & ~ATTR2_PalNumberMask) | (ATTR2_PalNumberMask & palnb);}
        inline void setPriority     (uint8_t prio)  {attr2 = (attr2 & ~ATTR2_PriorityMask) | (ATTR2_PriorityMask & prio);}
        inline void setCharBlockNum      (uint16_t tnum) {attr2 = (attr2 & ~ATTR2_TileNumMask) | (ATTR2_TileNumMask & tnum);}

        inline void setFrameIndex   (frmid_t id)    {frmidx = id;}

        inline void setResolutionType(eFrameRes res)
        {
            uint16_t flagval = static_cast<uint16_t>(res);
            attr1 = (attr1 & ~ATTR01_ResMask) | ((flagval << 14) & ATTR01_ResMask);
            attr0 = (attr0 & ~ATTR01_ResMask) | ((flagval << 12) & ATTR01_ResMask);
        }

        //Returns our size in nb of char blocks
        inline uint16_t calculateCharBlockSize()const
        {
            const auto res = GetResolution();
            const int  totalpixels = res.first * res.second;
            return static_cast<uint16_t>(totalpixels / NB_PIXELS_WAN_TILES);
        }
    };

    /**********************************************************************
     * animfrm_t
     *      Contains data on how a frame is used in an animation sequence.
     *      Makes up one animation frame of a sequence.
    **********************************************************************/
    struct animfrm_t
    {
        static const size_t LEN = 12;

        uint8_t duration;
        uint8_t flag;
        int16_t frmidx;
        int16_t xoffs;
        int16_t yoffs;
        int16_t shadowxoffs;
        int16_t shadowyoffs;

        animfrm_t()
            :duration(0), flag(0), frmidx(0), xoffs(0), yoffs(0), shadowxoffs(0), shadowyoffs(0)
        {
        }

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

        template<class HELPR_T>
            HELPR_T & write(HELPR_T & sir0hlpr)const //uses the sir0 helper to write data
        {
            sir0hlpr.writeVal(duration);
            sir0hlpr.writeVal(flag);
            sir0hlpr.writeVal(frmidx);
            sir0hlpr.writeVal(xoffs);
            sir0hlpr.writeVal(yoffs);
            sir0hlpr.writeVal(shadowxoffs);
            sir0hlpr.writeVal(shadowyoffs);
            return sir0hlpr;
        }
    };
};

#endif // RAW_SPRITE_DATA_HPP
