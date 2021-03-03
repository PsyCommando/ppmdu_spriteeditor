#ifndef TABFRAMES_HPP
#define TABFRAMES_HPP
#include <QWidget>
#include <QScopedPointer>
#include <QDataWidgetMapper>
#include <src/ui/tabs/basespritetab.hpp>
#include <src/ui/editor/frame/frame_editor.hpp>
#include <src/utility/baseqtexception.hpp>
#include <src/data/sprite/models/framepart_model.hpp>
#include <src/data/sprite/models/framepart_delegate.hpp>
#include <src/data/sprite/models/effect_set_delegate.hpp>

namespace Ui {
class TabFrames;
}

class ExBadFrame : public BaseException {public:using BaseException::BaseException;};

class TabFrames : public BaseSpriteTab
{
    Q_OBJECT

    static const QString TabAttachmentPointsName;
public:
    explicit TabFrames(QWidget *parent = nullptr);
    ~TabFrames();

    // BaseSpriteTab interface
public:
    void OnShowTab(QPersistentModelIndex element)override;
    void OnHideTab() override;
    void OnDestruction() override;
    void PrepareForNewContainer() override;
    void OnItemRemoval(const QModelIndex &item) override;

private:
    void setupMappedControls();
    void setupFramePartTable();
    void setupAttachTable();
    void setupFrameEditPageForPart(MFrame *frm, MFramePart *part);
    void ConnectSignals();
    void DisconnectSignals();

    bool setFrame(QPersistentModelIndex element, Sprite* spr);
    void clearFrame();
    void updateListAndEditor();
    void _MovePart(bool up);
    void updatePartImgSource();

private slots:
    void OnFrameEditorZoom(int diff);
    void On_spbFrmZoom_ValueChanged(int val);
    void OnDataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight, const QVector<int> & roles);
    void OnOffsetChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight, const QVector<int> & roles);
    void OnEditorSelectionChanged(QList<EditableItem *> parts);
    void OnOffsetSelected(QModelIndex selected);

    void on_btnEditAttachments_toggled(bool checked);
    void on_chkGridSnap_toggled(bool checked);
    void on_tblframeparts_clicked(const QModelIndex &index);
    void on_btnFrmExport_clicked();
    void on_btnFrmRmPart_clicked();
    void on_btnFrmAdPart_clicked();
    void on_btnFrmMvUp_clicked();
    void on_btnFrmMvDown_clicked();
    void on_btnFrmDup_clicked();

    void on_tvAttachments_clicked(const QModelIndex &index);

    void selectPart(const QModelIndex & index);

    void on_chkImgSourceRef_toggled(bool checked);

signals:
    void partSelected(const QModelIndexList & selected);
    void markerSelected(const QModelIndexList & selected);

private:
    Ui::TabFrames *ui;
    QScopedPointer<FrameEditor>         m_frmeditor;
    QScopedPointer<QDataWidgetMapper>   m_frmdatmapper;

    QPersistentModelIndex               m_frame;
    QScopedPointer<MFramePartModel>     m_frmModel;
    QScopedPointer<MFramePartDelegate>  m_frmDelegate;
    QScopedPointer<EffectSetModel>      m_attachModel;
    QScopedPointer<EffectSetDelegate>   m_attachDele;
};

#endif // TABFRAMES_HPP
