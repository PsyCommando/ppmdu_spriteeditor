#ifndef ATTACHMARKER_HPP
#define ATTACHMARKER_HPP
#include <src/ui/editor/frame/frame_editor_editable.hpp>
#include <QGraphicsTextItem>
#include <QPointer>
#include <QModelIndex>

class EffectOffset;
class AttachMarker : public EditableItem
{
    Q_OBJECT
    using parent_t = EditableItem;
    static const QString PosMarker;
public:
    AttachMarker(FrameEditor * frmedit, const QModelIndex & idx, QGraphicsItem * parent = nullptr);
    ~AttachMarker();

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;

protected:
    bool shouldShowBoundingBox() const final {return false;}

public slots:
    void updateOffset();

private:
    EffectOffset *  m_poffset   {nullptr};
    QGraphicsTextItem m_txtitem;
};

#endif // ATTACHMARKER_HPP
