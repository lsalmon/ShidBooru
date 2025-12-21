#ifndef BOORUITEMTYPE_H
#define BOORUITEMTYPE_H

#include <QImage>
#include <QString>
#include <QStringListModel>
#include <QSharedPointer>
#include <QByteArray>
#include <QMetaType>

typedef enum {
    UNINIT,
    STILL_IMG,
    GIF,
    MOVIE
} itemType;

struct BooruTypeItem {
    itemType type;
    QString extension;
    QString path;
    QVariant sql_id;
    QImage thumbnail;

    BooruTypeItem(itemType _type = STILL_IMG, QString _extension = "", QString _path = "", QVariant _sql_id = QVariant(), QImage _thumbnail = QImage()) :
        type(_type),
        extension(_extension),
        path(_path),
        sql_id(_sql_id),
        thumbnail(_thumbnail)
    {
    }
    BooruTypeItem(const BooruTypeItem& type_item) :
        type(type_item.type),
        extension(type_item.extension),
        path(type_item.path),
        sql_id(type_item.sql_id),
        thumbnail(type_item.thumbnail)       
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
