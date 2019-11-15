#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Database.h"

#include <QMainWindow>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSortFilterProxyModel>

namespace Ui { class MainWindow; }

namespace QtNodes { class FlowScene; class Node; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString GetNodeNameFilter();
    QString GetNodeTypeFilter();
    QString GetNodeOutputFilter();
    QString GetNodeParamNameFilter();
    QString GetNodeParamTypeFilter();
    QString GetParamNameFilter();
    QString GetParamTypeFilter();

    const Database& GetDatabase() { return m_database; }

public slots:
    void applyNodeEntryFilter();
    void applyParamEntryFilter();
    void nodeEntryClicked(const QModelIndex & index);
    void loadAssFile(bool deep = false);
    void deepLoadAssFile();
    void nodeClicked(const QModelIndex & index);

private:
    void ShowParamEntries(ArnoldNodeEntry* node);
    void ShowParams(ArnoldNode* node);
    void PlaceNode(QtNodes::Node* n, int& y, int x, std::vector<QtNodes::Node*>& nodePlaced);

private:
    Ui::MainWindow *ui;
    QStandardItemModel* m_nodeEntriesModel{nullptr};
    QSortFilterProxyModel* m_nodeEntriesProxyModel{nullptr};
    QStandardItemModel* m_paramEntriesModel{nullptr};
    QSortFilterProxyModel* m_paramEntriesProxyModel{nullptr};
    QStandardItemModel* m_nodesModel{nullptr};
    QStandardItemModel* m_paramsModel{nullptr};
    void DisplayParamValue(const ParamValue& pv, ArnoldParamEntry* param, QStandardItem* item) const;

    Database m_database;
    QtNodes::FlowScene* m_flowScene;
};

#endif // MAINWINDOW_H
