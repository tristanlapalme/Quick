#include "filter.h"

#include "mainwindow.h"

#include <QtCore/QtDebug>

Filter::Filter(QObject* parent, QObject* window)
    : QSortFilterProxyModel(parent)
    , m_mw(static_cast<MainWindow*>(window))
{

}

void Filter::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    m_model = static_cast<QStandardItemModel*>(sourceModel);
}

bool NodeFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
    {
        QStandardItem* root = m_model->invisibleRootItem();
        QStandardItem* item = root->child(source_row);

        QModelIndex index = item->index();

        const QString nameFilter = m_mw->GetNodeNameFilter().trimmed();
        const QString typeFilter = m_mw->GetNodeTypeFilter().trimmed();
        const QString outputFilter = m_mw->GetNodeOutputFilter().trimmed();
        const QString paramNameFilter = m_mw->GetNodeParamNameFilter().trimmed();
        const QString paramTypeFilter = m_mw->GetNodeParamTypeFilter().trimmed();

        const QString name = item->data(Qt::UserRole).toString();
        const QString type = m_model->itemFromIndex(index.siblingAtColumn(1))->text();
        const QString output = m_model->itemFromIndex(index.siblingAtColumn(2))->text();

        if(!nameFilter.isEmpty() && !name.contains(nameFilter, Qt::CaseInsensitive))
        {
            return false;
        }

        if(!typeFilter.isEmpty() && !type.contains(typeFilter, Qt::CaseInsensitive))
        {
            return false;
        }

        if(!outputFilter.isEmpty() && !output.contains(outputFilter, Qt::CaseInsensitive))
        {
            return false;
        }

        ArnoldNodeEntry* node = m_mw->GetDatabase().GetNodeEntry(name);
        if(node != nullptr)
        {
            if(!paramNameFilter.isEmpty())
            {
                bool found = false;
                for(auto param : node->params)
                {
                    if(param->name.contains(paramNameFilter, Qt::CaseInsensitive))
                    {
                        found = true;
                        break;
                    }
                }

                if(!found)
                {
                    return false;
                }
            }

            if(!paramTypeFilter.isEmpty())
            {
                bool found = false;
                for(auto param : node->params)
                {
                    if(param->paramTypeName.contains(paramTypeFilter, Qt::CaseInsensitive))
                    {
                        found = true;
                        break;
                    }
                }

                if(!found)
                {
                    return false;
                }
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool ParamFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
    {
        QStandardItem* root = m_model->invisibleRootItem();
        QStandardItem* item = root->child(source_row);

        QModelIndex index = item->index();

        const QString nameFilter = m_mw->GetParamNameFilter().trimmed();
        const QString typeFilter = m_mw->GetParamTypeFilter().trimmed();

        const QString name = item->text();
        const QString type = m_model->itemFromIndex(index.siblingAtColumn(1))->text();

        if(!nameFilter.isEmpty() && !name.contains(nameFilter, Qt::CaseInsensitive))
        {
            return false;
        }

        if(!typeFilter.isEmpty() && !type.contains(typeFilter, Qt::CaseInsensitive))
        {
            return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}
