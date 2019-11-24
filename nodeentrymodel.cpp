#include "nodeentrymodel.h"

#include <ai.h>

NodeEntryModel::NodeEntryModel(QObject *parent)
    : QStandardItemModel(parent)
{

}

NodeEntryModel::~NodeEntryModel()
{

}

void NodeEntryModel::Init()
{
    QStandardItem* root = invisibleRootItem();

    AiBegin();
    AiMsgSetConsoleFlags(AI_LOG_NONE);

    AtNodeEntryIterator* node_entry_it = AiUniverseGetNodeEntryIterator(AI_NODE_ALL);
    while (!AiNodeEntryIteratorFinished(node_entry_it))
    {
        AtNodeEntry* ne = AiNodeEntryIteratorGetNext(node_entry_it);
        QString name = AiNodeEntryGetName(ne);
        QString type = AiNodeEntryGetTypeName(ne);
        QString output = AiParamGetTypeName(static_cast<uint8_t>(AiNodeEntryGetOutputType(ne)));
        bool deprecated = false;
        AiMetaDataGetBool(ne, nullptr, "deprecated", &deprecated);
        QString showName = (deprecated) ? name + " [deprecated]" : name;

        QList<QStandardItem*> l;
        QStandardItem* item = new QStandardItem(showName);
        item->setData(name, Qt::UserRole);
        l.append(item);
        item = new QStandardItem(type);
        item->setData(name, Qt::UserRole);
        l.append(item);
        item = new QStandardItem(output);
        item->setData(name, Qt::UserRole);
        l.append(item);
        root->appendRow(l);
    }
}
