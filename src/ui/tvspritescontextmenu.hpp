#ifndef TVSPRITESCONTEXTMENU_HPP
#define TVSPRITESCONTEXTMENU_HPP
#include <QMenu>
#include <QPersistentModelIndex>
#include <QPointer>

class TreeElement;
class MainWindow;

//======================================================================================
//  TVSpritesContextMenu
//======================================================================================
class TVSpritesContextMenu : public QMenu
{
    Q_OBJECT

    QPersistentModelIndex m_itemidx;
    TreeElement * m_pitem{nullptr};
    MainWindow * m_pmainwindow{nullptr};

public:
    TVSpritesContextMenu( MainWindow * mainwindow, const QModelIndex & item, QWidget * parent = nullptr );

    void BuildMenu();

public:
    void ShowProperties();

    void SaveDump();

    void RemoveEntry();

signals:
    void afterclosed();

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;
};


#endif // TVSPRITESCONTEXTMENU_HPP
