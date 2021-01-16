#ifndef DIAGSINGLEIMGCROPPER_HPP
#define DIAGSINGLEIMGCROPPER_HPP

#include <QDialog>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/image.hpp>

namespace Ui
{
class DiagSingleImgCropper;
}

class DiagSingleImgCropper : public QDialog
{
    Q_OBJECT

public:
    explicit DiagSingleImgCropper(QWidget *parent, Image *pimg);
    ~DiagSingleImgCropper();

private:
    Ui::DiagSingleImgCropper *ui;
    Image                    *m_pimg;
};

#endif // DIAGSINGLEIMGCROPPER_HPP
