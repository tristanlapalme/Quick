#include "arnoldnodedatamodel.h"
#include "database.h"
#include <QtDebug>

ArnoldNodeDataModel::ArnoldNodeDataModel(ArnoldNode* node, Database* database)
    : m_node(node), m_database(database)
{
}

QString ArnoldNodeDataModel::caption() const
{
    return m_node->name + " : " + m_node->nodeEntry->name;
}

QString ArnoldNodeDataModel::name() const
{
    return m_node->name;
}

unsigned int ArnoldNodeDataModel::nPorts(PortType portType) const
{
    unsigned int result = 0;

    switch (portType)
    {
    case PortType::In:
        for(auto value : m_node->paramValues)
        {
            result += static_cast<unsigned int>(value.vn.size());
        }
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
        int i = 0;
        int total = 0;
        for(auto value : m_node->paramValues)
        {
            int size = static_cast<int>(value.vn.size());
            if(portIndex < total + size)
            {
                return m_node->paramValues.keys()[i];
            }
            else
            {
                total += size;
            }
            i++;
        }
        break;
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
