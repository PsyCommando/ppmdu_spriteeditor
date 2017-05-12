#ifndef DIAGSINGLEIMGCROPPER_HPP
#define DIAGSINGLEIMGCROPPER_HPP

#include <QDialog>

namespace Ui {
class DiagSingleImgCropper;
}

class DiagSingleImgCropper : public QDialog
{
    Q_OBJECT

public:
    explicit DiagSingleImgCropper(QWidget *parent = 0);
    ~DiagSingleImgCropper();

private:
    Ui::DiagSingleImgCropper *ui;
};

#endif // DIAGSINGLEIMGCROPPER_HPP
