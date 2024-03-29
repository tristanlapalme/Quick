#ifndef ARNOLDNODEDATAMODEL_H
#define ARNOLDNODEDATAMODEL_H

#include <QtCore/QObject>
#include <QtWidgets/QLabel>

#define NODE_EDITOR_SHARED 1
#include <nodes/NodeDataModel>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;

struct AtNode;
struct AtNodeEntry;

class MyNodeData : public NodeData
{
public:
    virtual NodeDataType type() const override
    {
        return NodeDataType {"conn", "conn"};
    }
};

class ArnoldNodeDataModel : public NodeDataModel
{
    Q_OBJECT

public:
    ArnoldNodeDataModel(const AtNode* node, const AtNodeEntry* nodeEntry);
    virtual ~ArnoldNodeDataModel() override {}

    static QString Name() { return QString("ArnoldNodeDataModel"); }

    virtual QString caption() const override;
    virtual QString name() const override;

    virtual unsigned int nPorts(PortType portType) const override;
    virtual NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
    virtual QString portCaption(PortType, PortIndex) const override;
    virtual bool portCaptionVisible(PortType, PortIndex) const override { return true; }
    virtual std::shared_ptr<NodeData> outData(PortIndex /*port*/) override;
    virtual void setInData(std::shared_ptr<NodeData> data, int) override;
    virtual QWidget* embeddedWidget() override;

    const AtNode* GetNode() const { return m_node; }
    const AtNode* GetInput(int portIndex) const;

private:
    const AtNode* m_node{nullptr};
    const AtNodeEntry* m_nodeEntry{nullptr};
    QString m_name;
    QString m_entryName;
    QStringList m_inputs;
    QList<AtNode*> m_srcs;
};




































#endif // ARNOLDNODEDATAMODEL_H
