#ifndef TABFRAMES_HPP
#define TABFRAMES_HPP
#include <QWidget>
#include <QScopedPointer>
#include <QDataWidgetMapper>
#include <src/ui/tabs/basespritetab.hpp>
#include <src/ui/editor/frame/frame_editor.hpp>
#include <src/utility/baseqtexception.hpp>

namespace Ui {
class TabFrames;
}

class ExBadFrame : public BaseException {public:using BaseException::BaseException;};

class TabFrames : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabFrames(QWidget *parent = nullptr);
    ~TabFrames();

    // BaseSpriteTab interface
public:
    void OnShowTab(Sprite *pspr, QPersistentModelIndex element) override;
    void OnHideTab() override;
    void OnDestruction() override;
    void PrepareForNewContainer() override;
    void OnItemRemoval(const QModelIndex &item) override;

private:
    void on_tblframeparts_clicked(const QModelIndex &index);
    void on_btnFrmExport_clicked();
    void on_btnFrmRmPart_clicked();
    void on_btnFrmAdPart_clicked();
    void on_btnFrmMvUp_clicked();
    void on_btnFrmMvDown_clicked();
    void on_btnFrmDup_clicked();
    void on_cmbFrmQuickPrio_currentIndexChanged(int index);

private:
    void setupFrameEditPageForPart(MFrame *frm, MFramePart *part);
    void ConnectSignals();
    void DisconnectSignals();

private slots:
    void OnFrameEditorZoom(int diff);
    void On_spbFrmZoom_ValueChanged(int val);
    void OnDataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight, const QVector<int> & roles);
    void OnSelectionChanged(QList<int> parts);

private:
    Ui::TabFrames *ui;
    QScopedPointer<FrameEditor>         m_frmeditor;
    QScopedPointer<QDataWidgetMapper>   m_frmdatmapper;
    QPersistentModelIndex               m_frame;

};

#endif // TABFRAMES_HPP
