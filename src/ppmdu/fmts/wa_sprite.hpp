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
    //Palette
    typedef uint32_t rgbx_t;

    /*
     *
     * Possible formats that a wa_ sprite might take.
    */
    enum struct eSpriteType : uint16_t
    {
        Prop        = 0,
        Character   = 1,
        Effect      = 2,
        WAT         = 3,
        INVALID,
    };

    /*
    */
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
    };

    /*
    */
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
    };





    /**/
    struct ImageDB
    {
        static const uint16_t FLAGS1_FlagBitsMask   = 0xFC00;   //1111 1110 0000 0000

        static const uint16_t FLAGS2_FlagBitsMask   = 0xFE00;   //1111 1100 0000 0000

        static const uint8_t  FLAG2_IsLastMask      = 0x8;      //0000 1000

        /*
        */
        struct palettedata
        {
            std::vector<rgbx_t> colors;
            //uint32_t ptrpalbeg; //
            uint16_t unk3;      //force4bb
            uint16_t nbcol;
            uint16_t unk4;
            uint16_t unk5;
            //4bytes of zeroes here
        };

        typedef std::vector<uint8_t> img_t;
        typedef int16_t              frmid_t; //-1 is a nodraw frame
        struct step_t
        {
            frmid_t  frmidx;
            uint16_t unk0;
            int16_t  yoffset;
            uint8_t  flags1;
            int16_t  xoffset;
            uint8_t  flags2;
            uint8_t  unk15;
            uint8_t  unk1;

            template<class _init>
                _init read(_init beg, _init end)
            {
                beg = utils::readBytesAs( beg, end, frmidx );
                beg = utils::readBytesAs( beg, end, unk0 );

                uint16_t yofftmp = utils::readBytesAs<uint16_t>(beg, end);
                flags1  = static_cast<uint8_t>((FLAGS1_FlagBitsMask & yofftmp) >> 8);
                yoffset = (~FLAGS1_FlagBitsMask) & yofftmp;

                uint16_t xofftmp = utils::readBytesAs<uint16_t>(beg, end);
                flags2 = static_cast<uint8_t>((FLAGS2_FlagBitsMask & xofftmp) >> 8);

                beg = utils::readBytesAs( beg, end, unk15 );
                beg = utils::readBytesAs( beg, end, unk1 );
                return beg;
            }

            inline bool islast()const
            {
                return (FLAG2_IsLastMask & flags2) != 0;
            }
        };
        typedef std::list<step_t> frm_t;

        std::vector<img_t> m_images;    //contains decoded raw linear images
        std::vector<frm_t> m_frames;    //References on how to assemble raw images into individual animation frames.
        palettedata        m_pal;

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
                if(!curstep.islast())
                    curfrm.push_back(move(curstep));
            }while( !curstep.islast() );

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

    /**/
    struct AnimDB
    {
        /*
        */
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
        };

        typedef int                             animseqid_t;    //Id for an animation sequence
        typedef int                             animgrpid_t;    //Id for an animation group
        typedef std::list<animfrm_t>            animseq_t;      //A single sequence of animated frames
        static const animseqid_t NullSeq = -1;  //Represents an entry not refering to any sequence
        static const animgrpid_t NullGrp = -1;  //Represents an entry not refering to any group

        /**/
        struct animgrp_t //A group is a set of animation sequences
        {
            std::vector<animseqid_t> seqs;
            uint16_t                 unk16;
        };

        std::unordered_map<animseqid_t, animseq_t>  m_animsequences;    //A table containing all uniques animation sequences
        std::unordered_map<animgrpid_t, animgrp_t>  m_animgrps;         //A table of all the unique animation groups
        std::vector<animgrpid_t>                    m_animtbl;          //A table of pointers to the animation group associated to an animation



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

    /*
    */
    struct effectoffset
    {
        uint16_t xoff = 0;
        uint16_t yoff = 0;
    };

    /*
     *
     * This class is used as a sort of in-between between the file representation and in-memory representation of a wa_ sprite.
     * Basically it contains the raw data of a sprite, and provides ways to translates those to and from.
    */
    class WA_SpriteHandler
    {
    public:
        typedef std::vector<effectoffset> OffsetsDB;

        template<class _init>
            _init Parse( _init itbeg, _init itend )
        {
            auto itsrcbeg = itbeg;

            //#1. Parse SIR0 hdr
            fmt::SIR0hdr hdr;
            itbeg = hdr.Read(itbeg, itend);

            //#2. Parse Sprite hdr
            itbeg = std::next( itsrcbeg, hdr.ptrsub );
            uint32_t ptraniminfo = utils::readBytesAs<uint32_t>( itbeg, itend );
            uint32_t ptrimginfo  = utils::readBytesAs<uint32_t>( itbeg, itend );
            m_sprty = static_cast<eSpriteType>(utils::readBytesAs<uint16_t>( itbeg, itend ));
            assert( m_sprty < eSpriteType::INVALID );
            itbeg = utils::readBytesAs( itbeg, itend, m_unk12 );

            //Parse the fmt subsections
            if( ptraniminfo != 0 )
            {
                itbeg = std::next( itsrcbeg, ptraniminfo );
                itbeg = m_animfmt.read(itbeg, itend);
            }

            if( ptrimginfo != 0 )
            {
                itbeg = std::next( itsrcbeg, ptrimginfo );
                m_imgfmt.read(itbeg, itend);
            }

            //#3. Build Frame Table
            m_images.ParseImgData(itsrcbeg, itend, m_imgfmt, m_animfmt);

            //#4. Load Image Data And Palette

            //#5. Build Animation Table
            m_animtions.ParseAnimTbl(itsrcbeg, itend,m_animfmt);

            //#6. Build Effect Offset Table
            if(m_animfmt.ptrefxtbl != 0)
            {
                //#TODO
            }

            return itbeg;
        }

        template<class _outit>
            _outit Write( _outit itout )
        {
            assert(false);
            //Generate and track SIR0 pointer list!!!
            return itout;
        }

        // ----------- Data Access -----------

    //private:

        //pointer to the sprite header
        //uint32_t    m_ptrsprhdr;

        //Sprite Header
        //uint32_t    m_ptraniminfo;
        //uint32_t    m_ptrimginfo;
        eSpriteType m_sprty;
        uint16_t    m_unk12;

        //
        imgfmtinfo  m_imgfmt;
        animfmtinfo m_animfmt;

        //Content
        ImageDB     m_images;
        AnimDB      m_animtions;
        OffsetsDB   m_efxoffsets;
    };
};

#endif // WA_SPRITE_HPP
