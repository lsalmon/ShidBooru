#ifndef BOORUITEMTYPE_H
#define BOORUITEMTYPE_H

#include <QPixmap>
#include <QString>
#include <QStringListModel>
#include <QSharedPointer>
#include <QMetaType>

struct BooruTypeItem {
    QPixmap picture;
    QSharedPointer<QStringListModel> tags;
    BooruTypeItem()
    {
    }
    BooruTypeItem(QPixmap _picture, QSharedPointer<QStringListModel> _tags) :
        picture(_picture),
        tags(_tags)
    {
    }
    BooruTypeItem(const BooruTypeItem& type_item) :
        picture(type_item.picture),
        tags(type_item.tags)
    {
    }
    auto operator=(const BooruTypeItem& type_item) -> BooruTypeItem&
    {
        BooruTypeItem temp = type_item;
        std::swap(temp, *this);
        return *this;
    }
    ~BooruTypeItem()
    {
    }
};

Q_DECLARE_METATYPE(BooruTypeItem)

#endif // BOORUITEMTYPE_H
