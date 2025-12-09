#ifndef QSQLQUERYHELPER_H
#define QSQLQUERYHELPER_H

#include <QImageReader>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QVector>
#include <QStringList>
#include <QQueue>
#include "BooruItemType.h"

const auto ITEM_SQL = QLatin1String(R"(
    CREATE table items(id_i INTEGER PRIMARY KEY, type integer, path varchar)
    )");

const auto TAG_SQL = QLatin1String(R"(
    CREATE table tags(id_t INTEGER PRIMARY KEY, tag varchar)
    )");

const auto LINK_SQL = QLatin1String(R"(
    CREATE table links(id_item INTEGER, id_tag INTEGER, PRIMARY KEY (id_item, id_tag), FOREIGN KEY (id_item) REFERENCES items(id_i) ON DELETE CASCADE, FOREIGN KEY (id_tag) REFERENCES tags(id_t) ON DELETE CASCADE)
    )");

const auto INSERT_ITEM_SQL = QLatin1String(R"(
    INSERT into items(type, path) values(?, ?)
    )");

const auto GET_ITEM_FROM_ID_SQL = QLatin1String(R"(
    SELECT i.type, i.path
    FROM items i
    WHERE i.id_i = ?
    )");

const auto GET_TAG_ID_SQL = QLatin1String(R"(
    SELECT t.id_t
    FROM tags t
    WHERE t.tag = ?
    )");

const auto GET_ITEMS_FOR_SINGLE_TAG_SQL = QLatin1String(R"(
    SELECT i.id_i, i.type, i.path
    FROM items i
    JOIN links l ON i.id_i = l.id_item
    JOIN tags t ON t.id_t = l.id_tag
    WHERE t.tag = ?
    )");

const auto GET_ITEMS_FOR_TAGS_SQL_TEMPLATE = QLatin1String(R"(
    SELECT i.id_i, i.type, i.path
    FROM items i
    JOIN links l ON i.id_i = l.id_item
    JOIN tags t ON t.id_t = l.id_tag
    WHERE t.tag
    )");

const auto GET_ITEMS_FOR_TAGS_SQL_OR_SEARCH = GET_ITEMS_FOR_TAGS_SQL_TEMPLATE
+ QLatin1String(R"(
    IN :list_or
    )");

const auto GET_ITEMS_FOR_TAGS_SQL_AND_SEARCH = GET_ITEMS_FOR_TAGS_SQL_OR_SEARCH
+ QLatin1String(R"(
    GROUP BY i.id_i
    HAVING COUNT(DISTINCT t.tag) = :num_tag
    )");

const auto GET_ITEMS_FOR_TAGS_SQL_WILDCARD_SEARCH_SUFFIX = QLatin1String(R"(
    LIKE :wildcard
    )");

const auto GET_ITEMS_FOR_TAGS_SQL_EXCLUDE_TAG_SEARCH = QLatin1String(R"(
    AND i.id_i NOT IN (
        SELECT i_ex.id_i
        FROM items i_ex
        JOIN links l ON i_ex.id_i = l.id_item
        JOIN tags t ON t.id_t = l.id_tag
        WHERE t.tag = :tag_exclude
    )
    )");

const auto GET_TAGS_FOR_ITEM_SQL = QLatin1String(R"(
    SELECT tags.id_t, tags.tag
    FROM links l
    JOIN tags ON l.id_tag = tags.id_t
    WHERE l.id_item = ?
    )");

const auto INSERT_TAG_SQL = QLatin1String(R"(
    INSERT INTO tags(tag) values(?)
    )");

const auto INSERT_LINK_SQL = QLatin1String(R"(
    INSERT INTO links(id_item, id_tag) values(?, ?)
    )");

const auto CHECK_TAG_EXISTENCE_SQL = QLatin1String(R"(
    SELECT 1
    FROM tags t
    WHERE t.tag = ?
    )");

const auto CHECK_LINK_EXISTENCE_SQL = QLatin1String(R"(
    SELECT 1
    FROM links l
    WHERE l.id_tag = ? AND l.id_item = ?
    )");

const auto REMOVE_ITEM_SQL = QLatin1String(R"(
    DELETE FROM items
    WHERE id_i = ?
    )");

const auto REMOVE_TAG_SQL = QLatin1String(R"(
    DELETE FROM tags
    WHERE tag = ?
    )");

const auto REMOVE_LINK_SQL = QLatin1String(R"(
    DELETE FROM links
    WHERE id_item = ? AND id_tag = ?
    )");

const auto DUMP_TO_FILE = QLatin1String(R"(
    VACUUM INTO ?
    )");

const auto DUMP_ALL_ITEMS_SQL = QLatin1String(R"(
    SELECT *
    FROM items
    )");

QVariant addItemQuery(int type, const QVariant &path);

int getIDFromTagQuery(const QVariant &tag);

bool getItemsFromCustomQuery(QString query, QVector<BooruTypeItem> &item_vector);
bool getItemsFromSingleTagQuery(QString tag, QVector<BooruTypeItem> &item_vector);
bool dumpItemsQuery(QQueue<BooruTypeItem> &item_vector);
bool getItemFromIDQuery(int id_item, BooruTypeItem &item);

bool getTagsFromItemQuery(const QVariant &id_item, QStringList &tags_list);

bool removeItemQuery(const QVariant &id_item);

QVariant addTagQuery(const QString &tag);
bool removeTagQuery(const QString &tag);

void addLinkQuery(const QVariant &id_item, const QVariant &id_tag);
bool removeLinkQuery(const QVariant &id_item, const QVariant &id_tag);

int checkDuplicateTagQuery(const QString &tag);
int checkDuplicateLinkQuery(const QVariant &id_tag, const QVariant &id_item);

#endif // QSQLQUERYHELPER_H
