#ifndef QUICKSCENEWIDGET_H
#define QUICKSCENEWIDGET_H

#include "pixelbuffer.h"

#include <QWidget>

#include <QtGui/QStandardItemModel>

namespace Ui {
class QuickSceneWidget;
}

namespace QtNodes { class FlowScene; class Node; }

class RenderView;
class NodeEntryModel;
class AtNode;

class QuickSceneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuickSceneWidget(QWidget *parent = nullptr);
    ~QuickSceneWidget();

public slots:
    void loadAssFile(bool deep = false);
    void deepLoadAssFile();
    void nodeClicked(const QModelIndex & index);

private:
    void PlaceNode(QtNodes::Node* n, int& y, int x, std::vector<QtNodes::Node*>& nodePlaced);

    void ShowParamEntries(const QString& nodeEntryName);
    void ShowParams(AtNode* node);

    void DisplayParamValue(const AtNode* node, const AtParamEntry* paramEntry, QStandardItem* item) const;

private:
    Ui::QuickSceneWidget *ui;

    QStandardItemModel* m_nodesModel{nullptr};
    QStandardItemModel* m_paramsModel{nullptr};

    QtNodes::FlowScene* m_flowScene;
    RenderView* m_renderView;
};

#endif // QUICKSCENEWIDGET_H
