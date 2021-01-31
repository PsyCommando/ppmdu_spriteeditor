#ifndef SPRITE_HANDLER_HPP
#define SPRITE_HANDLER_HPP
#include <cstdint>
#include <vector>
#include <src/ppmdu/fmts/sprite/raw_sprite_data.hpp>
#include <src/ppmdu/fmts/sprite/sprite_content.hpp>

namespace fmt
{
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
        static const uint8_t PADDING_BYTE;
        typedef std::vector<frameoffsets_t> OffsetsDB;

        //Returns whether the sprite is valid or not
        bool ValidateContent()
        {
            //Check if the amount of frames match the amount of groups of frames offsets groups
            if(m_images.m_frames.size() != m_animtions.m_attachPoints.size())
            {
                assert(false);
                return false;
            }
            return true;
        }

        template<class _init>
            void Parse( _init itbeg, _init itend )
        {
            ParseHeaders(itbeg, itend);
            ParseImageInfo(itbeg, itend);
            ParseAnimInfoAndData(itbeg, itend); //anim must be after images
        }

        template<class _outit>
            _outit Write( _outit itout )
        {
            SIR0hdr                                 hdr;                    //SIR0 wrapper header that will be written later
            hdr_wan                                 wanhdr;
            std::vector<uint8_t>                    buffer;                 //Output buffer, contains content that will be written to file
            auto                                    itbackins = std::back_inserter(buffer);
            SIR0_WriterHelper<decltype(itbackins)>  sw(itbackins, hdr);     //Helper for handling marking pointer offsets automatically for the SIR0 wraper
            std::vector<uint32_t>                   imgptrs;                //Pointers to the individual compressed image entries, for building the img ptr table
            std::vector<uint32_t>                   frameptrs;              //Pointers to the individual frames entries for building the frame ptr table
            std::vector<uint32_t>                   ptrseqs;                //Pointers to individual animations sequences for building sequence ptr table
            std::vector<std::pair<uint32_t,size_t>> ptrgrps;                //Pointers to anim groups for assembling the anim group table
            uint32_t                                offspal = 0;            //Offset of the palette info chunk right after the palette data
            hdr_imgfmtinfo                          imginf  = m_imgfmt;     //img info chunk object
            hdr_animfmtinfo                         amiminf = m_animfmt;    //anim info chunk object

            //#1. Write frame assembly
            amiminf.maxnbusedblocks = m_images.calculateLargestFrameSize();
            assert(amiminf.maxnbusedblocks != 0 || (amiminf.maxnbusedblocks == 0 && m_images.m_frames.size() == 0)); //Only time char blocks usage should be 0, is when there are no frames
            m_images.WriteFrames(sw, frameptrs);

            //#2. Write animation sequences
            m_animtions.WriteSequences(sw, ptrseqs);

            //Need to pad anim sequences on 4 bytes
            sw.putPadding(4, PADDING_BYTE);

            //#3. Write compressed images
            m_images.WriteImages(sw, imgptrs);

            //#4. Write palette
            m_images.WritePalette(sw, offspal);
            imginf.ptrpal = offspal;                            //mark offset of palette data for imgfmt chunk

            //#5. Write frame pointer table
            amiminf.ptroamtbl = sw.getCurOffset();              //mark offset of frame table for animfmt chunk
            for(const auto & ptr : frameptrs)
                sw.writePtr(ptr);

            //#6. Write effect offset table(optional)
            if( !m_animtions.m_attachPoints.empty() )
            {
                amiminf.ptrattachtbl = sw.getCurOffset();          //mark offset of effect offset table for animfmt chunk
                m_animtions.WriteEffectsTbl(sw);
            }
            else
            {
                eSpriteType sprty = static_cast<eSpriteType>(m_wanhdr.spritety);
                //For some reasons when we're writing a character sprites, and there's no attachments, we still need to put the current offset for dummied-out sprites in m_attack.bin
                if(sprty == eSpriteType::Character)
                    amiminf.ptrattachtbl = sw.getCurOffset();
                else
                    amiminf.ptrattachtbl = 0;                          //Don't write an effect table at all in this case!
            }

            //#7. Write animation groups sequences array table
            m_animtions.WriteAnimGroups(sw, ptrseqs, ptrgrps);

            //#8. Write anim group table
            amiminf.ptranimtbl  = sw.getCurOffset();            //mark offset of anim table for animfmt chunk
            amiminf.nbanims     = m_animtions.m_animtbl.size(); //mark nb animations for animfmt chunk
            m_animtions.WriteAnimTbl(sw, ptrgrps);

            //#8. Write image pointer table
            imginf.ptrimgtable = sw.getCurOffset();             //mark chunk position for imgfmt chunk
            imginf.nbimgs      = m_images.m_images.size();      //mark nb images for imgfmt chunk
            for(const auto & ptr : imgptrs)
                sw.writePtr(ptr);

            //#9. Write anim info chunk
            wanhdr.ptraniminfo = sw.getCurOffset();                     //mark chunk position for wan header
            WriteAnimInfo(sw, amiminf);

            //#10.Write image info chunk
            wanhdr.ptrimgdatinfo = sw.getCurOffset();                      //mark chunk position for wan header
            WriteImageInfo(sw, imginf);

            //#11.Write wan header
            hdr.ptrsub = sw.getCurOffset();                     //mark header pos in sir0 header
            wanhdr.write(sw);

            //Padding to align the encoded ptr offset table on 16 bytes
            sw.putPadding(16, PADDING_BYTE);

            hdr.ptrtranslatetbl = sw.getCurOffset();            //mark ptr offset list pos in sir0 header

            //#12.Write out buffer with SIR0 header and pointerlist!
            //Generate and track SIR0 pointer list!!!
            return hdr.Write( itout, buffer.begin(), buffer.end() );
        }

        // ----------- Data Access -----------
        inline ImageDB::frm_t      & getFrame(frmid_t frmid)                   {return m_images.m_frames.at(frmid);}
        inline ImageDB::frmtbl_t   & getFrames()                               {return m_images.m_frames;}
        inline void                  setFrames(const ImageDB::frmtbl_t & frms) {m_images.m_frames = frms;}

        inline ImageDB::img_t      & getImage(size_t imgidx)                   {return m_images.m_images.at(imgidx);}
        inline ImageDB::imgtbl_t   & getImages()                               {return m_images.m_images;}
        inline void                  setImages(const ImageDB::imgtbl_t & imgs) {m_images.m_images = imgs;}

        inline rbgx24pal_t         & getPalette()                              {return m_images.m_pal.colors;}
        inline void                  setPalette(const rbgx24pal_t & pal)       {m_images.m_pal.colors = pal;}
        inline hdr_palettedata     & getPaletteData()                          {return m_images.m_pal;}

        inline AnimDB::animtbl_t   & getAnimationTable()                       {return m_animtions.m_animtbl;}
        inline void                  setAnimationTable(const AnimDB::animtbl_t & anit)  {m_animtions.m_animtbl = anit;}

        inline AnimDB::animgrp_t   & getAnimGroup(animgrpid_t id)      {return m_animtions.m_animgrps[id];}
        inline AnimDB::animgrptbl_t& getAnimGroups()                           {return m_animtions.m_animgrps;}
        inline void                  setAnimGroups(const AnimDB::animgrptbl_t & agrps)  {m_animtions.m_animgrps = agrps;}

        inline AnimDB::animseq_t   & getAnimSeq(animseqid_t id)        {return m_animtions.m_animsequences[id];}
        inline AnimDB::animseqtbl_t& getAnimSeqs()                             {return m_animtions.m_animsequences;}
        inline void                  setAnimSeqs(const AnimDB::animseqtbl_t& aseqs) {m_animtions.m_animsequences = aseqs;}

        inline OffsetsDB           & getEffectOffset()                         {return m_attachPoints;}
        inline void                  setEffectOffset(const OffsetsDB & attach) {m_attachPoints = attach;}

        inline const hdr_animfmtinfo   & getAnimFmtInfo()const                     {return m_animfmt;}
        inline hdr_animfmtinfo         & getAnimFmtInfo()                          {return m_animfmt;}
        inline const hdr_imgfmtinfo    & getImageFmtInfo()const                    {return m_imgfmt;}
        inline hdr_imgfmtinfo          & getImageFmtInfo()                         {return m_imgfmt;}

        inline eSpriteType           getSpriteType()const                      {return static_cast<eSpriteType>(m_wanhdr.spritety);}
        void                         setSpriteType(eSpriteType ty);

        inline const uint16_t      & getUnk12()const                           {return m_wanhdr.unk12;}
        inline uint16_t            & getUnk12()                                {return m_wanhdr.unk12;}

    private:
        //Sprite Header
        hdr_wan     m_wanhdr;

        //Info chunks
        hdr_imgfmtinfo  m_imgfmt;
        hdr_animfmtinfo m_animfmt;

        //Content
        ImageDB     m_images;
        AnimDB      m_animtions;
        OffsetsDB   m_attachPoints;

    private:
        template<class _init>
            void ParseHeaders(_init itbeg, _init itend )
        {
            auto itsrcbeg = itbeg;
            //#1. Parse SIR0 hdr
            fmt::SIR0hdr hdr;
            itbeg = hdr.Read(itbeg, itend);

            //#2. Parse Sprite hdr
            itbeg = m_wanhdr.read(std::next( itsrcbeg, hdr.ptrsub ), itend);
            eSpriteType sprty = static_cast<eSpriteType>(m_wanhdr.spritety);

            if(sprty >= eSpriteType::INVALID )
            {
                std::string error;
                std::stringstream sstr(error);
                sstr << "WA_SpriteHandler::Parse(): Invalid sprite type " << static_cast<unsigned int>(sprty) <<"!";
                throw std::runtime_error(error); //STOP HERE!!
            }

            if(m_wanhdr.ptraniminfo != 0)
                m_animfmt.read(std::next( itsrcbeg, m_wanhdr.ptraniminfo ), itend); //Parse anim info chunk

            if(m_wanhdr.ptrimgdatinfo != 0) //Effects often have no image data
                m_imgfmt.read(std::next( itsrcbeg, m_wanhdr.ptrimgdatinfo ), itend);
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

            //Parse attachment table if its there
            if(m_animfmt.ptrattachtbl != 0)
            {
                eSpriteType sprty = static_cast<eSpriteType>(m_wanhdr.spritety);
                if(sprty != eSpriteType::Character)
                    assert(false);
                m_animtions.ParseAttachmentOffsets(itbeg, itend, m_animfmt, m_images.getNbFrames());
                m_attachPoints = m_animtions.m_attachPoints;
            }
        }

        template<class _writerhelper_t> void WriteAnimInfo(_writerhelper_t & sir0hlpr, const hdr_animfmtinfo & amiminf)
        {
            //#TODO: Ideally make a last check here and make sure everything is consistant!

            //curoffs = amiminf.MarkPointers(uint32_t(curoffs), ptroffsets);
            amiminf.write(sir0hlpr);
            //return where;
        }

        template<class _writerhelper_t> void WriteImageInfo(_writerhelper_t & sir0hlpr, const hdr_imgfmtinfo & imginf)
        {
            //#TODO: Ideally make a last check here and make sure everything is consistant!

            //curoffs = imginf.MarkPointers(uint32_t(curoffs), ptroffsets);
            imginf.write(sir0hlpr);
            //return where;
        }

    };

};

#endif // SPRITE_HANDLER_HPP
