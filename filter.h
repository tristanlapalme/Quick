#ifndef FILTER_H
#define FILTER_H

#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QModelIndex>
#include <QtGui/QStandardItemModel>

class MainWindow;

class Filter : public QSortFilterProxyModel
{
public:
    Filter(QObject* parent, QObject* window);
    virtual void setSourceModel(QAbstractItemModel *sourceModel) override;


protected:
    MainWindow* m_mw;
    QStandardItemModel* m_model;
};

class NodeFilter : public Filter
{
    Q_OBJECT
public:
    NodeFilter(QObject* parent, QObject* window) : Filter(parent, window) {}
    virtual ~NodeFilter() override {}
protected:
     virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};


class ParamFilter : public Filter
{
    Q_OBJECT
public:
    ParamFilter(QObject* parent, QObject* window) : Filter(parent, window) {}
    virtual ~ParamFilter() override {}
protected:
     virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
#endif // FILTER_H
