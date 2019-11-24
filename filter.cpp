#include "filter.h"

#include "quickparamwidget.h"

#include <QtCore/QtDebug>

#include <ai.h>
#include <QtDebug>

Filter::Filter(QObject* parent, QObject* window)
    : QSortFilterProxyModel(parent)
    , m_w(static_cast<QuickParamWidget*>(window))
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

        const QString nameFilter = m_w->GetNodeNameFilter().trimmed();
        const QString typeFilter = m_w->GetNodeTypeFilter().trimmed();
        const QString outputFilter = m_w->GetNodeOutputFilter().trimmed();
        const QString paramNameFilter = m_w->GetNodeParamNameFilter().trimmed();
        const QString paramTypeFilter = m_w->GetNodeParamTypeFilter().trimmed();

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

        const AtNodeEntry* ne = AiNodeEntryLookUp(name.toUtf8());
        if(ne != nullptr)
        {
            if(!paramNameFilter.isEmpty())
            {
                bool found = false;
                AtParamIterator* piter = AiNodeEntryGetParamIterator(ne);
                while (!AiParamIteratorFinished(piter))
                {
                    const AtParamEntry* pe = AiParamIteratorGetNext(piter);
                    QString pName = AiParamGetName(pe).c_str();
                    if(pName.contains(paramNameFilter, Qt::CaseInsensitive))
                    {
                        found = true;
                        break;
                    }
                }
                AiParamIteratorDestroy(piter);

                if(!found)
                {
                    return false;
                }
            }

            if(!paramTypeFilter.isEmpty())
            {
                bool found = false;
                AtParamIterator* piter = AiNodeEntryGetParamIterator(ne);
                while (!AiParamIteratorFinished(piter))
                {
                    const AtParamEntry* pe = AiParamIteratorGetNext(piter);;
                    uint8_t t = AiParamGetType(pe);
                    QString pType = AiParamGetTypeName(t);
                    if(AiParamGetType(pe) == AI_TYPE_ARRAY)
                    {
                        const AtParamValue* defaultValue = AiParamGetDefault(pe);
                        AtArray* arr = defaultValue->ARRAY();
                        pType = QString(AiParamGetTypeName(AiArrayGetType(arr))) + "[]";
                    }

                    if(pType.contains(paramTypeFilter, Qt::CaseInsensitive))
                    {
                        found = true;
                        break;
                    }
                }
                AiParamIteratorDestroy(piter);

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

        const QString nameFilter = m_w->GetParamNameFilter().trimmed();
        const QString typeFilter = m_w->GetParamTypeFilter().trimmed();

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
