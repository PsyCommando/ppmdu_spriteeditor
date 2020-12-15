#ifndef DIALOGNEW_HPP
#define DIALOGNEW_HPP

#include <QDialog>
#include <src/data/sprite/sprite_container.hpp>

namespace Ui {
class DialogNew;
}

class DialogNew : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNew(QWidget *parent = nullptr);
    ~DialogNew();

    void InitTabs();
    void InitTypeList();
    void SelectType(const QString & type);

    //Result Access Stuff
    QString getContentType()const;
    eCompressionFmtOptions          getSprCompression()const;
    SpriteContainer::eContainerType getSprFormatType()const;

public:


private slots:
    void on_cmbContainerType_activated(int index);
    void on_cmbContainerType_currentIndexChanged(int index);
    void on_cmbSprFormat_activated(int index);
    void on_cmbSprCompression_activated(int index);

private:
    Ui::DialogNew *ui;

};

#endif // DIALOGNEW_HPP
