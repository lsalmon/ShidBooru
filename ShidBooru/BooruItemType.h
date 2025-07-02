#ifndef BOORUITEMTYPE_H
#define BOORUITEMTYPE_H

#include <QPixmap>
#include <QString>
#include <QStringListModel>
#include <QSharedPointer>
#include <QByteArray>
#include <QMetaType>


typedef enum {
    STILL_IMG,
    GIF
} itemType;

struct BooruTypeItem {
    QPixmap picture;
    QStringList tags;
    QByteArray gif;
    itemType type;
    BooruTypeItem()
    {
    }
    BooruTypeItem(QPixmap _picture, QStringList _tags, QByteArray _gif = QByteArray(), itemType _type = STILL_IMG) :
        picture(_picture),
        tags(_tags),
        type(_type)
    {
    }
    BooruTypeItem(const BooruTypeItem& type_item) :
        picture(type_item.picture),
        tags(type_item.tags),
        gif(type_item.gif),
        type(type_item.type)
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
