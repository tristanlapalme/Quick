#include "database.h"
#include "arnoldriver.h"

#include <QtCore/QThread>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>

#include <QtDebug>

class MyThread : public QThread
{
public:
    explicit MyThread(const QString filename, QObject *parent = nullptr) : QThread(parent), m_filename(filename) {}
    virtual ~MyThread() {qDebug() << "Die";}
    void run()
    {
        AiBegin();
        AiMsgSetConsoleFlags(AI_LOG_ALL);
        InitializeArnoldDriver();
        AiASSLoad(m_filename.toUtf8());
        AiRender(AI_RENDER_MODE_CAMERA);
        AiEnd();

        this->deleteLater();
    }

    QString m_filename;
};

void ParamValue::FromAtValue(const AtParamEntry* pe, uint8_t t)
{
    const AtParamValue* v = AiParamGetDefault(pe);
    switch(t)
    {
    case AI_TYPE_BYTE: i = v->BYTE(); break;
    case AI_TYPE_INT: i = v->INT(); break;
    case AI_TYPE_UINT: i = static_cast<long>(v->UINT()); break;
    //case AI_TYPE_BOOLEAN: b.push_back(v->BOOL()); break;
    case AI_TYPE_BOOLEAN: b = v->BOOL(); break;
    case AI_TYPE_FLOAT: f = v->FLT(); break;
    //case AI_TYPE_STRING: s = v->STR().c_str(); break;
    case AI_TYPE_STRING: s.push_back(new QString(v->STR().c_str())); break;
    case AI_TYPE_RGB: rgb = v->RGB(); break;
    case AI_TYPE_RGBA: rgba = v->RGBA(); break;
    case AI_TYPE_VECTOR: this->v = v->VEC(); break;
    case AI_TYPE_ENUM:
    {
        s.push_back(new QString(AiEnumGetString(AiParamGetEnum(pe), v->INT())));
        i = v->INT();
        break;
    }
    }
}

Database& Database::GetInstance()
{
    static Database instance;
    return instance;
}

Database::Database()
{

}

void Database::Populate()
{
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

        ArnoldNodeEntry* node = new ArnoldNodeEntry(name, type, output, deprecated);
        m_nodeEntries[name] = node;

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
                const AtParamValue* default_value = AiParamGetDefault(pe);
                AtArray* arr = default_value->ARRAY();
                at = AiArrayGetType(arr);
                QString atName = AiParamGetTypeName(at);

                pType = atName + "[]";
            }

            ParamValue pv;
            pv.FromAtValue(pe, t);
            ArnoldParamEntry* param = new ArnoldParamEntry(pName, pType, t, at);

            if(t == AI_TYPE_ENUM)
            {
                AtEnum atEnum = AiParamGetEnum(pe);
                int index = 0;
                const char* atEnumEntry = AiEnumGetString(atEnum, index++);
                while(atEnumEntry != nullptr)
                {
                    param->valueEnum.append(atEnumEntry);
                    atEnumEntry = AiEnumGetString(atEnum, index++);
                }
            }

            param->value = pv;
            node->params[pName] = param;
        }

        if(!m_types.contains(type))
        {
            m_types.append(type);
        }

        AiParamIteratorDestroy(piter);
    }

    AiNodeEntryIteratorDestroy(node_entry_it);

    AiEnd();

    m_types.sort();
}

const QMap<QString, ArnoldNodeEntry*>& Database::GetNodeEntries() const
{
    return m_nodeEntries;
}

ArnoldNodeEntry* Database::GetNodeEntry(const QString& name) const
{
    return m_nodeEntries[name];
}

const QMap<QString, ArnoldNode*>& Database::GetNodes() const
{
    return m_scene.nodes;
}

const QStringList& Database::GetTypes() const
{
    return m_types;
}

const Scene& Database::GetScene() const
{
    return m_scene;
}


Color_fl* Database::GetPixels()
{
    return m_pixels.data();
}

void Database::BuildPixels(int size)
{
    m_pixels.resize(size);
}

void Database::EmitPixelsReady()
{
    emit PixelsReady();
}

bool Database::LoadScene(const QString& filename, bool deep)
{
    m_scene.name = filename;
    m_scene.clear();
    const QStringList& types = GetTypes();
    const int typeCount = types.size();
    m_scene.sceneNodeEntries.resize(typeCount);

    AiBegin();
    AiMsgSetConsoleFlags(AI_LOG_ALL);
    AiASSLoad(filename.toUtf8());

    static const char nameLabel[] = "name";

    AtNodeIterator* iter = AiUniverseGetNodeIterator(nullptr, AI_NODE_ALL);
    while (!AiNodeIteratorFinished(iter))
    {
        AtNode* atNode = AiNodeIteratorGetNext(iter);
        if (atNode == nullptr)
            continue;

        QString name = AiNodeGetStr(atNode, nameLabel).c_str();
        const AtNodeEntry* ne = AiNodeGetNodeEntry(atNode);
        QString nen = AiNodeEntryGetName(ne);
        ArnoldNodeEntry* nEntry = m_nodeEntries[nen];
        ArnoldNode* node = new ArnoldNode(name, nEntry);
        m_scene.nodes[name] = node;

        int index = types.indexOf(nEntry->nodeType);
        QMap<QString, QVector<ArnoldNode*>>& sne = m_scene.sceneNodeEntries[index];
        sne[nen].append(node);

    }
    AiNodeIteratorDestroy(iter);

    iter = AiUniverseGetNodeIterator(nullptr, AI_NODE_ALL);
    while (!AiNodeIteratorFinished(iter))
    {
        AtNode* atNode = AiNodeIteratorGetNext(iter);
        if (atNode == nullptr)
            continue;

        QString name = AiNodeGetStr(atNode, nameLabel).c_str();
        const AtNodeEntry* ne = AiNodeGetNodeEntry(atNode);
        QString nen = AiNodeEntryGetName(ne);
        ArnoldNodeEntry* nEntry = m_nodeEntries[nen];

        ArnoldNode* node = m_scene.nodes[name];

        for(ArnoldParamEntry* pe : nEntry->params)
        {
            if(AiNodeIsLinked(atNode, pe->name.toUtf8()))
            {
                AtNode* src = AiNodeGetLink(atNode, pe->name.toUtf8());
                if(src != nullptr)
                {
                    QString srcName = AiNodeGetStr(src, nameLabel).c_str();
                    ArnoldNode* srcNode = m_scene.nodes[srcName];
                    if(srcNode != nullptr)
                    {
                        ParamValue pv;
                        pv.vn.push_back(srcNode);
                        node->paramValues[pe->name] = pv;
                    }
                }
            }
            else
            {
                switch(pe->paramType)
                {
                case AI_TYPE_BYTE:
                {
                    int i = AiNodeGetByte(atNode, pe->name.toUtf8());
                    if(pe->value.i == i)
                        continue;

                    ParamValue pv;
                    pv.i = i;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_INT:
                {
                    int i = AiNodeGetInt(atNode, pe->name.toUtf8());
                    if(pe->value.i == i)
                        continue;

                    ParamValue pv;
                    pv.i = i;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_UINT:
                {
                    long l = static_cast<long>(AiNodeGetUInt(atNode, pe->name.toUtf8()));
                    if(pe->value.i == l)
                        continue;
                    ParamValue pv;
                    pv.i = l;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_FLOAT:
                {
                    float f = AiNodeGetFlt(atNode, pe->name.toUtf8());
                    if(abs(pe->value.f - f) < 0.00001f)
                        continue;

                    ParamValue pv;
                    pv.f = f;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_BOOLEAN:
                {
                    bool b = AiNodeGetBool(atNode, pe->name.toUtf8());
                    //if(!pe->value.b.empty() && pe->value.b.front() == b)
                    if(pe->value.b == b)
                        continue;

                    ParamValue pv;
                    //pv.b.push_back(b);
                    pv.b = b;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_STRING:
                {
                    QString s = AiNodeGetStr(atNode, pe->name.toUtf8()).c_str();
                    if(!pe->value.s.empty() && pe->value.s.front() == s)
                    //if(pe->value.s == s)
                        continue;

                    ParamValue pv;
                    //pv.s.push_back(s);
                    pv.s.push_back(new QString(s));
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_RGB:
                {
                    AtRGB rgb = AiNodeGetRGB(atNode, pe->name.toUtf8());
                    if(pe->value.rgb == rgb)
                        continue;

                    ParamValue pv;
                    pv.rgb = rgb;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_RGBA:
                {
                    AtRGBA rgba = AiNodeGetRGBA(atNode, pe->name.toUtf8());
                    if(pe->value.rgba == rgba)
                        continue;

                    ParamValue pv;
                    pv.rgba = rgba;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_NODE:
                {
                    AtNode* src = static_cast<AtNode*>(AiNodeGetPtr(atNode, pe->name.toUtf8()));
                    if(src != nullptr)
                    {
                        QString srcName = AiNodeGetStr(src, nameLabel).c_str();
                        ArnoldNode* srcNode = m_scene.nodes[srcName];
                        ParamValue& pv = node->paramValues[pe->name];
                        pv.vn.push_back(srcNode);
                    }
                    break;
                }
                case AI_TYPE_VECTOR:
                {
                    AtVector vec = AiNodeGetVec(atNode, pe->name.toUtf8());
                    ParamValue pv;
                    pv.v = vec;
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_ENUM:
                {
                    int i = AiNodeGetInt(atNode, pe->name.toUtf8());
                    if(pe->value.i == i)
                    //if(pe->value.s == s)
                        continue;

                    ParamValue pv;
                    //pv.s.push_back(s);
                    pv.s.push_back(new QString(pe->valueEnum[i]));
                    node->paramValues[pe->name] = pv;
                    break;
                }
                case AI_TYPE_ARRAY:
                {


                    AtArray* array = AiNodeGetArray(atNode, pe->name.toUtf8());
                    switch(pe->arrayType)
                    {
                    case AI_TYPE_NODE:
                    {
                        uint32_t num = AiArrayGetNumElements(array);
                        for (uint32_t i = 0; i < num; ++i)
                        {
                            AtNode* elem = static_cast<AtNode*>(AiArrayGetPtr(array, i));
                            if (elem != nullptr)
                            {
                                QString srcName = AiNodeGetStr(elem, nameLabel).c_str();
                                ArnoldNode* srcNode = m_scene.nodes[srcName];
                                ParamValue& pv = node->paramValues[pe->name];
                                pv.vn.push_back(srcNode);
                            }
                        }
                        break;
                    }
                    }
                    break;
                }
                default:
                {
                    qDebug() << "Param load not handled: " << pe->name << " of type " << pe->paramTypeName;
                    break;
                }
                }
            }
        }
    }

    AiEnd();

    if(deep)
    {
        BuildPixels(400*400);
        MyThread* thread = new MyThread(filename);
        thread->start();
    }


    /*
    AiBegin();
    AiMsgSetConsoleFlags(AI_LOG_ALL);

    InitializeArnoldDriver();
    AiASSLoad(filename.toUtf8());
    if(deep)
    {
        BuildPixels(400*400);
        AiRender(AI_RENDER_MODE_CAMERA);
    }

    AiEnd();
    */

    return true;
}
