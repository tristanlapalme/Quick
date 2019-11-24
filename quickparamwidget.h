#ifndef QUICKPARAMWIDGET_H
#define QUICKPARAMWIDGET_H

#include <QWidget>
#include <QtCore/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>

namespace Ui {
class QuickParamWidget;
}

class NodeEntryModel;
class AtParamEntry;

class QuickParamWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuickParamWidget(QWidget *parent = nullptr);
    ~QuickParamWidget();

    QString GetNodeNameFilter();
    QString GetNodeTypeFilter();
    QString GetNodeOutputFilter();
    QString GetNodeParamNameFilter();
    QString GetNodeParamTypeFilter();
    QString GetParamNameFilter();
    QString GetParamTypeFilter();

public slots:
    void applyNodeEntryFilter();
    void applyParamEntryFilter();
    void nodeEntryClicked(const QModelIndex & index);

private:
    void ShowParamEntries(const QString& nodeEntryName);
    void DisplayParamValue(uint8_t type, const AtParamEntry* pe, QStandardItem* item) const;

private:
    Ui::QuickParamWidget *ui;

    NodeEntryModel* m_nodeEntriesModel{nullptr};
    QSortFilterProxyModel* m_nodeEntriesProxyModel{nullptr};

    QStandardItemModel* m_paramEntriesModel{nullptr};
    QSortFilterProxyModel* m_paramEntriesProxyModel{nullptr};
};

#endif // QUICKPARAMWIDGET_H
