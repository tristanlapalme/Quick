#ifndef DATABASE_H
#define DATABASE_H

#include "color_fl.h"

#include <ai.h>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QString>

namespace QtNodes { class Node; }
struct ArnoldNode;

struct ParamValue
{
    long i{0};
    //QVector<bool> b;
    bool b{false};
    float f{0.0};
    AtRGB rgb;
    AtRGBA rgba;
    AtVector v;
    AtVector2 v2;
    //QVector<QString> s;
    std::vector<QString*> s;
    std::vector<ArnoldNode*> vn;

    void FromAtValue(const AtParamEntry* pe, uint8_t t);
};

struct ArnoldParamEntry
{
    ArnoldParamEntry(const QString& n, const QString& tn, uint8_t t, uint8_t at=AI_TYPE_NONE) : name(n), paramTypeName(tn), paramType(t), arrayType(at) {}
    QString name;
    QString paramTypeName;
    uint8_t paramType;
    uint8_t arrayType;
    ParamValue value;
    QStringList valueEnum;
};

struct ArnoldNodeEntry
{
    ArnoldNodeEntry(const QString& n, const QString& t, const QString& o, bool d) : name(n), nodeType(t), output(o), deprecated(d) {}

    QString name;
    QString nodeType;
    QString output;
    bool deprecated;

    QMap<QString, ArnoldParamEntry*> params;
};

struct ArnoldNode
{
    ArnoldNode(const QString& n, ArnoldNodeEntry* ne) : name(n), nodeEntry(ne) {}

    QString name;
    ArnoldNodeEntry* nodeEntry;

    QMap<QString, ParamValue> paramValues;

    QtNodes::Node* sceneNode{nullptr};
};

struct Scene
{
    void clear()
    {
        nodes.clear();
        sceneNodeEntries.clear();
    }

    QString name;
    QMap<QString, ArnoldNode*> nodes;
    QVector<QMap<QString, QVector<ArnoldNode*>>> sceneNodeEntries;
};

class Database : public QObject
{
    Q_OBJECT

public:
    static Database& GetInstance();

    void Populate();
    bool LoadScene(const QString& filename, bool deep = false);

    const QMap<QString, ArnoldNodeEntry*>& GetNodeEntries() const;
    ArnoldNodeEntry* GetNodeEntry(const QString& name) const;
    const QMap<QString, ArnoldNode*>& GetNodes() const;
    ArnoldNode* GetNode(const QString& name) const;

    const QStringList& GetTypes() const;
    const Scene& GetScene() const;

    Color_fl* GetPixels();
    void BuildPixels(int size);
    void EmitPixelsReady();

signals:
    void PixelsReady();

private:
    Database();

    QStringList m_types;
    QMap<QString, ArnoldNodeEntry*> m_nodeEntries;
    Scene m_scene;

    std::vector<Color_fl> m_pixels;
};

#endif // DATABASE_H
