#include "QSqlQueryHelper.h"

QVariant addItemQuery(int type, const QVariant &path)
{
    QSqlQuery q;
    q.prepare(INSERT_ITEM_SQL);
    q.addBindValue(type);
    q.addBindValue(path);
    q.exec();
    return q.lastInsertId();
}

int getIDFromTagQuery(const QVariant &tag)
{
    QSqlQuery q;
    q.prepare(GET_TAG_ID_SQL);
    q.addBindValue(tag);
/*
    tag.append('\'');
    tag.prepend('\'');
    QString search_query(GET_TAG_ID_SQL);
    search_query.replace(QRegularExpression("<search term>"), tag);
    qDebug() << search_query;
    q.prepare(search_query);
*/

    if(q.exec())
    {
        if(q.first())
        {
            int tag_id = q.value(0).toInt();
            qDebug() << "Found ID "+QString(tag_id)+" for tag "+tag.toString();
    qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str() << "     " << q.value(0) << q.value(1);
            return tag_id;
            //while(q.next())
            //{

            //}
        }
    }
    else
    {
        qDebug() << "Failed to get tag id from tag "+tag.toString()+" error "+q.lastError().text();
    }
    //qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str();
    return -1;
}

bool getItemsFromTagQuery(int tag_id, QVector<BooruTypeItem> &item_vector)
{
    QSqlQuery q;
    QVariant bind_val(tag_id);
    q.prepare(GET_ITEMS_FOR_TAG_SQL);
    q.addBindValue(bind_val);
    if(q.exec())
    {
        while(q.next())
        {
            BooruTypeItem item;
            int item_id = q.value(0).toInt();
            item.sql_id = item_id;
            item.type = itemType(q.value(1).toInt());
            item.path = q.value(2).toString();
            //qDebug() << "Found item "+QString(item_id)+"  "+item.path+" for tag "+QString(tag_id);
            item_vector.push_back(item);
        }
        return true;
    }
    else
    {
        qDebug() << "Failed to get item from tag id "+QString(tag_id);
    }
    qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str() << "     " << q.value(0) << q.value(1);
    return false;
}

bool getItemFromIDQuery(const QVariant &id_item, BooruTypeItem &item)
{
    QSqlQuery q;
    q.prepare(GET_ITEM_FROM_ID_SQL);
    q.addBindValue(id_item);
    if(q.exec())
    {
        if(q.first())
        {
            item.type = itemType(q.value(0).toInt());
            item.path = q.value(1).toString();

            QImage image(item.path);
            item.picture = QPixmap::fromImage(image);

            if(item.type == GIF)
            {
                QFile file(item.path);
                if(file.open(QIODevice::ReadOnly))
                {
                    item.gif = file.readAll();
                }
            }
            //qDebug() << "Got item type "+QString(item.type)+"  path  "+item.path+" for id_item "+QString(id_item);
            return true;
        }
    }
    else
    {
        qDebug() << "Failed to get item from id_item "+QString(id_item.toInt());
    }
    qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str() << "     " << q.value(0) << q.value(1);
    
    return false;
}

bool getTagsFromItemQuery(const QVariant &id_item, QStringList &tags_list)
{
    QSqlQuery q;
    q.prepare(GET_TAGS_FOR_ITEM_SQL);
    q.addBindValue(id_item);
    if(q.exec())
    {
        while(q.next())
        {
            QString tag_string = q.value(1).toString();
            qDebug() << "Found tag "+tag_string+" for item "+QString(id_item.toInt());
            tags_list.append(tag_string);
        }
    qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str() << "     " << q.value(0) << q.value(1);
        return true;
    }
    else
    {
        qDebug() << "Failed to get tag from item id "+QString(id_item.toInt());
    }
    qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str() << "     " << q.value(0) << q.value(1);
    return false;
}

QVariant addTagQuery(const QString &tag)
{
    QSqlQuery q;
    q.prepare(INSERT_TAG_SQL);
    q.addBindValue(tag);
    if(q.exec())
    {
        qDebug() << "Added tag "+tag;
    }
    else
    {
        qDebug() << "Failed to add tag "+tag;
    }
    return q.lastInsertId();
}

bool removeTagQuery(const QString &tag)
{
    QSqlQuery q;
    q.prepare(REMOVE_TAG_SQL);
    q.addBindValue(tag);
    if(q.exec())
    {
        qDebug() << "Removed tag "+tag;
        return true;
    }
    else
    {
        qDebug() << "Failed to remove tag "+tag;
qDebug() << "Cannot get items for tag "+tag+" "+q.lastError().text()+" "+q.executedQuery()+"   "+q.boundValue(0).toString();
    }
    return false;
}

void addLinkQuery(const QVariant &id_item, const QVariant &id_tag)
{
    QSqlQuery q;
    q.prepare(INSERT_LINK_SQL);
    q.addBindValue(id_item);
    q.addBindValue(id_tag);
    q.exec();
qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str() << "     " << q.value(0) << q.value(1);
}

bool removeLinkQuery(const QVariant &id_item, const QVariant &id_tag)
{
    QSqlQuery q;
    q.prepare(REMOVE_LINK_SQL);
    q.addBindValue(id_item);
    q.addBindValue(id_tag);
    if(q.exec())
    {
        qDebug() << "Removed link "+QString(id_item.toInt())+" -> "+QString(id_tag.toInt());
        return true;
    }
    else
    {
        qDebug() << "Failed to remove link "+QString(id_item.toInt())+" -> "+QString(id_tag.toInt());
    }
    return false;
}

int checkDuplicateTagQuery(const QString &tag)
{
    QSqlQuery q;
    q.prepare(CHECK_TAG_EXISTENCE_SQL);
    q.addBindValue(tag);

    if(q.exec())
    {
        if(q.first())
        {
            qDebug() << "Tag "+tag+" already exists";
            return q.value(0).toInt();
        }
    }
    else
    {
        qDebug() << "Failed to check if tag "+tag+" exists";
    }
    return -1;
}

int checkDuplicateLinkQuery(const QVariant &id_tag, const QVariant &id_item)
{
    QSqlQuery q;
    q.prepare(CHECK_LINK_EXISTENCE_SQL);
    q.addBindValue(id_tag);
    q.addBindValue(id_item);

    if(q.exec())
    {
        if(q.first())
        {
            qDebug() << "Link "+id_tag.toString()+"->"+id_item.toString()+" already exists";
            return q.value(0).toInt();
        }
    }
    else
    {
        qDebug() << "Failed to check if link "+id_tag.toString()+"->"+id_item.toString()+" exists";
    }
qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str() << "     " << q.value(0) << q.value(1);
    return -1;
}
