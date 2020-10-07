#ifndef ANIMFRAME_HPP
#define ANIMFRAME_HPP
#include <QTableWidget>
#include <QList>
#include <QVector>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>
#include <QStyledItemDelegate>
#include <QStandardItemModel>

#include <src/data/treenodeterminal.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>


class Sprite;
class MFrame;
extern const QString ElemName_AnimFrame;

//*******************************************************************
//  AnimFrame
//*******************************************************************
class AnimFrame : public TreeNodeTerminal
{
    friend class AnimFramesModel;
public:
    AnimFrame(TreeNode * parent)
        :TreeNodeTerminal(parent)
    {
        m_flags |= Qt::ItemFlag::ItemIsEditable;
    }
    AnimFrame(const AnimFrame& cp)
        :TreeNodeTerminal(cp)
    {
        m_data = cp.m_data;
    }
    AnimFrame(AnimFrame&& mv)
        : TreeNodeTerminal(mv)
    {
        m_data = qMove(mv.m_data);
    }
    AnimFrame& operator=(const AnimFrame& cp)
    {
        m_data = cp.m_data;
        TreeNodeTerminal::operator=(cp);
        return *this;
    }
    AnimFrame& operator=(AnimFrame&& mv)
    {
        m_data = qMove(mv.m_data);
        TreeNodeTerminal::operator=(mv);
        return *this;
    }
    ~AnimFrame(){}

    eTreeElemDataType   nodeDataTy()const override      {return eTreeElemDataType::animFrame;}
    const QString&      nodeDataTypeName()const override{return ElemName_AnimFrame;}

    TreeNode * clone()const override
    {
        AnimFrame * ptr = new AnimFrame(m_parentItem);
        (*ptr) = (*this);
        return ptr;
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

    bool nodeIsMutable()const override    {return true;}

    QImage makePreview(const Sprite* owner)const;

private:
    fmt::animfrm_t  m_data;
    QPixmap         m_cached;
};


#endif // ANIMFRAME_HPP
