#include "quickparamwidget.h"
#include "ui_quickparamwidget.h"

#include "nodeentrymodel.h"
#include "filter.h"

#include <ai.h>

#include <QtDebug>

QuickParamWidget::QuickParamWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QuickParamWidget)
{
    ui->setupUi(this);

    m_nodeEntriesModel = new NodeEntryModel(this);
    m_nodeEntriesModel->setColumnCount(3);
    m_nodeEntriesModel->setHorizontalHeaderLabels({"Name", "Type", "Output"});

    m_nodeEntriesProxyModel = new NodeFilter(m_nodeEntriesModel, this);
    m_nodeEntriesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_nodeEntriesProxyModel->setFilterKeyColumn(0);
    m_nodeEntriesProxyModel->setSourceModel(m_nodeEntriesModel);

    m_nodeEntriesModel = new NodeEntryModel(this);
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

    m_nodeEntriesModel->Init();

    ui->nodeEntries->setModel(m_nodeEntriesProxyModel);
    ui->nodeEntries->setColumnWidth(0, 200);
    ui->nodeEntries->setColumnWidth(1, 75);
    ui->nodeEntries->setColumnWidth(2, 75);
    ui->nodeEntries->setSortingEnabled(true);
    ui->nodeEntries->sortByColumn(0, Qt::AscendingOrder);

    ui->paramEntries->setModel(m_paramEntriesProxyModel);
    ui->paramEntries->setColumnWidth(0, 200);
    ui->paramEntries->setColumnWidth(1, 75);

    connect(ui->nodeName, &EditClearWidget::textEdited, this, &QuickParamWidget::applyNodeEntryFilter);
    connect(ui->nodeType, &EditClearWidget::textEdited, this, &QuickParamWidget::applyNodeEntryFilter);
    connect(ui->nodeOutput, &EditClearWidget::textEdited, this, &QuickParamWidget::applyNodeEntryFilter);
    connect(ui->nodeParamName, &EditClearWidget::textEdited, this, &QuickParamWidget::applyNodeEntryFilter);
    connect(ui->nodeParamType, &EditClearWidget::textEdited, this, &QuickParamWidget::applyNodeEntryFilter);

    connect(ui->paramName, &EditClearWidget::textEdited, this, &QuickParamWidget::applyParamEntryFilter);
    connect(ui->paramType, &EditClearWidget::textEdited, this, &QuickParamWidget::applyParamEntryFilter);

    connect(ui->nodeEntries, &QAbstractItemView::clicked, this, &QuickParamWidget::nodeEntryClicked);
}

QuickParamWidget::~QuickParamWidget()
{
    delete ui;
}

void QuickParamWidget::applyNodeEntryFilter()
{
    m_nodeEntriesProxyModel->setFilterWildcard("");
    m_paramEntriesModel->clear();
}

void QuickParamWidget::applyParamEntryFilter()
{
    m_paramEntriesProxyModel->setFilterWildcard("");
}

void QuickParamWidget::nodeEntryClicked(const QModelIndex & index)
{
    QModelIndex idx = m_nodeEntriesProxyModel->mapToSource(index);
    QStandardItem* item = m_nodeEntriesModel->itemFromIndex(idx);
    const QString& name = item->data(Qt::UserRole).toString();
    if(!name.isEmpty())
    {
        ShowParamEntries(name);
    }
}

void QuickParamWidget::ShowParamEntries(const QString& nodeEntryName)
{

    m_paramEntriesModel->clear();

    m_paramEntriesModel->setColumnCount(3);
    m_paramEntriesModel->setHorizontalHeaderLabels({"Name", "Type", "Default"});

    ui->paramEntries->setColumnWidth(0, 250);
    ui->paramEntries->setColumnWidth(1, 100);
    ui->paramEntries->setSortingEnabled(false);

    QStandardItem* root = m_paramEntriesModel->invisibleRootItem();

    const AtNodeEntry* ne = AiNodeEntryLookUp(nodeEntryName.toUtf8());
    if(ne != nullptr)
    {
        AtParamIterator* piter = AiNodeEntryGetParamIterator(ne);
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
            DisplayParamValue(t, pe, item);
            l.append(item);
            root->appendRow(l);
        }

        AiParamIteratorDestroy(piter);
    }

    ui->paramEntries->setSortingEnabled(true);
}

QString QuickParamWidget::GetNodeNameFilter()
{
    return ui->nodeName->text();
}

QString QuickParamWidget::GetNodeTypeFilter()
{
    return ui->nodeType->text();
}

QString QuickParamWidget::GetNodeOutputFilter()
{
    return ui->nodeOutput->text();
}

QString QuickParamWidget::GetNodeParamNameFilter()
{
    return ui->nodeParamName->text();
}

QString QuickParamWidget::GetNodeParamTypeFilter()
{
    return ui->nodeParamType->text();
}

QString QuickParamWidget::GetParamNameFilter()
{
    return ui->paramName->text();
}

QString QuickParamWidget::GetParamTypeFilter()
{
    return ui->paramType->text();
}

void QuickParamWidget::DisplayParamValue(uint8_t type, const AtParamEntry* pe, QStandardItem* item) const
{
    const AtParamValue* defaultValue = AiParamGetDefault(pe);
    QString value = "";
    switch(type)
    {
    case AI_TYPE_BYTE:
        value = QString::number(defaultValue->BYTE());
    break;
    case AI_TYPE_INT:
        value = QString::number(defaultValue->INT());
    break;
    case AI_TYPE_UINT:
        value = QString::number(defaultValue->UINT());
    break;
    case AI_TYPE_FLOAT:
        value = QString::number(defaultValue->FLT());
    break;
    case AI_TYPE_BOOLEAN:
        value = QString::number(defaultValue->BOOL());
    break;
    case AI_TYPE_STRING:
        value =  QString(defaultValue->STR());
    break;
    case AI_TYPE_RGB:
    {
        const AtRGB& rgb = defaultValue->RGB();
        QColor c;
        c.setRgbF(static_cast<double>(rgb.r), static_cast<double>(rgb.g), static_cast<double>(rgb.b));
        item->setBackground(c);
    }
    break;
    case AI_TYPE_RGBA:
    {
       const AtRGBA& rgba = defaultValue->RGBA();
       QColor c;
       c.setRgbF(static_cast<double>(rgba.r), static_cast<double>(rgba.g), static_cast<double>(rgba.b), static_cast<double>(rgba.a));
       item->setBackground(c);
    }
    break;
    case AI_TYPE_VECTOR:
    {
        const AtVector& vec = defaultValue->VEC();
        value = QString::number(vec.x) + ", " + QString::number(vec.y) + ", " + QString::number(vec.y);
    }
    break;
    case AI_TYPE_ENUM:
    {
        AtEnum atEnum = AiParamGetEnum(pe);
        int v = defaultValue->INT();
        value = QString(AiEnumGetString(atEnum, v)) + " ( ";

        int index = 0;
        const char* atEnumEntry = AiEnumGetString(atEnum, index++);
        while(atEnumEntry != nullptr)
        {
            ;
            value += QString(atEnumEntry) + " ";
            atEnumEntry = AiEnumGetString(atEnum, index++);
        }
        value += ")";
    }
    break;
    case AI_TYPE_ARRAY:
    case AI_TYPE_POINTER:
    case AI_TYPE_NODE:
    break;
    default:
    {
       qDebug() << "Param default display not handled: " << type;
    }
    break;
    }

    item->setText(value);
}

