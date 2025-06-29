#ifndef TAGFILTERPROXYMODEL_H
#define TAGFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QStringList>
#include "BooruItemType.h"

class TagFilterProxyModel : public QSortFilterProxyModel {
public:
    TagFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

    void setSearchTag(const QStringList tags, bool enable = false)
    {
        searchTags = tags;
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

        // Search in list of tags inside item
        for(const QString& tag : item_data.tags)
        {
            for(const QString& ref : searchTags)
            {
                if(tag.contains(ref)) {
                    return true;
                }
            }
        }
        return false;
    }

private:
    QStringList searchTags;
    bool searching = false;
};

#endif // TAGFILTERPROXYMODEL_H
