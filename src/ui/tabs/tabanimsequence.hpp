#ifndef TABANIMSEQUENCE_HPP
#define TABANIMSEQUENCE_HPP

#include <QWidget>
#include <src/ui/mainwindow.hpp>
#include <src/ui/tabs/basespritetab.hpp>
#include <src/ui/rendering/sprite_scene.hpp>

namespace Ui {
class tabAnimSequence;
}

class TabAnimSequence : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabAnimSequence(QWidget *parent = nullptr);
    ~TabAnimSequence();

// BaseSpriteTab interface
    void OnShowTab(Sprite *pspr, QPersistentModelIndex element) override;
    void OnHideTab() override;
    void OnDestruction()override;
    void OnItemRemoval(const QModelIndex & item)override;
    void PrepareForNewContainer()override;

    void SetCurrentFrame(int frameidx, bool bupdatescene = true);
private:
    void ConnectSceneRenderer();
    void DisconnectSceneRenderer();
    void UpdateTickCounter();

private slots:
    //Elements callbacks
    void on_btnSeqExport_clicked();
    void on_btnSeqAddFrm_clicked();
    void on_btnSeqRemFrm_clicked();
    void on_btnSeqMvUp_clicked();
    void on_btnSeqMvDown_clicked();
    void on_btnSeqDup_clicked();
    void on_tblseqfrmlst_activated(const QModelIndex &index);

    //Scene Renderer callbacks
    void OnPreviewRangeChanged(int beg, int length);
    void OnPreviewFrameChanged(int curfrm, QRectF area);
    void OnPreviewTick(int curtick);

    void on_btnSeqPlay_clicked();

    void on_btnSeqStop_clicked();

    void on_chkAnimSeqLoop_toggled(bool checked);

    void on_sldrAnimSeq_sliderMoved(int position);

    void on_btnSeqImport_clicked();

    void on_spinCurFrm_editingFinished();

    void on_tblseqfrmlst_clicked(const QModelIndex &index);

private:
    Ui::tabAnimSequence *ui;
    SpriteScene m_previewrender;

};

#endif // TABANIMSEQUENCE_HPP
