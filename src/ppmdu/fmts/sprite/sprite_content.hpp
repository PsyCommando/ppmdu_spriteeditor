#ifndef SPRITE_CONTENT_HPP
#define SPRITE_CONTENT_HPP
#include <vector>
#include <list>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <src/ppmdu/fmts/sprite/shared_sprite_constants.hpp>
#include <src/ppmdu/fmts/sprite/raw_sprite_data.hpp>

namespace fmt
{
//-----------------------------------------------------------------------------
//  ImageDB
//-----------------------------------------------------------------------------
    /**********************************************************************
     * Handles parsing/writing/containing everything related to image data
     * from a sprite!
    **********************************************************************/
    struct ImageDB
    {
        static const int FRAME_TILE_SZ_BYTES; //Size of a frame in the loaded image data. Fixed, since we force 8bpp during import
        static const int FRAME_TILE_SZ_PIXELS; //Size of a single tile loaded in pixel
        struct img_t
        {
            std::vector<uint8_t> data;
            uint16_t             unk2;
            uint16_t             unk14;
        };
        typedef std::list<step_t>       frm_t;      //"Meta-Frame", a sequence of oam entries to assemble an image
        typedef std::vector<img_t>      imgtbl_t;
        typedef std::vector<frm_t>      frmtbl_t;
        imgtbl_t        m_images;    //contains decoded raw linear images
        frmtbl_t        m_frames;    //References on how to assemble raw images into individual animation frames.
        hdr_palettedata m_pal;


        /*
         *  imgstriptblentry
         *      Entry in an image's zero-strip table!
        */
        struct imgstriptblentry
        {
            static const size_t ENTRY_LEN = 12;
            uint32_t src;
            uint16_t len;
            uint16_t unk14;
            uint32_t unk2;

            imgstriptblentry( uint32_t imsrc = 0, uint16_t imlen = 0, uint16_t imunk14 = 0, uint32_t imunk2 = 0 )
                :src(imsrc), len(imlen), unk14(imunk14), unk2(imunk2)
            {}

            template<class outit> outit write(outit where)const
            {
                where = utils::writeBytesFrom(src, where);
                where = utils::writeBytesFrom(len, where);
                where = utils::writeBytesFrom(unk14, where);
                where = utils::writeBytesFrom(unk2, where);
                return where;
            }

            template<class HELPR_T> HELPR_T & writeWHlpr( HELPR_T & sir0hlpr )const
            {
                sir0hlpr.writePtr(src); //If its 0 it won't mark the offset, so no need to put a conditional here!
                sir0hlpr.writeVal(len);
                sir0hlpr.writeVal(unk14);
                sir0hlpr.writeVal(unk2);
                return sir0hlpr;
            }


            template<class init> init read(init beg, init end)
            {
                beg = utils::readBytesAs( beg, end, src  );
                beg = utils::readBytesAs( beg, end, len  );
                beg = utils::readBytesAs( beg, end, unk14);
                beg = utils::readBytesAs( beg, end, unk2 );
                return beg;
            }

            inline bool isnull()const
            {
                return src == 0 && len == 0 /*&& unk14 == 0 && unk2 == 0*/;
            }
        };

        /*
            imgEncoder
                Encode an image to the format in the WAN sprites.
                AKA it removes tiles full of zeros.
        */
        template<class HELPR_T>
            class imgEncoder
        {
            typedef HELPR_T             helpr_t;
            helpr_t                    *sir0hlpr;
            std::list<imgstriptblentry> asmtbl;
            std::vector<uint8_t>        stripsdat;
            uint32_t                    totallen;
            uint32_t                    baseoffset;
            uint16_t                    unk2;
            uint16_t                    unk14;
            std::vector<uint32_t>       &imgptrs;

        public:

            imgEncoder(HELPR_T & helpr, std::vector<uint32_t> &imgpointers)
                :sir0hlpr(&helpr), imgptrs(imgpointers)
            {}

            /*
                operator()
                    Encode the image specified!
                    Returns the total length of the image block written!
            */
            template<class _init>
                uint32_t operator()(_init imgbeg,
                                    _init imgend,
                                    uint16_t _unk2,
                                    uint16_t _unk14)
            {
                asmtbl.resize(0);
                stripsdat.resize(0);
                totallen = 0;
                baseoffset = sir0hlpr->getCurOffset();
                unk2  = _unk2;
                unk14 = _unk14;
                auto strips = MakeStrips(imgbeg, imgend);
                BuildTable(strips);
                WriteEncoded();
                return totallen;
            }

        private:

            struct stripkind
            {
                std::vector<uint8_t> stripdata;
                bool                 iszeros;
                uint32_t             src;

                template<class _HelprT> void writedata(_HelprT & helper)const
                {
                    for(uint8_t by : stripdata)
                        helper.writeVal(by);
                }
            };

            /*
                MakeStrips
                    Since the assembly table lengths are divisible by 32 (aka 64 pixels or 1, 8x8 tile)
                    we cut it up into strips right away, and then we check which ones are consecutively 0 or not
                    and merge their entries in the assembly table!.
            */
            template<class _init>
                std::vector<stripkind> MakeStrips(_init imgbeg, _init imgend)
            {
                static const size_t STRIPLEN = 32; //0x20 bytes
                std::vector<stripkind> outstrips;
                size_t totalby = 0;
                while(imgbeg != imgend)
                {
                    stripkind curstrip;
                    curstrip.stripdata.resize(STRIPLEN,0);
                    curstrip.iszeros = true;
                    for(size_t cntpix = 0; (cntpix < STRIPLEN) && (imgbeg != imgend); ++imgbeg, ++cntpix)
                    {
                        curstrip.stripdata[cntpix] = *imgbeg;
                        if( curstrip.iszeros && curstrip.stripdata[cntpix] != 0)
                        {
                            //If we had all zeros so far and we found a non-zero byte, make this a non-zero stripe!
                            curstrip.iszeros = false;
                        }
                    }

                    if( curstrip.iszeros )
                        curstrip.src = 0;
                    else
                        curstrip.src = totalby;

                    totalby += STRIPLEN;
                    outstrips.push_back(std::move(curstrip));
                }
                return std::move(outstrips);
            }

            void BuildTable(const std::vector<stripkind> & strips)
            {
                auto beg = strips.begin();
                auto end = strips.end();
                while(beg != end)
                {
                    bool searchforzeros = beg->iszeros; //Set whether we're looking for adjacents strips of 0s or strips of non-zeros
                    imgstriptblentry entry;
                    entry.src = (searchforzeros)? 0 : (totallen + baseoffset);

                    //Combine adjacent entries that are the same into one entry
                    for(; (beg != end) && (beg->iszeros == searchforzeros); ++beg )
                    {
                        entry.len   += beg->stripdata.size();
                        entry.unk2  =  unk2;
                        entry.unk14 =  unk14;
                        //if we're not looking for a strip of zeros, also put the image strips into the output
                        if( !searchforzeros )
                        {
                            beg->writedata(*sir0hlpr);
                            totallen += beg->stripdata.size();
                        }
                    }


                    asmtbl.push_back(std::move(entry)); //Add the assembled entry to the list
                }
            }

             void WriteEncoded()
             {
                //write the table!
                imgptrs.push_back(totallen + baseoffset); //mark the begining of the assembly table for this image!
                for( const auto & astrip : asmtbl )
                {
                    astrip.writeWHlpr(*sir0hlpr);
                    totallen += imgstriptblentry::ENTRY_LEN;
                }
                //write the last null entry!
                imgstriptblentry().writeWHlpr(*sir0hlpr);
                totallen += imgstriptblentry::ENTRY_LEN;
             }
        };

        //_writerhelper_t should be a SIR0_WriterHelper
        template<class _writerhelper_t>
            void WriteFrames( _writerhelper_t & hlpr, std::vector<uint32_t> & pointers )const
        {
            for(const frm_t & frm : m_frames)
            {
                pointers.push_back(hlpr.getCurOffset()); //Mark the start of the frame
                //Make sure last step is set right
                int nbparts = frm.size();
                int cntfrm  = 0;

                for(const step_t & step : frm)
                {
                    if(cntfrm == (nbparts - 1))
                    {
                        step_t stepcpy = step;
                        stepcpy.setLast(true);          //Mark the last frame
                        stepcpy.writeWHelpr(hlpr);
                    }
                    else
                        step.writeWHelpr(hlpr);
                    ++cntfrm;
                }
            }
        }

        //_writerhelper_t should be a SIR0_WriterHelper
        template<class _writerhelper_t>
            void WriteImages( _writerhelper_t & hlpr, std::vector<uint32_t> & imgptrs )const
        {
            imgEncoder<_writerhelper_t> encoder(hlpr, imgptrs);
            for(const img_t & img : m_images)
                encoder(img.data.begin(), img.data.end(), img.unk2, img.unk14 );
        }

        template<class _writerhelper_t>
            void WritePalette( _writerhelper_t & hlpr, uint32_t & offsetpalette )const
        {
            uint32_t palbeg = hlpr.getCurOffset();

            //write colors!
            for( const rgbx_t & color : m_pal.colors )
                hlpr.writeVal(color,false);
            const uint16_t nbcolors = static_cast<uint16_t>(m_pal.colors.size());

            offsetpalette = hlpr.getCurOffset();
            hlpr.writePtr(palbeg);
            hlpr.writeVal(m_pal.unk3);
            if(m_images.empty())
                hlpr.writeVal(static_cast<uint16_t>(0)); //The original files set this to 0 if there's no image, even though it wrote the color table.
            else
                hlpr.writeVal(nbcolors);
            hlpr.writeVal(m_pal.unk4);
            hlpr.writeVal(m_pal.unk5);
            hlpr.writeVal(static_cast<uint32_t>(0));
        }


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

            if(utils::readBytesAs<uint32_t>( itpaldata, itsrcend ) != 0)
                throw std::runtime_error("ImageDB::ParsePalette(): Palette data doesn't end with 4 bytes of zeroes!");

            //grab colors!
            auto itpalcolor = next(itsrcbeg, ptrbegpal);
            size_t pallen = (paloff - ptrbegpal) / sizeof(uint32_t);
            for( size_t cntcol = 0; cntcol < pallen; ++cntcol )
            {
                rgbx_t curcol;
                itpalcolor = utils::readBytesAs( itpalcolor, itsrcend, curcol, false );
                m_pal.colors.push_back( curcol );
            }
        }

        template<class _init>
            void ParseAFrame(_init itsrcbeg, _init itsrcend, uint32_t frmoff, uint32_t nextfrmoff)
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
                frmoff+= step_t::FRAME_LEN;
            }while( (!curstep.islast() || frmoff < nextfrmoff ) && //Added extra check to make sure if the "islast" flag isn't set we won't read the whole file..
                    itsrcbeg != itsrcend );
            m_frames.push_back(std::move(curfrm));
        }

        template<class _init>
            void ParseAnImage(_init itsrcbeg, _init itsrcend, uint32_t imgoff)
        {
            using namespace std;

            if(imgoff == 0)
                return;

            //Read image assembly table
            // Basically each entry tells us how to assemble the image,
            // step by step. Each steps tells us whether we write zeros, or a pixel strip from the file.
            auto itcmptable = next(itsrcbeg, imgoff);
            img_t curimg;
            uint16_t len = 0;
            do
            {
                uint32_t pixsrc = utils::readBytesAs<uint32_t>(itcmptable, itsrcend);
                len             = utils::readBytesAs<uint16_t>(itcmptable, itsrcend);

                //NOTE: Break here so we don't end up changing unk14 and unk2 to zero for the entire image
                if(pixsrc == 0 && len == 0)
                    break;

                curimg.unk14    = utils::readBytesAs<uint16_t>(itcmptable, itsrcend);
                curimg.unk2     = utils::readBytesAs<uint32_t>(itcmptable, itsrcend);

                if( pixsrc == 0 )
                    fill_n( back_inserter(curimg.data), len, 0 );
                else
                {
                    auto itimg = next(itsrcbeg, pixsrc);
                    copy( itimg, next(itimg,len), back_inserter(curimg.data) );
                }

            }while( len != 0 ); //null entry is last
            m_images.push_back(move(curimg));
        }

        template<class _init>
            void ParseImgData(_init itsrcbeg, _init itsrcend, const hdr_imgfmtinfo & imginf, const hdr_animfmtinfo & animinf)
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
                uint32_t endFrmTable = CalculateFrameRefTblEnd(itsrcbeg, itsrcend, animinf);
                uint32_t endFrmParts = CalculateFramePartsEnd(itsrcbeg, itsrcend, animinf, imginf);
                assert(endFrmTable != 0);   //Shouldn't happen
                assert(endFrmParts != 0);   //Shouldn't happen

                if(endFrmTable == 0 || endFrmParts == 0)
                    throw std::runtime_error("ImageDB::ParseImgData(): End of frame parts, or frame refs table not found! Can't validate.");

                //Grab the frames
                auto itfrmtbl = next(itsrcbeg, animinf.ptroamtbl);
                const size_t nbfrm  = (endFrmTable - animinf.ptroamtbl) / sizeof(uint32_t);

                for( size_t cntfrm = 0; cntfrm < nbfrm && itfrmtbl != itsrcend; ++cntfrm )
                {
                    //Grab the current and peek at the next one!
                    uint32_t curptr  = utils::readBytesAs<uint32_t>(itfrmtbl, itsrcend);
                    uint32_t nextptr = 0;

                    if( itfrmtbl != itsrcend && (cntfrm + 1) < nbfrm )
                        utils::readBytesAs<uint32_t>(itfrmtbl, itsrcend, nextptr); //Don't assign the iterator, so we actually process this next turn
                    else
                        nextptr = endFrmParts; //If we're at the last entry, just use the end of the table as nextptr

                    if( curptr != 0 )
                        ParseAFrame(itsrcbeg, itsrcend, curptr, nextptr);
                }
            }
        }

        /*
            GetFirstNonNullAnimSequenceRefTblEntry
                Search the animation table for an animation linking to the first animation sequence pointer in the animation sequence ref table, and returns it.
                If it can't find one it'll return 0.
        */
        template<class _init>
            uint32_t GetFirstNonNullAnimSequenceRefTblEntry(_init itsrcbeg, _init itsrcend, const hdr_animfmtinfo & animinf)
        {
            //In this case, we have to improvise and find the start of the Animation Sequence table.
            auto itanim = std::next(itsrcbeg, animinf.ptranimtbl);
            //Find the first non-null animation
            for( size_t cntanim = 0; cntanim < animinf.nbanims && itanim != itsrcend; ++cntanim )
            {
                uint32_t seqlistptr = utils::readBytesAs<uint32_t>(itanim, itsrcend);
                if(seqlistptr != 0)
                    return seqlistptr;  //Return the offset of the first sequence list we have. That's the start of the table!
                std::advance(itanim, 4); //skip the rest of the entry
            }
            return 0; //This can happen sometimes in m_attack.bin sprites
        }

        /*
            CalculateFrameRefTblEnd
                Try to find the offset of the end of the Frame references table.
                Usually, its the offset of the effect offset table, but it works only for character sprites.
                If it can't get the effect offset table's position, it'll try to find the start of the next
                chunk after, aka the animation sequence ref table.
                If it can't find that either, it'll return the start of the animation table.
        */
        template<class _init>
            uint32_t CalculateFrameRefTblEnd(_init itsrcbeg, _init itsrcend, const hdr_animfmtinfo & animinf)
        {
            if( animinf.ptrattachtbl != 0 )
                return animinf.ptrattachtbl;
            else
            {
                uint32_t seqref = GetFirstNonNullAnimSequenceRefTblEntry(itsrcbeg, itsrcend, animinf);
                if(seqref != 0)
                    return seqref;
            }
            //If everything else fails, return the offset of the anim table
            return animinf.ptranimtbl;
        }

        /*
            CalculateFramePartsEnd
                Find the offset of the end of the frame part lists chunk.
                Its usually the start of the animation frames lists chunk.
                If it can't find that, it'll return 0!
        */
        template<class _init>
            uint32_t CalculateFramePartsEnd(_init itsrcbeg, _init itsrcend, const hdr_animfmtinfo & animinf, const hdr_imgfmtinfo & imginf)
        {
            //We want the address of the first animation sequence's frame entry
            uint32_t endfrmparts = 0;
            uint32_t seqref = GetFirstNonNullAnimSequenceRefTblEntry(itsrcbeg, itsrcend, animinf);
            if(seqref != 0)
            {
                auto itseqref = std::next(itsrcbeg, seqref);
                if(itseqref != itsrcend)
                {
                    endfrmparts = utils::readBytesAs<uint32_t>(itseqref, itsrcend);
                }
            }
            else
            {
                //Otherwise we'd want to get the start of the compressed images block..
                endfrmparts = getStartOffsetImageData(itsrcbeg, itsrcend, imginf);
            }

            //If we still can't get the end, the next block is the palette
            if(endfrmparts == 0)
            {
                auto itpal = std::next(itsrcbeg, imginf.ptrpal);
                uint32_t ptrpaldat = utils::readBytesAs<uint32_t>(itpal, itsrcend); //The first value at the palette pointer is the beginning of palette data
                endfrmparts = ptrpaldat;
            }
            assert(endfrmparts != 0);
            return endfrmparts;
        }

        template<class _init>
            uint32_t getStartOffsetImageData(_init itsrcbeg, _init itsrcend, const hdr_imgfmtinfo & imginf)
        {
            uint32_t ofsimgdat = 0;
            auto itimgtbl = std::next(itsrcbeg, imginf.ptrimgtable);
            for(size_t cntimg = 0; cntimg < imginf.nbimgs; ++cntimg)
            {
                uint32_t ptrimgdat = utils::readBytesAs<uint32_t>(itimgtbl, itsrcend);
                if(ofsimgdat == 0 || ptrimgdat < ofsimgdat)
                    ofsimgdat = ptrimgdat;
            }
            return ofsimgdat;
        }

        uint32_t getNbFrames()const
        {
            return m_frames.size();
        }

        uint16_t calculateLargestFrameSize()const //As nb of char blocks
        {
            uint16_t largest = 0; // the largest amount of char blocks used out of all frames
            for(const frm_t & frm : m_frames)
            {
                uint16_t curfrmtotalsz = 0; //highest char block number + size used out of all steps
                for(const step_t & stp : frm)
                {
                    const uint16_t curmax = stp.getCharBlockNum() + stp.calculateCharBlockSize();
                    if(curmax > curfrmtotalsz)
                        curfrmtotalsz = curmax;
                }
                if(curfrmtotalsz > largest)
                    largest = curfrmtotalsz;
            }
            return largest;
        }
    };

//-----------------------------------------------------------------------------
//  AnimDB
//-----------------------------------------------------------------------------
    typedef int animseqid_t;   //Id for an animation sequence
    typedef int animgrpid_t;   //Id for an animation group
    extern const animseqid_t NullSeqIndex;  //Represents an entry not refering to any sequence
    extern const animgrpid_t NullGrpIndex;  //Represents an entry not refering to any group
    /**********************************************************************
     * Sub component of a sprite for handling animation data.
    **********************************************************************/
    class AnimDB
    {
    public:
        //A single sequence of animated frames

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
        typedef std::map<animgrpid_t, animgrp_t>    animgrptbl_t;
        typedef std::map<animseqid_t, animseq_t>    animseqtbl_t;
        typedef std::vector<frameoffsets_t>         frmoffslst_t;

        animseqtbl_t  m_animsequences;  //A table containing all uniques animation sequences
        animgrptbl_t  m_animgrps;       //A table of all the unique animation groups
        animtbl_t     m_animtbl;        //A table of pointers to the animation group associated to an animation
        frmoffslst_t  m_attachPoints;     //A table of 16bits X,Y coordinates indicating where on the sprite some effects are drawn! In groups of 4, one group for each "frame".

    public:
        template<class _writerhelper_t> void WriteEffectsTbl(_writerhelper_t & sir0hlpr)const
        {
            for( const frameoffsets_t & ofs : m_attachPoints )
                ofs.Write(sir0hlpr);
        }

        template<class _writerhelper_t> void WriteSequences( _writerhelper_t & sir0hlpr, std::vector<uint32_t> & ptrseqs )const
        {
            for( const auto & seq : m_animsequences )
            {
                ptrseqs.push_back(sir0hlpr.getCurOffset());
                for(const animfrm_t & frm : seq.second)
                    frm.write(sir0hlpr);
                animfrm_t().write(sir0hlpr); //null frame at the end
            }
        }

        template<class _writerhelper_t> void WriteAnimGroups( _writerhelper_t                           & sir0hlpr,
                                                              const std::vector<uint32_t>               & ptrseqs,
                                                              std::vector<std::pair<uint32_t, size_t>>  & ptrgrpslst)const
        {
            /*We have 2 tables to write here!
             * First we have
             *
             *  vector<vector<uint32_t>> groupssequenceptr;
             *
             * which stores the list of sequences for each individual groups
             *
             * And then, we got pointers to those groups:
             *  struct grpentry
             * {
             *  uint32_t grpptr;
             *  uint16_t grplen;
             *  uint16_t unk16;
             * };
             *
             * grpentry groups[];
            */

            for(size_t idxanim = 0; idxanim <  m_animtbl.size(); ++idxanim )
            {
                if(m_animtbl[idxanim] == NullGrpIndex)
                    sir0hlpr.writePtr(static_cast<uint32_t>(0));
                else
                {
                    const auto & grp = m_animgrps.at(m_animtbl[idxanim]);
                    //Then we can put the data for our valid group!
                    ptrgrpslst.push_back(std::make_pair( sir0hlpr.getCurOffset(),
                                                         grp.seqs.size())); //add to the list of group array pointers + sizes
                    for( const auto & seq : grp.seqs )
                    {
                        if(seq >= static_cast<int>(ptrseqs.size()))
                            throw std::out_of_range("AnimDB::WriteAnimGroups(): Sequence ID is out of bound!!");
                        sir0hlpr.writePtr(ptrseqs.at(seq)); //mark the pointer position for the SIR0 later!
                    }
                }
            }
        }

        template<class _writerhelper_t> void WriteAnimTbl( _writerhelper_t                                 & sir0hlpr,
                                                           const std::vector<std::pair<uint32_t, size_t>>  & ptrgrps)const
        {
            //#FIXME : Anim table works differently if the sprite is or isn't a character sprite!!!!
            for( animgrpid_t grpid : m_animtbl )
            {
                uint32_t ptr    = 0;
                uint16_t len    = 0;
                uint16_t unk16  = 0;
                //Check if is empty group
                if(grpid != -1)
                {
                    //Check if the grpid is valid to detect any issues with the algorithm itself
                    if( (grpid >= static_cast<int>(ptrgrps.size()) || grpid >= static_cast<int>(m_animgrps.size())) )
                    {
                        assert(false);
                        std::stringstream sstr;
                        sstr << "AnimDB::WriteAnimTbl(): Group ID " <<grpid <<" is out of bound!!";
                        throw std::out_of_range(sstr.str());
                    }
                    else
                    {
                        const auto & refgrp = ptrgrps.at(grpid);
                        ptr = refgrp.first;
                        len = static_cast<uint16_t>(refgrp.second);
                        unk16 = m_animgrps.at(grpid).unk16;
                    }
                }
                //Empty groups just have all set to 0
                sir0hlpr.writePtr(ptr);     //write ptr to group's sequence array in the group table!
                sir0hlpr.writeVal(len);     //write the size of the group's sequence array in the group table!
                sir0hlpr.writeVal(unk16);   //write the unk16 value for the current group!
            }
        }


        template<class _init>
            animseqid_t ParseAnimSeq( std::unordered_map<uint32_t,animgrpid_t> & animseqreftable,
                                      _init itsrcbeg,
                                      _init itsrcend,
                                      uint32_t seqoffset )
        {
            using namespace std;
            if(seqoffset == 0)
                return NullSeqIndex;

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
                return NullGrpIndex;

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
            void ParseAnimTbl(_init itsrcbeg, _init itsrcend, const hdr_animfmtinfo & animinf)
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

        template<class _init>
            void ParseAttachmentOffsets(_init itsrcbeg, _init itsrcend, const hdr_animfmtinfo & animinf, uint32_t nbframes)
        {
            if(animinf.ptrattachtbl == 0)
                return;
            assert(itsrcbeg != itsrcend);

            const uint32_t lenAttachmentsSet = nbframes * frameoffsets_t::LEN;
            const uint32_t ofsEndAttchments = animinf.ptrattachtbl + lenAttachmentsSet;

            //#NOTE: Its possible that on other sprite types this pointer is used for something else!

            auto tblbeg = std::next(itsrcbeg, animinf.ptrattachtbl);
            auto tblend = std::next(itsrcbeg, ofsEndAttchments);

            for( ; tblbeg != tblend; )
            {
                assert(tblbeg != itsrcend);
                frameoffsets_t ofs;
                tblbeg = ofs.Read(tblbeg, tblend);
                m_attachPoints.push_back(ofs);
            }
        }

    private:
        animgrpid_t curgrpid;
        animseqid_t curseqid;
    };

};

#endif // SPRITE_CONTENT_HPP
