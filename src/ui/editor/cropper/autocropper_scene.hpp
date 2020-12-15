#ifndef AUTOCROPPERSCENE_HPP
#define AUTOCROPPERSCENE_HPP

#include <QGraphicsScene>

class AutoCropperGizmo;
class AutoCropperScene : public QGraphicsScene
{
    Q_OBJECT;
    using parent_t = QGraphicsScene;
public:
    AutoCropperScene();
    ~AutoCropperScene();

    void Reset();

    void setImage(const QImage & img);
    const QImage &getImage()const;

    AutoCropperGizmo *getGizmo();

public slots:
    void showGizmo(bool bshow);

signals:

private:
    AutoCropperGizmo *  m_pgizmo {nullptr};
    QImage              m_sheet;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

#endif // AUTOCROPPERSCENE_HPP
