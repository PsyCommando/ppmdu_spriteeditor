#ifndef TABANIMTABLE_HPP
#define TABANIMTABLE_HPP

#include <QWidget>
#include <src/ui/mainwindow.hpp>
#include <src/ui/tabs/basespritetab.hpp>
#include <src/ui/rendering/sprite_scene.hpp>

namespace Ui {
class TabAnimTable;
}

class TabAnimTable : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabAnimTable(QWidget *parent = nullptr);
    ~TabAnimTable();

    // BaseSpriteTab interface
    void OnShowTab(Sprite *pspr, QPersistentModelIndex element) override;
    void OnHideTab() override;
    void OnDestruction()override;
    void PrepareForNewContainer() override;

private:
    void ConnectControls();
    void DisconnectControls();

private slots:
    void OnAnimTableItemActivate(const QModelIndex &index);
    void OnAmimTableGroupListItemActivate(const QModelIndex &index);
    void OnAmimTableSequenceListItemActivate(const QModelIndex &index);
    void UpdateAnimTblPreview( fmt::AnimDB::animseqid_t seqid );

    void on_tvAnimTbl_activated(const QModelIndex &index);
    void on_btnAnimTblMoveSeq_clicked();
    void on_btnAnimTblExportTemplate_pressed();
    void on_btnAnimTblReplaceSeq_pressed();
    void on_btnAnimTblImportTemplate_pressed();
    void on_btnAnimTblAddSeq_pressed();
    void on_btnAnimTblRemSeq_pressed();
    void on_btnAnimTblMoveSeq_pressed();
    void on_btnAnimTblAddAnim_pressed();
    void on_btnAnimTblRemAnim_pressed();
    void on_btnAnimTblAppendAnim_pressed();
    void on_btnAnimTblCvHeroAnims_pressed();
    void on_btnAnimTblAnimMvUp_pressed();
    void on_btnAnimTblAnimMvDown_pressed();
    void on_chkAnimTblAutoPlay_toggled(bool checked);

private:
    Ui::TabAnimTable *ui;
    QScopedPointer<SpriteScene> m_previewrender;
    QPersistentModelIndex m_animTable;
};

#endif // TABANIMTABLE_HPP
