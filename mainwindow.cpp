#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <ai.h>

#include "filter.h"

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

    m_nodeEntriesModel = new QStandardItemModel(this);
    m_nodeEntriesModel->setColumnCount(3);
    m_nodeEntriesModel->setHorizontalHeaderLabels({"Name", "Type", "Output"});

    m_nodeEntriesProxyModel = new NodeFilter(m_nodeEntriesModel, this);
    m_nodeEntriesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_nodeEntriesProxyModel->setFilterKeyColumn(0);
    m_nodeEntriesProxyModel->setSourceModel(m_nodeEntriesModel);

    m_paramEntriesModel = new QStandardItemModel(this);
    m_paramEntriesModel->setColumnCount(3);
    m_paramEntriesModel->setHorizontalHeaderLabels({"Name", "Type", "Default"});

    m_paramEntriesProxyModel = new ParamFilter(m_paramEntriesModel, this);
    m_paramEntriesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_paramEntriesProxyModel->setFilterKeyColumn(0);
    m_paramEntriesProxyModel->setSourceModel(m_paramEntriesModel);

    m_database.Populate();

    QStandardItem* root = m_nodeEntriesModel->invisibleRootItem();
    auto nodes = m_database.GetNodeEntries();
    auto i = nodes.constBegin();
    while (i != nodes.constEnd())
    {
        QString name = i.key();
        QString showName = (i.value()->deprecated) ? name + " [deprecated]" : name;
        const QString& type = i.value()->nodeType;
        const QString& output = i.value()->output;

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

        ++i;
    }

    ui->nodeEntries->setModel(m_nodeEntriesProxyModel);
    ui->nodeEntries->setColumnWidth(0, 200);
    ui->nodeEntries->setColumnWidth(1, 75);
    ui->nodeEntries->setColumnWidth(2, 75);
    ui->nodeEntries->setSortingEnabled(true);
    ui->nodeEntries->sortByColumn(0, Qt::AscendingOrder);

    ui->paramEntries->setModel(m_paramEntriesProxyModel);
    ui->paramEntries->setColumnWidth(0, 200);
    ui->paramEntries->setColumnWidth(1, 75);

    connect(ui->nodeName, &QLineEdit::textEdited, this, &MainWindow::applyNodeEntryFilter);
    connect(ui->nodeType, &QLineEdit::textEdited, this, &MainWindow::applyNodeEntryFilter);
    connect(ui->nodeOutput, &QLineEdit::textEdited, this, &MainWindow::applyNodeEntryFilter);
    connect(ui->nodeParamName, &QLineEdit::textEdited, this, &MainWindow::applyNodeEntryFilter);
    connect(ui->nodeParamType, &QLineEdit::textEdited, this, &MainWindow::applyNodeEntryFilter);

    connect(ui->paramName, &QLineEdit::textEdited, this, &MainWindow::applyParamEntryFilter);
    connect(ui->paramType, &QLineEdit::textEdited, this, &MainWindow::applyParamEntryFilter);

    connect(ui->nodeEntries, &QAbstractItemView::clicked, this, &MainWindow::nodeEntryClicked);

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
}

MainWindow::~MainWindow()
{
    m_nodeEntriesModel->deleteLater();

    delete ui;
}

void MainWindow::applyNodeEntryFilter()
{
    m_nodeEntriesProxyModel->setFilterWildcard("");
    m_paramEntriesModel->clear();
}

void MainWindow::applyParamEntryFilter()
{
    m_paramEntriesProxyModel->setFilterWildcard("");
}

void MainWindow::nodeEntryClicked(const QModelIndex & index)
{
    QModelIndex idx = m_nodeEntriesProxyModel->mapToSource(index);
    QStandardItem* item = m_nodeEntriesModel->itemFromIndex(idx);
    const QString& name = item->data(Qt::UserRole).toString();
    ArnoldNodeEntry* node = m_database.GetNodeEntry(name);
    if(node != nullptr)
    {
        ShowParamEntries(node);
    }
}

void MainWindow::deepLoadAssFile()
{
    loadAssFile(true);
}

void MainWindow::nodeClicked(const QModelIndex & index)
{
    QStandardItem* item = m_nodesModel->itemFromIndex(index);
    const QString& name = item->data(Qt::UserRole).toString();
    ArnoldNode* node = m_database.GetScene().nodes[name];
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
        if(m_database.LoadScene(filename, deep))
        {
            const Scene& scene = m_database.GetScene();
            QStandardItem* root = m_nodesModel->invisibleRootItem();

            int i = 0;
            for(const QString& type : m_database.GetTypes())
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
                auto& sceneNode = m_flowScene->createNode(std::make_unique<ArnoldNodeDataModel>(node, &m_database));
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

QString MainWindow::GetNodeNameFilter()
{
    return ui->nodeName->text();
}

QString MainWindow::GetNodeTypeFilter()
{
    return ui->nodeType->text();
}

QString MainWindow::GetNodeOutputFilter()
{
    return ui->nodeOutput->text();
}

QString MainWindow::GetNodeParamNameFilter()
{
    return ui->nodeParamName->text();
}

QString MainWindow::GetNodeParamTypeFilter()
{
    return ui->nodeParamType->text();
}

QString MainWindow::GetParamNameFilter()
{
    return ui->paramName->text();
}

QString MainWindow::GetParamTypeFilter()
{
    return ui->paramType->text();
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
            //value = (!pv.s.empty()) ? pv.s.front() : "";
            value = pv.s;
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
            //value = ((!pv.s.empty()) ? pv.s.front() : "") + " ( ";
            value =  pv.s + " ( ";
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
                for(const QString& s : pv.s)
                {
                    value += "\"" + s + "\" ";
                }
                break;
            }
            case AI_TYPE_BOOLEAN:
            {
                for(const QString& s : pv.s)
                {
                    value += "\"" + s + "\" ";
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

void MainWindow::ShowParamEntries(ArnoldNodeEntry* node)
{
    m_paramEntriesModel->clear();

    m_paramEntriesModel->setColumnCount(3);
    m_paramEntriesModel->setHorizontalHeaderLabels({"Name", "Type", "Default"});

    ui->paramEntries->setColumnWidth(0, 250);
    ui->paramEntries->setColumnWidth(1, 100);
    ui->paramEntries->setSortingEnabled(false);

    QStandardItem* root = m_paramEntriesModel->invisibleRootItem();

    for(auto param : node->params)
    {
        QList<QStandardItem*> l;
        QStandardItem* item = new QStandardItem(param->name);
        l.append(item);
        item = new QStandardItem(param->paramTypeName);
        l.append(item);
        item = new QStandardItem();
        const ParamValue& pv  = param->value;
        DisplayParamValue(pv, param, item);
        l.append(item);
        root->appendRow(l);
    }

    ui->paramEntries->setSortingEnabled(true);
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
