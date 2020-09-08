#ifndef ANIMFRAME_HPP
#define ANIMFRAME_HPP
#include <QTableWidget>
#include <QList>
#include <QVector>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>
#include <QStyledItemDelegate>
#include <QStandardItemModel>

#include <src/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>


class Sprite;
class MFrame;
extern const char * ElemName_AnimFrame;

//*******************************************************************
//  AnimFrame
//*******************************************************************
class AnimFrame :  public BaseTreeTerminalChild<&ElemName_AnimFrame>
{
    static QSize calcTextSize(const QString &str);
public:
    static const char *         UProp_AnimFrameID;  //UserProp name used for storing the id of this frame!
    static const QStringList    ColumnNames;        //Name displayed in the column header for each properties of the frame! Is tied to eColumnsType

    //NOTE: Be sure to update ColumnNames when changing this!
    enum struct eColumnsType : int
    {
        Frame = 0,
        Duration,
        Offset,
        ShadowOffset,
        Flags,
        NBColumns,
        //Everything below this is not displayed as header column

        //To acces some of the merged data individually via model data! Since we merged both x/y param entry into a single one for each categories
        Direct_XOffset,
        Direct_YOffset,
        Direct_ShadowXOffset,
        Direct_ShadowYOffset,
    };


public:
    AnimFrame( TreeElement * parent )
        :BaseTreeTerminalChild(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags())
    {
        setNodeDataTy(eTreeElemDataType::animFrame);
    }

    void clone(const TreeElement *other)
    {
        const AnimFrame * ptr = static_cast<const AnimFrame*>(other);
        if(!ptr)
            throw std::runtime_error("AnimFrame::clone(): other is not a AnimFrame!");
        (*this) = *ptr;
    }

    inline bool operator==( const AnimFrame & other)const  {return this == &other;}
    inline bool operator!=( const AnimFrame & other)const  {return !operator==(other);}

    void importFrame( const fmt::animfrm_t & frm )
    {
        m_data = frm;
    }

    fmt::animfrm_t exportFrame()const
    {
        return m_data;
    }

    inline uint8_t duration()const {return m_data.duration;}
    inline int16_t frmidx  ()const {return m_data.frmidx;}
    inline uint8_t flags   ()const {return m_data.flag;}
    inline int16_t xoffset ()const {return m_data.xoffs;}
    inline int16_t yoffset ()const {return m_data.yoffs;}
    inline int16_t shadowx ()const {return m_data.shadowxoffs;}
    inline int16_t shadowy ()const {return m_data.shadowyoffs;}

    inline void setDuration(uint8_t val) {m_data.duration = val;}
    inline void setFrmidx  (int16_t val) {m_data.frmidx = val;}
    inline void setFlags   (uint8_t val) {m_data.flag = val;}
    inline void setXoffset (int16_t val) {m_data.xoffs = val;}
    inline void setYoffset (int16_t val) {m_data.yoffs = val;}
    inline void setShadowx (int16_t val) {m_data.shadowxoffs = val;}
    inline void setShadowy (int16_t val) {m_data.shadowyoffs = val;}

    Sprite * parentSprite() override;
    const Sprite * parentSprite() const override
    {
        return const_cast<AnimFrame*>(this)->parentSprite();
    }

    virtual QVariant nodeData(int column, int role) const override;
    bool nodeIsMutable()const override    {return true;}

    QImage makePreview()const;

private:
    fmt::animfrm_t  m_data;
    QPixmap         m_cached;
};


#endif // ANIMFRAME_HPP
