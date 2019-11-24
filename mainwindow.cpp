#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <ai.h>

#include "filter.h"
#include "nodeentrymodel.h"
#include "quickparamwidget.h"

#include "renderview.h"

#include <QtGui/QStandardItem>
#include <QtCore/QFile>
#include <QtDebug>

#include "arnoldnodedatamodel.h"

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Arnold Quick Param (quick.exe)");
    this->setWindowFlags(Qt::Window);
    ui->statusBar->hide();
    ui->menuBar->hide();
    ui->mainToolBar->hide();

    ui->paramTab->layout()->addWidget(new QuickParamWidget(this));

    connect(ui->loadAssFile, &QPushButton::clicked, this, &MainWindow::loadAssFile);
    connect(ui->deepLoadAssFile, &QPushButton::clicked, this, &MainWindow::deepLoadAssFile);

    m_nodesModel = new QStandardItemModel(this);
    m_nodesModel->setColumnCount(1);
    m_nodesModel->setHorizontalHeaderLabels({"Name"});
    ui->nodes->setModel(m_nodesModel);

    m_paramsModel = new QStandardItemModel(this);
    ui->params->setModel(m_paramsModel);

    connect(ui->nodes, &QAbstractItemView::clicked, this, &MainWindow::nodeClicked);

    ui->tabWidget->setCurrentIndex(0);
    ui->splitter->setSizes({200, 600, 200});

    ::setStyle();
    m_flowScene = new FlowScene(registerDataModels(), ui->graphLayout);
    ui->graphLayout->layout()->addWidget(new FlowView(m_flowScene));

    m_renderView = new RenderView(this);
    ui->render->layout()->addWidget(m_renderView);
    connect(&Database::GetInstance(), &Database::PixelsReady, m_renderView, &RenderView::ConsumePixels);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::deepLoadAssFile()
{
    loadAssFile(true);
    m_renderView->ShowIt();
}

void MainWindow::nodeClicked(const QModelIndex & index)
{
    QStandardItem* item = m_nodesModel->itemFromIndex(index);
    const QString& name = item->data(Qt::UserRole).toString();
    ArnoldNode* node = Database::GetInstance().GetScene().nodes[name];
    if(node != nullptr)
    {
        ShowParams(node);
    }
}

void MainWindow::loadAssFile(bool deep)
{
    QString filename = ui->assFile->text();
    if(QFile::exists(filename))
    {
        if(Database::GetInstance().LoadScene(filename, deep))
        {
            const Scene& scene = Database::GetInstance().GetScene();
            QStandardItem* root = m_nodesModel->invisibleRootItem();

            int i = 0;
            for(const QString& type : Database::GetInstance().GetTypes())
            {
                int typeCount = 0;
                const QMap<QString, QVector<ArnoldNode*>>& map = scene.sceneNodeEntries[i];
                if(!map.empty())
                {
                    QStandardItem* typeItem = new QStandardItem();
                    root->appendRow(typeItem);

                    auto itt = map.constBegin();
                    while (itt != map.constEnd())
                    {
                        int count = itt.value().size();
                        typeCount += count;
                        QStandardItem* neItem = new QStandardItem(itt.key() + " (" + QString::number(count) + ")");
                        typeItem->appendRow(neItem);

                        const QVector<ArnoldNode*>& nodes = itt.value();
                        for(ArnoldNode* node : nodes)
                        {
                            QStandardItem* item = new QStandardItem(node->name);
                            item->setData(node->name, Qt::UserRole);
                            neItem->appendRow(item);
                        }

                        ++itt;
                    }

                    typeItem->setText(type + " (" + QString::number(typeCount) + ")");
                }

                i++;
            }

            //todo: better position logic

            for(const auto& node : scene.nodes)
            {
                auto& sceneNode = m_flowScene->createNode(std::make_unique<ArnoldNodeDataModel>(node, &Database::GetInstance()));
                sceneNode.nodeGeometry().setSpacing(10);
                node->sceneNode = &sceneNode;
                m_flowScene->setNodePosition(sceneNode, QPointF(-10000.0, -10000.0));
            }

            for(Node* n : m_flowScene->allNodes())
            {
                ArnoldNodeDataModel* nodeModel = static_cast<ArnoldNodeDataModel*>(n->nodeDataModel());
                ArnoldNode* node = nodeModel->GetNode();

                int i=0;
                for(const ParamValue& value : node->paramValues)
                {
                    for(ArnoldNode* an : value.vn)
                    {
                        m_flowScene->createConnection(*n, i++, *(an->sceneNode), 0);
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
}

void MainWindow::PlaceNode(Node* n, int& y, int x, std::vector<Node*>& nodePlaced)
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

void MainWindow::DisplayParamValue(const ParamValue& pv, ArnoldParamEntry* param, QStandardItem* item) const
{
    QString value = "";
    if(!pv.vn.empty())
    {
        value = "{ ";
        for(ArnoldNode* an : pv.vn)
        {
            value += an->name + " ";
        }
        value += "}";
    }
    else
    {
        switch(param->paramType)
        {
        case AI_TYPE_BYTE:
        case AI_TYPE_INT:
        case AI_TYPE_UINT:
            value = QString::number(pv.i);
        break;
        case AI_TYPE_FLOAT:
            value = QString::number(static_cast<double>(pv.f));
        break;
        case AI_TYPE_BOOLEAN:
            //value = (!pv.b.empty()) ? ((pv.b.front()) ? "true" : "false") : "";
            value = (pv.b ? "true" : "false");
        break;
        case AI_TYPE_STRING:
            value = (!pv.s.empty()) ? *(pv.s.front()) : "";
            //value = pv.s;
        break;
        case AI_TYPE_RGB:
        {
            const AtRGB& rgb = pv.rgb;
            QColor c;
            c.setRgbF(static_cast<double>(rgb.r), static_cast<double>(rgb.g), static_cast<double>(rgb.b));
            item->setBackground(c);
        }
        break;
        case AI_TYPE_RGBA:
        {
           const AtRGBA& rgb = pv.rgba;
           QColor c;
           c.setRgbF(static_cast<double>(rgb.r), static_cast<double>(rgb.g), static_cast<double>(rgb.b), static_cast<double>(rgb.a));
           item->setBackground(c);
        }
        break;
        case AI_TYPE_VECTOR:
        {
            const AtVector& vec = pv.v;
            value = QString::number(vec.x) + ", " + QString::number(vec.y) + ", " + QString::number(vec.y);
        }
        break;
        case AI_TYPE_ENUM:
            value = ((!pv.s.empty()) ? *(pv.s.front()) : "") + " ( ";
            //value =  pv.s + " ( ";
            for(const QString& s : param->valueEnum)
            {
                value += s + " ";
            }
            value += ")";
        break;
        case AI_TYPE_ARRAY:
        {
            switch(param->arrayType)
            {
            case AI_TYPE_NODE:
            break;
            case AI_TYPE_STRING:
            {
                for(QString* s : pv.s)
                {
                    value += "\"" + *s + "\" ";
                }
                break;
            }
            case AI_TYPE_BOOLEAN:
            {
                for(QString* s : pv.s)
                {
                    value += "\"" + *s + "\" ";
                }
                break;
            }
            default:
            {
               qDebug() << "Array param display not handled: " << param->name << " of type " << param->paramTypeName;
            }
            break;
            }
        }
        break;
        default:
        {
           qDebug() << "Param display not handled: " << param->name << " of type " << param->paramTypeName;
        }
        break;
        }
    }

    item->setText(value);
}

void MainWindow::ShowParams(ArnoldNode* node)
{
    m_paramsModel->clear();

    m_paramsModel->setColumnCount(3);
    m_paramsModel->setHorizontalHeaderLabels({"Name", "Type", "Value"});

    ui->params->setColumnWidth(0, 100);
    ui->params->setColumnWidth(1, 100);
    ui->params->setSortingEnabled(false);

    QStandardItem* root = m_paramsModel->invisibleRootItem();
    ArnoldNodeEntry* ne = node->nodeEntry;
    for(auto param : ne->params)
    {

        if(node->paramValues.contains(param->name))
        {

            QList<QStandardItem*> l;
            QStandardItem* item = new QStandardItem(param->name);
            l.append(item);
            item = new QStandardItem(param->paramTypeName);
            l.append(item);
            item = new QStandardItem();
            const ParamValue& pv = node->paramValues[param->name];
            DisplayParamValue(pv, param, item);
            l.append(item);
            root->appendRow(l);
        }
    }

    ui->params->setSortingEnabled(true);
}
