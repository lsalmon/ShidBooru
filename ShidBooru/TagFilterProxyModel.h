#ifndef TAGFILTERPROXYMODEL_H
#define TAGFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QStringList>
#include <QVector>
#include "QSqlQueryHelper.h"
#include "BooruItemType.h"

class TagFilterProxyModel : public QSortFilterProxyModel {
public:
    TagFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

    void setSqlIDFilter(const QVector<int> itemSqlIDs, bool enable = false)
    {
        databaseIDs = itemSqlIDs;
        searching = enable;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if(!searching) {
            return true;
        }
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        QVariant item_var = index.data(Qt::UserRole);
        BooruTypeItem item_data = item_var.value<BooruTypeItem>();

        // Test if the item corresponds to the ID in the links database
        return databaseIDs.contains(item_data.sql_id.toInt());
    }

private:
    QVector<int> databaseIDs;
    bool searching = false;
};

#endif // TAGFILTERPROXYMODEL_H
