#include "quickscenewidget.h"
#include "ui_quickscenewidget.h"

#include "arnoldriver.h"
#include "renderview.h"
#include "arnoldnodedatamodel.h"

#include <QtCore/QFile>
#include <QtDebug>

#include <nodes/FlowView>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <nodes/NodeState>
#include <nodes/DataModelRegistry>
#include <nodes/NodeStyle>

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;
using QtNodes::Node;
using QtNodes::NodeState;
using QtNodes::Connection;
using QtNodes::NodeStyle;

namespace
{
    void* GetPTR(QStandardItem* item)
    {
        static const QVariant invalidVariant;
        if (item != nullptr)
        {
            QVariant v = item->data(Qt::UserRole);
            if (v != invalidVariant)
            {
                bool ok = false;
                qulonglong l = v.toULongLong(&ok);
                if (ok)
                {
                    return reinterpret_cast<void*>(l);
                }
            }
        }

        return nullptr;
    }
}

#include <QtCore/QThread>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>

#include <QtDebug>

class MyThread : public QThread
{
public:
    explicit MyThread(QObject *parent = nullptr) : QThread(parent) {}
    virtual ~MyThread() {qDebug() << "Die";}
    void run()
    {
        AiRender(AI_RENDER_MODE_CAMERA);
        this->deleteLater();
    }
};

static const AtString atName("name");
static std::shared_ptr<DataModelRegistry>
registerDataModels()
{
    auto ret = std::make_shared<DataModelRegistry>();
    return ret;
}

static void setStyle()
{
  NodeStyle::setNodeStyle(
  R"(
  {
    "NodeStyle": {
  "NormalBoundaryColor": [255, 255, 255],
  "SelectedBoundaryColor": [255, 165, 0],
  "GradientColor0": "gray",
  "GradientColor1": [80, 80, 80],
  "GradientColor2": [64, 64, 64],
  "GradientColor3": [58, 58, 58],
  "ShadowColor": [20, 20, 20],
  "FontColor" : "white",
  "FontColorFaded" : "gray",
  "ConnectionPointColor": [169, 169, 169],
  "FilledConnectionPointColor": "cyan",
  "ErrorColor": "red",
  "WarningColor": [128, 128, 0],
  "PenWidth": 1.0,
  "HoveredPenWidth": 1.5,
  "ConnectionPointDiameter": 8.0,
  "Opacity": 0.8
    }
  }
  )");
}

QuickSceneWidget::QuickSceneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QuickSceneWidget)
{
    ui->setupUi(this);

    connect(ui->loadAssFile, &QPushButton::clicked, this, &QuickSceneWidget::loadAssFile);
    connect(ui->deepLoadAssFile, &QPushButton::clicked, this, &QuickSceneWidget::deepLoadAssFile);

    m_nodesModel = new QStandardItemModel(this);
    m_nodesModel->setColumnCount(1);
    m_nodesModel->setHorizontalHeaderLabels({"Name"});
    ui->nodes->setModel(m_nodesModel);

    m_paramsModel = new QStandardItemModel(this);
    ui->params->setModel(m_paramsModel);

    connect(ui->nodes, &QAbstractItemView::clicked, this, &QuickSceneWidget::nodeClicked);

    ui->splitter->setSizes({200, 600, 200});

    ::setStyle();
    m_flowScene = new FlowScene(registerDataModels(), ui->graphLayout);
    ui->graphLayout->layout()->addWidget(new FlowView(m_flowScene));

    m_renderView = new RenderView(this);
    ui->render->layout()->addWidget(m_renderView);
    connect(&PixelBuffer::GetInstance(), &PixelBuffer::PixelsReady, m_renderView, &RenderView::ConsumePixels);
}

QuickSceneWidget::~QuickSceneWidget()
{
    delete ui;
}


void QuickSceneWidget::deepLoadAssFile()
{
    loadAssFile(true);

    PixelBuffer::GetInstance().BuildPixels(400*400);
    MyThread* thread = new MyThread();
    thread->start();

    m_renderView->ShowIt();
}

void QuickSceneWidget::nodeClicked(const QModelIndex & index)
{
    QStandardItem* item = m_nodesModel->itemFromIndex(index);
    if(item->parent()->index().isValid() && item->parent()->parent()->index().isValid())
    {
        AtNode* node = static_cast<AtNode*>(GetPTR(item));
        if(node != nullptr)
        {
            ShowParams(node);
        }
    }
}

void QuickSceneWidget::loadAssFile(bool deep)
{
    QString filename = ui->assFile->text();
    if(QFile::exists(filename))
    {
        AiBegin();
        AiMsgSetConsoleFlags(AI_LOG_ALL);
        InitializeArnoldDriver();
        AiASSLoad(filename.toUtf8());

        QStandardItem* root = m_nodesModel->invisibleRootItem();

        AtNodeIterator* iter = AiUniverseGetNodeIterator(nullptr, AI_NODE_ALL);
        while (!AiNodeIteratorFinished(iter))
        {
            AtNode* atNode = AiNodeIteratorGetNext(iter);
            if (atNode == nullptr)
                continue;

            QString name = AiNodeGetStr(atNode, atName).c_str();

            const AtNodeEntry* nodeEntry = AiNodeGetNodeEntry(atNode);
            int nodeEntryType = AiNodeEntryGetType(nodeEntry);

            // type item
            QStandardItem* typeItem = nullptr;
            int count = root->rowCount();
            for(int i=0; i<count; ++i)
            {
                if(root->child(i)->data(Qt::UserRole).toInt() == nodeEntryType)
                {
                    typeItem = root->child(i);
                    break;
                }
            }

            if(typeItem == nullptr)
            {
                typeItem = new QStandardItem(AiNodeEntryGetTypeName(nodeEntry));
                typeItem->setData(nodeEntryType, Qt::UserRole);
                root->appendRow(typeItem);
            }

            // node entry item
            QStandardItem* nodeEntryItem = nullptr;
            count = typeItem->rowCount();
            for(int i=0; i<count; ++i)
            {
                void* ne = GetPTR(typeItem->child(i));
                if(ne == nodeEntry)
                {
                    nodeEntryItem = typeItem->child(i);
                    break;
                }
            }

            if(nodeEntryItem == nullptr)
            {
                nodeEntryItem = new QStandardItem(AiNodeEntryGetName(nodeEntry));
                nodeEntryItem->setData(reinterpret_cast<uintptr_t>(nodeEntry), Qt::UserRole);
                typeItem->appendRow(nodeEntryItem);
            }

            // node item
            QStandardItem* nodeItem = new QStandardItem(name);
            nodeItem->setData(reinterpret_cast<uintptr_t>(atNode), Qt::UserRole);
            nodeEntryItem->appendRow(nodeItem);
            Node& sceneNode = m_flowScene->createNode(std::make_unique<ArnoldNodeDataModel>(atNode, nodeEntry));
            sceneNode.nodeGeometry().setSpacing(10);
            m_flowScene->setNodePosition(sceneNode, QPointF(-10000.0, -10000.0));

        }
        AiNodeIteratorDestroy(iter);


        //AiEnd();

        //todo: better position logic

        for(Node* n : m_flowScene->allNodes())
        {
            ArnoldNodeDataModel* nodeModel = static_cast<ArnoldNodeDataModel*>(n->nodeDataModel());
            unsigned int num = nodeModel->nPorts(PortType::In);
            for(unsigned int i = 0; i < num; ++i)
            {
                const AtNode* src = nodeModel->GetInput(i);
                for(Node* srcNode : m_flowScene->allNodes())
                {
                    ArnoldNodeDataModel* srcModel = static_cast<ArnoldNodeDataModel*>(srcNode->nodeDataModel());
                    if(srcModel->GetNode() == src)
                    {
                        m_flowScene->createConnection(*n, i, *srcNode, 0);
                        break;
                    }
                }

            }
        }

        std::vector<Node*> nodes = m_flowScene->allNodes();
        std::vector<Node*> haveInputs;
        std::vector<Node*> noInputs;
        std::vector<Node*> nodePlaced;

        for(Node* n : m_flowScene->allNodes())
        {
            const NodeState& ns = n->nodeState();
            const NodeState::ConnectionPtrSet& out = ns.connections(PortType::Out, 0);
            if(out.size() == 0)
            {
                bool foundOne = false;
                for(const auto& in : ns.getEntries(PortType::In))
                {
                    if(in.size() != 0)
                    {
                        foundOne = true;
                        break;
                    }
                }

                if(foundOne)
                {
                    haveInputs.push_back(n);
                }
                else
                {
                    noInputs.push_back(n);
                }
            }
        }

        int y = 0;
        for(Node* n : haveInputs)
        {
            PlaceNode(n, y, 300, nodePlaced);
        }

        for(Node* n : noInputs)
        {
            m_flowScene->setNodePosition(*n, QPointF(300, y));
            float yInc = n->nodeGeometry().height() + 50.f;
            y += yInc;
        }
    }
}

void QuickSceneWidget::PlaceNode(Node* n, int& y, int x, std::vector<Node*>& nodePlaced)
{
    m_flowScene->setNodePosition(*n, QPointF(x, y));
    nodePlaced.push_back(n);

    const NodeState& ns = n->nodeState();
    for(const auto& in : ns.getEntries(PortType::In))
    {
        int i = 0;
        for(const auto& conn : in)
        {
            Node* src = conn.second->getNode(PortType::Out);
            if(std::find(nodePlaced.begin(), nodePlaced.end(), src) == nodePlaced.end())
            {
                PlaceNode(src, y, x-300, nodePlaced);
                if(i++ > 0)
                {
                    float yInc = src->nodeGeometry().height() + 50.f;
                    y += yInc;
                }
            }
        }
    }

    if(ns.getEntries(PortType::In).empty())
    {
        float yInc = n->nodeGeometry().height() + 50.f;
        y += yInc;
    }
}

void QuickSceneWidget::DisplayParamValue(const AtNode* node, const AtParamEntry* paramEntry, QStandardItem* item) const
{
    QString value = "";

    AtString pName = AiParamGetName(paramEntry);
    if(AiNodeIsLinked(node, pName))
    {
        AtNode* src = AiNodeGetLink(node, pName);
        if(src != nullptr)
        {
            QString srcName = AiNodeGetStr(src, atName).c_str();
            value = "{" + srcName + "}";
        }
    }
    else
    {
        uint8_t t = AiParamGetType(paramEntry);
        switch(t)
        {
        case AI_TYPE_BYTE: value = QString::number(AiNodeGetByte(node, pName)); break;
        case AI_TYPE_INT: value = QString::number(AiNodeGetInt(node, pName)); break;
        case AI_TYPE_UINT: value = QString::number(AiNodeGetUInt(node, pName)); break;
        case AI_TYPE_FLOAT: value = QString::number(AiNodeGetFlt(node, pName)); break;
        case AI_TYPE_STRING: value = AiNodeGetStr(node, pName).c_str(); break;
        case AI_TYPE_BOOLEAN: value = AiNodeGetBool(node, pName) ? "true" : "false"; break;
        case AI_TYPE_RGB:
        {
            const AtRGB rgb = AiNodeGetRGB(node, pName);
            QColor c;
            c.setRgbF(static_cast<double>(rgb.r), static_cast<double>(rgb.g), static_cast<double>(rgb.b));
            item->setBackground(c);
            break;
        }
        case AI_TYPE_RGBA:
        {
            const AtRGBA rgb = AiNodeGetRGBA(node, pName);
            QColor c;
            c.setRgbF(static_cast<double>(rgb.r), static_cast<double>(rgb.g), static_cast<double>(rgb.b), static_cast<double>(rgb.a));
            item->setBackground(c);
            break;
        }
        case AI_TYPE_VECTOR:
        {
            const AtVector vec = AiNodeGetVec(node, pName);
            value = QString::number(vec.x) + ", " + QString::number(vec.y) + ", " + QString::number(vec.z);
            break;
        }
        case AI_TYPE_VECTOR2:
        {
            const AtVector2 vec = AiNodeGetVec2(node, pName);
            value = QString::number(vec.x) + ", " + QString::number(vec.y);
            break;
        }
        case AI_TYPE_ENUM:
        {
            int v = AiNodeGetInt(node, pName);
            value = AiEnumGetString(AiParamGetEnum(paramEntry), v);
            break;
        }
        case AI_TYPE_NODE:
        {
            AtNode* src = static_cast<AtNode*>(AiNodeGetPtr(node, pName));
            if(src != nullptr)
            {
                QString srcName = AiNodeGetStr(src, atName).c_str();
                value = "{" + srcName + "}";
            }
            break;
        }
        case AI_TYPE_MATRIX:
        {
            AtMatrix m = AiNodeGetMatrix(node, pName);
            value = "matrix";
            QString tt = QString::number(m[0][0]) + " " + QString::number(m[0][1]) + " " + QString::number(m[0][2]) + " " + QString::number(m[0][3]) + "\n" +
                    QString::number(m[1][0]) + " " + QString::number(m[1][1]) + " " + QString::number(m[1][2]) + " " + QString::number(m[1][3]) + "\n" +
                    QString::number(m[2][0]) + " " + QString::number(m[2][1]) + " " + QString::number(m[2][2]) + " " + QString::number(m[2][3]) + "\n" +
                    QString::number(m[3][0]) + " " + QString::number(m[3][1]) + " " + QString::number(m[3][2]) + " " + QString::number(m[3][3]);

            item->setToolTip(tt);
            break;
        }
        case AI_TYPE_ARRAY:
        {
            AtArray* array = AiNodeGetArray(node, pName);
            uint8_t at = AiArrayGetType(array);
            uint32_t num = AiArrayGetNumElements(array);
            switch(at)
            {
            case AI_TYPE_NODE:
            {
                value = "{ ";
                for (uint32_t i = 0; i < num; ++i)
                {
                    AtNode* elem = static_cast<AtNode*>(AiArrayGetPtr(array, i));
                    if (elem != nullptr)
                    {
                        qDebug() << "here 3";

                        QString srcName = AiNodeGetStr(elem, atName).c_str();
                        value += srcName + " ";
                    }
                }
                value += "}";
            }
            break;
            case AI_TYPE_MATRIX:
            {
                value = "matrices";
                QString tt;
                for (uint32_t i = 0; i < num; ++i)
                {
                    AtMatrix m = AiArrayGetMtx(array, i);
                    tt += QString::number(m[0][0]) + " " + QString::number(m[0][1]) + " " + QString::number(m[0][2]) + " " + QString::number(m[0][3]) + "\n" +
                            QString::number(m[1][0]) + " " + QString::number(m[1][1]) + " " + QString::number(m[1][2]) + " " + QString::number(m[1][3]) + "\n" +
                            QString::number(m[2][0]) + " " + QString::number(m[2][1]) + " " + QString::number(m[2][2]) + " " + QString::number(m[2][3]) + "\n" +
                            QString::number(m[3][0]) + " " + QString::number(m[3][1]) + " " + QString::number(m[3][2]) + " " + QString::number(m[3][3]) + "\n\n" ;
                }

                item->setToolTip(tt);
            }
            break;
            }
        }
        break;
        }
    }

    item->setText(value);
}

void QuickSceneWidget::ShowParams(AtNode* node)
{
    m_paramsModel->clear();

    m_paramsModel->setColumnCount(3);
    m_paramsModel->setHorizontalHeaderLabels({"Name", "Type", "Value"});

    ui->params->setColumnWidth(0, 100);
    ui->params->setColumnWidth(1, 100);
    ui->params->setSortingEnabled(false);

    QStandardItem* root = m_paramsModel->invisibleRootItem();
    const AtNodeEntry* nodeEntry = AiNodeGetNodeEntry(node);
    AtParamIterator* piter = AiNodeEntryGetParamIterator(nodeEntry);
    while (!AiParamIteratorFinished(piter))
    {
        const AtParamEntry* pe = AiParamIteratorGetNext(piter);
        QString pName = AiParamGetName(pe).c_str();
        uint8_t t = AiParamGetType(pe);
        uint8_t at = AI_TYPE_NONE;
        QString pType = AiParamGetTypeName(t);
        if(t == AI_TYPE_ARRAY)
        {
            const AtParamValue* defaultValue = AiParamGetDefault(pe);
            AtArray* arr = defaultValue->ARRAY();
            at = AiArrayGetType(arr);
            QString atName = AiParamGetTypeName(at);
            pType = atName + "[]";
        }

        QList<QStandardItem*> l;
        QStandardItem* item = new QStandardItem(pName);
        l.append(item);
        item = new QStandardItem(pType);
        l.append(item);
        item = new QStandardItem();
        DisplayParamValue(node, pe, item);
        l.append(item);
        root->appendRow(l);
    }

    AiParamIteratorDestroy(piter);

    ui->params->setSortingEnabled(true);
}


