#ifndef TABANIMTABLE_HPP
#define TABANIMTABLE_HPP

#include <QWidget>
#include <QItemSelection>
#include <src/ui/mainwindow.hpp>
#include <src/ui/tabs/basespritetab.hpp>
#include <src/ui/rendering/sprite_scene.hpp>
#include <src/data/sprite/models/animtable_model.hpp>

namespace Ui {
class TabAnimTable;
}

//Tab for displaying the full animation table "at a glance" with what each group is associated with as animation
class TabAnimTable : public BaseSpriteTab
{
    Q_OBJECT
public:
    explicit TabAnimTable(QWidget *parent = nullptr);
    ~TabAnimTable();

    // BaseSpriteTab interface
    void OnShowTab(QPersistentModelIndex element)override;
    void OnHideTab() override;
    void OnDestruction()override;
    void PrepareForNewContainer() override;

private:
    void ConnectControls();
    void DisconnectControls();
    bool setAnimTable(QPersistentModelIndex table, Sprite* spr);
    void clearAnimTable();

    void OnSelectGroup(const QItemSelection & selection);
    void OnDeselectAll();
    void PreviewGroupAnimSequence(int idxsubseq, bool bplaynow = false);
    void ClearPreview();

    //Helpers to display the groups and sequences from the preview section into the editor tab
    void OpenCurrentAnimSequence(); //Opens the tab in the main window at the sequence currently being previewed
    void OpenCurrentGroup();

private slots:
    void OnAnimTableItemActivate(const QModelIndex &index);
    //void OnAmimTableGroupListItemActivate(const QModelIndex &index);
    //void OnAmimTableSequenceListItemActivate(const QModelIndex &index);
    //void UpdateAnimTblPreview( fmt::AnimDB::animseqid_t seqid );

    //##### NEW SLOTS
    void on_btnPlay_clicked();
    void on_btnStop_clicked();
    void on_sldrSubSequence_valueChanged(int value);
    void on_chkAutoPlay_toggled(bool checked);
    void on_chkPreviewLoop_toggled(bool checked);
    void on_btnMoveGroupUp_clicked();
    void on_btnMoveGroupDown_clicked();

private:
    Ui::TabAnimTable*                   ui;
    QScopedPointer<SpriteScene>         m_previewrender;
    QPersistentModelIndex               m_animTable;
    QScopedPointer<AnimTableModel>      m_animTableModel;
    QPersistentModelIndex               m_previewedGroup;
};

#endif // TABANIMTABLE_HPP
