#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Database.h"

#include <QMainWindow>
#include <QtGui/QStandardItemModel>

namespace Ui { class MainWindow; }

namespace QtNodes { class FlowScene; class Node; }

class RenderView;
class NodeEntryModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void loadAssFile(bool deep = false);
    void deepLoadAssFile();
    void nodeClicked(const QModelIndex & index);

private:
    void ShowParamEntries(const QString& nodeEntryName);
    void ShowParams(ArnoldNode* node);
    void PlaceNode(QtNodes::Node* n, int& y, int x, std::vector<QtNodes::Node*>& nodePlaced);

    void DisplayParamValue(const ParamValue& pv, ArnoldParamEntry* param, QStandardItem* item) const;

private:
    Ui::MainWindow *ui;

    QStandardItemModel* m_nodesModel{nullptr};
    QStandardItemModel* m_paramsModel{nullptr};

    QtNodes::FlowScene* m_flowScene;
    RenderView* m_renderView;
};

#endif // MAINWINDOW_H
