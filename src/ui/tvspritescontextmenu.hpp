#ifndef TVSPRITESCONTEXTMENU_HPP
#define TVSPRITESCONTEXTMENU_HPP
#include <QMenu>
#include <QPersistentModelIndex>
#include <QPointer>

class TreeNode;
class MainWindow;

//======================================================================================
//  TVSpritesContextMenu
//======================================================================================
class TVSpritesContextMenu : public QMenu
{
    Q_OBJECT

    QPersistentModelIndex   m_itemidx;
    MainWindow *            m_pmainwindow{nullptr};

public:
    TVSpritesContextMenu( MainWindow * mainwindow, const QModelIndex & item, QWidget * parent = nullptr );

    void BuildMenu();

public:
    void ShowProperties();

    void SaveDump();
    void SaveXMLDump();

    void RemoveEntry();

signals:
    void afterclosed();

private:
    void closeAndDelete();

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;
};


#endif // TVSPRITESCONTEXTMENU_HPP
