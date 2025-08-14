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
    QByteArray gif;
    QStringList tags;
    itemType type;
    QString extension;
    QString path;
    QVariant sql_id;
/*
    BooruTypeItem()
    {
    }
*/
    BooruTypeItem(QPixmap _picture = QPixmap(), QByteArray _gif = QByteArray(), QStringList _tags = QStringList(), itemType _type = STILL_IMG, QString _extension = "", QString _path = "", QVariant _sql_id = QVariant()) :
        picture(_picture),
        gif(_gif),
        tags(_tags),
        type(_type),
        extension(_extension),
        path(_path),
        sql_id(_sql_id)
    {
    }
    BooruTypeItem(const BooruTypeItem& type_item) :
        picture(type_item.picture),
        gif(type_item.gif),
        tags(type_item.tags),
        type(type_item.type),
        extension(type_item.extension),
        path(type_item.path),
        sql_id(type_item.sql_id)
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
