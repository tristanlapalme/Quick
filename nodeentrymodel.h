#ifndef NODEENTRYMODEL_H
#define NODEENTRYMODEL_H

#include <QStandardItemModel>

class NodeEntryModel : public QStandardItemModel
{
    Q_OBJECT

public:
    NodeEntryModel(QObject *parent = nullptr);
    virtual ~NodeEntryModel() override;

    void Init();
};

#endif // NODEENTRYMODEL_H
