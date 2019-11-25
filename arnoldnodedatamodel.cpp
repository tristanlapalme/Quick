#include "arnoldnodedatamodel.h"

#include <ai.h>
#include <QtDebug>

ArnoldNodeDataModel::ArnoldNodeDataModel(const AtNode* node, const AtNodeEntry* nodeEntry)
    : m_node(node)
    , m_nodeEntry(nodeEntry)
{
    m_name = AiNodeGetStr(node, "name");
    m_entryName = AiNodeEntryGetName(AiNodeGetNodeEntry(node));

    AtParamIterator* piter = AiNodeEntryGetParamIterator(nodeEntry);
    while (!AiParamIteratorFinished(piter))
    {
        const AtParamEntry* pe = AiParamIteratorGetNext(piter);
        QString pName = AiParamGetName(pe).c_str();
        if(AiNodeIsLinked(node, pName.toUtf8()))
        {
            m_inputs.append(pName);
            AtNode* src = AiNodeGetLink(node, pName.toUtf8());
            m_srcs.append(src);
        }

    }
    AiParamIteratorDestroy(piter);
}

QString ArnoldNodeDataModel::caption() const
{
    return m_name + " : " + m_entryName;
}

QString ArnoldNodeDataModel::name() const
{
    return m_name;
}

unsigned int ArnoldNodeDataModel::nPorts(PortType portType) const
{
    unsigned int result = 0;

    switch (portType)
    {
    case PortType::In:
        return m_inputs.size();
        break;
    case PortType::Out:
        //result = m_node->nodeEntry->output.isEmpty() ? 0 : 1;
        result = 1;
        break;
    case PortType::None:
        break;
    }

    return result;
}

NodeDataType ArnoldNodeDataModel::dataType(PortType /*portType*/, PortIndex /*portIndex*/) const
{
    return MyNodeData().type();
}

QString ArnoldNodeDataModel::portCaption(PortType portType, PortIndex portIndex) const
{
    switch (portType)
    {
    case PortType::In:
    {
        return m_inputs.at(portIndex);
    }
    case PortType::Out:
        switch (portIndex)
        {
            case 0:
            return "Out";
        }
        break;

    case PortType::None:
        break;
    }

    return QString();
}

std::shared_ptr<NodeData> ArnoldNodeDataModel::outData(PortIndex /*port*/)
{
    return std::make_shared<MyNodeData>();
}

void ArnoldNodeDataModel::setInData(std::shared_ptr<NodeData> /*data*/, int)
{
    //
}

QWidget* ArnoldNodeDataModel::embeddedWidget()
{
    return nullptr;
}

const AtNode* ArnoldNodeDataModel::GetInput(int portIndex) const
{
    return m_srcs.at(portIndex);
}
