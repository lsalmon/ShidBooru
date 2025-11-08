#include "BooruMenu.h"
#include "ui_BooruMenu.h"
#include "QSqlQueryHelper.h"
#include <QImageReader>
#include <algorithm>

// For the webm thumbnail
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video.hpp"
#include "opencv2/imgproc/imgproc.hpp"

static QSqlDatabase db;
static int key_pressed = Qt::Key_Clear;

static void removeDb()
{
    db.close();
    QSqlDatabase::removeDatabase("qt_sql_default_connection");
    qDebug() << "DB removed";
}

void BooruMenu::BooruMenuUISetup(void)
{
    proxyModel = new TagFilterProxyModel(this);
    proxyModel->setSourceModel(&model);

    // Enable multiple selection by user (shift/ctrl + click/arrow)
    ui->listViewFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listViewFiles->setModel(proxyModel);
    ui->listViewFiles->setViewMode(QListView::ListMode);
    ui->listViewFiles->setIconSize(QSize(128,128));
    ui->listViewFiles->setResizeMode(QListView::Adjust);
    ui->listViewTags->setModel(&tagModel);

    auto list_actions = ui->toolBar->actions();
    foreach(QAction* item, list_actions){
        qDebug() << item->text();
        if(item->text() == "Find Image") {
            connect(item, &QAction::triggered, this, &BooruMenu::findImage);
        }
        if(item->text() == "Add Image") {
            connect(item, &QAction::triggered, this, &BooruMenu::addImage);
        }
        if(item->text() == "Remove Image") {
            connect(item, &QAction::triggered, this, &BooruMenu::removeImage);
        }
        if(item->text() == "Export to Booru File") {
            connect(item, &QAction::triggered, this, &BooruMenu::exportToBooruFile);
        }
    }

    connect(ui->listViewFiles, &QListView::clicked, this, &BooruMenu::viewClickedItemTag);
    connect(ui->listViewFiles, &QListView::doubleClicked, this, &BooruMenu::viewDoubleClickedItem);
    connect(ui->listViewFiles->selectionModel(), &QItemSelectionModel::currentChanged, this,
        &BooruMenu::currentChanged);
    // eventFilter function catches mouse events on viewport and key presses on listView
    ui->listViewFiles->viewport()->installEventFilter(this);
    ui->listViewFiles->installEventFilter(this);

    // Refresh tag list to first item if it exists
    QModelIndex first_idx = model.index(0, 0);
    if(first_idx.isValid())
    {
        // Force select first item
        ui->listViewFiles->setCurrentIndex(first_idx);
        ui->listViewFiles->selectionModel()->select(first_idx, QItemSelectionModel::Select);

        BooruMenu::viewClickedItemTag(first_idx);
    }
}

BooruMenu::BooruMenu(QWidget *parent, QString _file_or_db_path, BooruInitType type) :
    QMainWindow(parent),
    ui(new Ui::BooruMenu),
    file_or_db_path(_file_or_db_path)
{
    ui->setupUi(this);
    db = QSqlDatabase::addDatabase("QSQLITE");

    if(type == CREATE)
    {
        // Store table in RAM
        db.setDatabaseName(":memory:");

        if(!db.open())
        {
            DisplayWarningMessage("Creating db failed with "+db.lastError().text());
            this->close();
            return ;
        }
    }
    else
    {
        // Load database from file
        db.setDatabaseName(_file_or_db_path);

        if(!db.open())
        {
            DisplayWarningMessage("Importing db failed with "+db.lastError().text());
            this->close();
            return ;
        }
    }

    // Support for foreign keys
    QSqlQuery q;
    q.exec("PRAGMA foreign_keys = ON;");

    if(type == CREATE)
    {
        QStringList tables = db.tables();
        if (tables.contains("items", Qt::CaseInsensitive)
            && tables.contains("tags", Qt::CaseInsensitive)
            && tables.contains("links", Qt::CaseInsensitive))
        {
            DisplayWarningMessage("db is already full, abort");
            removeDb();
            this->close();
            return ;
        }

        if (!q.exec(ITEM_SQL))
        {
            DisplayWarningMessage("Creating Item table failed with "+q.lastError().text());
            removeDb();
            this->close();
            return ;
        }
        if (!q.exec(TAG_SQL))
        {
            DisplayWarningMessage("Creating Tag table failed with "+q.lastError().text());
            removeDb();
            this->close();
            return ;
        }
        if (!q.exec(LINK_SQL))
        {
            DisplayWarningMessage("Creating Item<->Tag table failed with "+q.lastError().text());
            removeDb();
            this->close();
            return ;
        }

        // Load items from user directory or from single file
        QFileInfo fileinfo(file_or_db_path);
        if(fileinfo.isDir())
        {
            BrowseFiles(QDir(file_or_db_path));
        }
        else
        {
            LoadFile(fileinfo, -1);
        }
    }
    else
    {
        QStringList tables = db.tables();
        if (!tables.contains("items", Qt::CaseInsensitive)
            || !tables.contains("tags", Qt::CaseInsensitive)
            || !tables.contains("links", Qt::CaseInsensitive))
        {
            DisplayWarningMessage("db is missing tables, abort");
            removeDb();
            this->close();
            return ;
        }

        importBooruFromFile();
    }

    BooruMenuUISetup();
}

void BooruMenu::findImage(void)
{
    // If a search window is already there, dont create another
    if(searchInProgress)
    {
        return ;
    }
    SearchTagDialog* tagDialog = new SearchTagDialog(this);
    tagDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(tagDialog, &SearchTagDialog::searchRequest, this, &BooruMenu::searchImage);
    connect(tagDialog, &SearchTagDialog::accepted, this, &BooruMenu::resetSearchImage);
    connect(tagDialog, &SearchTagDialog::rejected, this, &BooruMenu::resetSearchImage);
    connect(tagDialog, &SearchTagDialog::finished, this, &BooruMenu::searchImageFinished);
    searchInProgress = true;
    tagDialog->show();
}

void BooruMenu::addImage(void)
{
    SelectFilesDialog dialbox(this);
    if(dialbox.exec() == QDialog::Accepted)
    {
        QFileInfo fileinfo(dialbox.selected);
        if(fileinfo.isDir())
        {
            BrowseFiles(QDir(dialbox.selected));
        }
        else
        {
            LoadFile(fileinfo, -1);
        }
    }
}

void BooruMenu::removeImage(void)
{
    const QModelIndexList idx_list = this->ui->listViewFiles->selectionModel()->selectedIndexes();
    std::for_each(idx_list.begin(), idx_list.end(), [&](const QModelIndex &idx_proxy) {
        return proxyModel->mapToSource(idx_proxy);
    });
    QString item_desc_list;
    foreach(QModelIndex idx, idx_list)
    {
        QString item_desc = idx.data().toString();
        if(!idx.isValid())
        {
            DisplayWarningMessage("Could not find the item at index "+item_desc);
        }
        else
        {
            item_desc_list += item_desc;
            item_desc_list += QString(" ");
        }
    }

    QMessageBox::StandardButton confirm = QMessageBox::question(this, "Confirm suppression", "Delete item(s) "+item_desc_list+" ?", QMessageBox::Yes|QMessageBox::No);

    if(confirm == QMessageBox::Yes)
    {
        foreach(QModelIndex idx, idx_list)
        {
            QVariant item_var = idx.data(Qt::UserRole);
            BooruTypeItem item_data = item_var.value<BooruTypeItem>();

            QStringList tags_list;

            getTagsFromItemQuery(item_data.sql_id, tags_list);

            // Remove links to item
            for(const QString &tag : tags_list)
            {
                qDebug() << "Remove item : item " << tag;
                int id_tag = getIDFromTagQuery(tag);
                if(id_tag >= 0) {
                    if(removeLinkQuery(item_data.sql_id, id_tag))
                    {
                        qDebug() << "Remove link from "+QString(item_data.sql_id.toInt())+" to "+id_tag;
                        // If link was the last to use the tag, also remove tag
                        QVector<BooruTypeItem> items;
                        getItemsFromSingleTagQuery(tag, items);

                        if(items.empty()) {
                            qDebug() << "Remove tag "+tag+" completely";
                            removeTagQuery(tag);
                        }
                    }
                    else
                    {
                        qDebug() << "Failed to remove link from "+QString(item_data.sql_id.toInt())+" to "+id_tag;
                    }
                }
            }

            // Remove item from db
            if(!proxyModel->removeRow(idx.row()) || !removeItemQuery(item_data.sql_id)) {
                DisplayWarningMessage("Could not delete the item "+idx.data().toString());
                return ;
            }
        }

        // Reset tag window
        BooruMenu::ClearItemTag();
    }
}

void BooruMenu::searchQueryParser(QStringList tag_list, QVector<BooruTypeItem> &items)
{
    QVector<QString> tags_or;
    QVector<QString> tags_exclude;
    QVector<QString> tags_wildcard;
    QVector<QString> tags_and;
    QString query(GET_ITEMS_FOR_TAGS_SQL_TEMPLATE);
    QString query_exclude("");

    for(QString tag : tag_list)
    {
        // Skip malformed tags
        if(tag.isEmpty() || tag.contains(" "))
        {
            continue;
        }

        // OR
        if(tag[0] == '~')
        {
            tag.remove(0, 1);
            tags_or.push_back(tag);
        }
        // NOT
        else if(tag[0] == '-')
        {
            tag.remove(0, 1);
            tags_exclude.push_back(tag);
        }
        // Wildcard
        else if(tag.contains("*"))
        {
            tag.replace('*', "%", Qt::CaseInsensitive);
            tags_wildcard.push_back(tag);
        }
        // Default is AND
        else
        {
            tags_and.push_back(tag);
        }
    }

    qDebug() << "Search tags total : ";
    for(const QString &tag : tags_or)
    {
        qDebug() << "OR tag : " << tag;
    }
    for(const QString &tag : tags_exclude)
    {
        qDebug() << "Exclude tag : " << tag;
    }
    for(const QString &tag : tags_wildcard)
    {
        qDebug() << "Wildcard tag : " << tag;
    }
    for(const QString &tag : tags_and)
    {
        qDebug() << "Default (AND) tag : " << tag;
    }

    // Get list of tags to exclude first
    // as they will be used for other queries
    if(tags_exclude.size() > 0)
    {
        for(int i = 0; i < tags_exclude.count(); i++)
        {
            QString query_exclude_single_tag(GET_ITEMS_FOR_TAGS_SQL_EXCLUDE_TAG_SEARCH);
            QString tag_exclude = "\'" + tags_exclude[i] + "\'";
            query_exclude_single_tag.replace(":tag_exclude", tag_exclude, Qt::CaseSensitive);
            query_exclude += query_exclude_single_tag;
        }

        qDebug() << "NOT tag query : " << query_exclude;
    }

    // Run queries separately for each type of queries,
    // with the exclude tags attached
    if(tags_or.size() > 0)
    {
        QString query_or(GET_ITEMS_FOR_TAGS_SQL_OR_SEARCH);
        QString tags_array("(");
        for(int i = 0; i < tags_or.count(); i++)
        {
            tags_array += "\'" + tags_or[i] + "\'";
            if(i != tags_or.count()-1)
            {
                tags_array += ", ";
            }
        }

        tags_array += ")";
        query_or.replace(":list_or", tags_array, Qt::CaseSensitive);
        // Add exclude tags
        query_or += query_exclude;

        qDebug() << "OR query : " << query_or;

        if(!getItemsFromCustomQuery(query_or, items))
        {
            DisplayWarningMessage("Cannot get items for query OR "+query_or);
        }
    }

    if(tags_wildcard.size() > 0)
    {
        QString query_wildcard(GET_ITEMS_FOR_TAGS_SQL_TEMPLATE);
        QString tags_combo("");
        for(int i = 0; i < tags_wildcard.count(); i++)
        {
            QString wildcard_suffix(GET_ITEMS_FOR_TAGS_SQL_WILDCARD_SEARCH_SUFFIX);
            wildcard_suffix.replace(":wildcard", "\'" + tags_wildcard[i] + "\'");
            tags_combo += wildcard_suffix;
            if(i != tags_wildcard.count()-1)
            {
                tags_combo += "OR t.tag ";
            }
        }

        query_wildcard += tags_combo;
        // Add exclude tags
        query_wildcard += query_exclude;
        qDebug() << "WILDCARD tag list : " << query_wildcard;

        if(!getItemsFromCustomQuery(query_wildcard, items))
        {
            DisplayWarningMessage("Cannot get items for query WILDCARD "+query_wildcard);
        }
    }

    // AND query is an OR query with grouping
    if(tags_and.size() > 0)
    {
        QString query_and(GET_ITEMS_FOR_TAGS_SQL_AND_SEARCH);
        QString tags_array("(");
        for(int i = 0; i < tags_and.count(); i++)
        {
            tags_array += "\'" + tags_and[i] + "\'";
            if(i != tags_and.count()-1)
            {
                tags_array += ", ";
            }
        }

        tags_array += ")";
        query_and.replace(":list_or", tags_array, Qt::CaseSensitive);
        query_and.replace(":num_tag", QString::number(tags_and.count()), Qt::CaseSensitive);

        // Add exclude tags
        query_and += query_exclude;
        qDebug() << "AND tag list : " << query_and;

        if(!getItemsFromCustomQuery(query_and, items))
        {
            DisplayWarningMessage("Cannot get items for query AND "+query_and);
        }
    }
}

void BooruMenu::searchImage(QString tags)
{
    QStringList tag_list = tags.split(" ", Qt::SkipEmptyParts);
    QVector<int> sql_id_list;
    QVector<BooruTypeItem> items;

    searchQueryParser(tag_list, items);

    for(int i = 0; i < items.count(); ++i)
    {
        qDebug() << "--> got item "+items[i].path+" ID item "+items[i].sql_id.toString();
        sql_id_list.push_back(items[i].sql_id.toInt());
    }
    proxyModel->setSqlIDFilter(sql_id_list, true);

    // Force auto-select first item in list if it exists,
    // and show the tags for this item
    QModelIndex start = proxyModel->index(0,0);

    if(start.isValid() && start.data().type() == (QVariant::Type::String))
    {
        this->ui->listViewFiles->setCurrentIndex(start);
        this->ui->listViewFiles->selectionModel()->select(start, QItemSelectionModel::Select);
        BooruMenu::viewClickedItemTag(start);
    }
    else
    {
        // Reset tag list if no hits
        BooruMenu::ClearItemTag();
    }
}

void BooruMenu::searchImageFinished(bool res)
{
    Q_UNUSED(res);
    proxyModel->setSqlIDFilter(QVector<int>(), false);
    searchInProgress = false;
}

void BooruMenu::resetSearchImage(void)
{
    proxyModel->setSqlIDFilter(QVector<int>(), false);
    searchInProgress = false;
}

void BooruMenu::exportToBooruFile(void)
{
    QString file_path = QFileDialog::getSaveFileName(this, "Export DB", QDir::homePath(), "SQLite Database (*.sqlite)");
    if(!file_path.isEmpty() && !file_path.isNull())
    {
        QFileInfo file_info(file_path);
        // Just force the .sqlite extension along with the abs path
#ifdef Q_OS_LINUX
        file_path = file_info.absolutePath() + "/" + file_info.baseName() + ".sqlite";
#else
        file_path = file_info.absolutePath() + "\\" + file_info.baseName() + ".sqlite";
#endif
        // Disable overwriting file
        if(QFile::exists(file_path))
        {
            DisplayWarningMessage(file_path+" already exists, choose another file");
        }
        else
        {
            QSqlQuery q;
            q.prepare(DUMP_TO_FILE);
            q.addBindValue(QVariant(file_path));
            if(q.exec())
            {
                DisplayInfoMessage("Exported db to "+file_path);
            }
            else
            {
                DisplayWarningMessage("Failed to export db to "+file_path+" "+db.lastError().text());
            }
        qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str();
            return ;
        }
    }
    else
    {
        DisplayInfoMessage("Export cancelled by user");
    }
}

void BooruMenu::importBooruFromFile(void)
{
    QVector<BooruTypeItem> items;
    if(!dumpItemsQuery(items))
    {
        DisplayWarningMessage("Cannot dump items table");
        return ;
    }
    else
    {
        for(int i = 0; i < items.count(); ++i)
        {
            // If file doesnt exist, ask user for another path
            if(!QFile::exists(items[i].path))
            {
                DisplayWarningMessage(items[i].path+" does not exist, point to another file");
                QString file_dialog_title("Get new path for ");
                file_dialog_title += items[i].path;

                QString file = QFileDialog::getOpenFileName(this, tr(file_dialog_title.toStdString().c_str()), QDir().absolutePath());
                if(file.isEmpty())
                {
                    DisplayInfoMessage(items[i].path+" not fixed");
                }
                else
                {
                    items[i].path = file;
                }
            }

            LoadFile(items[i].path, items[i].sql_id.toInt());
            qDebug() << "Import item "+items[i].path+" ID item "+items[i].sql_id.toString();
        }
    }
}

BooruMenu::~BooruMenu()
{
    delete ui;
    removeDb();
    model.clear();
}

void BooruMenu::BrowseFiles(QDir dir)
{
    QStringList files = dir.entryList(QDir::Files);
    for(const QString &file : files)
    {
        QString path = dir.filePath(file);
        if(path.isEmpty())
        {
            continue;
        }

        QFileInfo fileinfo(path);
        LoadFile(path, -1);
    }
}

bool BooruMenu::LoadFile(QFileInfo info, int item_id)
{
    QStandardItem* item;
    if(!info.isFile() || !info.exists() || !info.isReadable())
    {
        return false;
    }

    item = new QStandardItem(info.completeBaseName());
    BooruTypeItem item_data;

    if(info.suffix() == "gif" ||
        info.suffix() == "png" ||
        info.suffix() == "jpg" ||
        info.suffix() == "jpeg" ||
        info.suffix() == "webp" ||
        info.suffix() == "webm" ||
        info.suffix() == "mp4")
    {
        qDebug() << "Load " << info.completeBaseName() << Qt::endl;
        QImage image(info.absoluteFilePath());
        item->setText(info.completeBaseName());
        item_data.extension = info.suffix();
        item_data.path = info.absoluteFilePath();
        if(info.suffix() == "gif")
        {
            item->setIcon(QIcon(QPixmap::fromImage(image)));
            item_data.type = GIF;
        }
        else if(info.suffix() == "webm" || info.suffix() == "mp4")
        {
            QEventLoop loop;
            QImage thumbnail = QImage();
            cv::VideoCapture capture(item_data.path.toStdString());
            cv::Mat first_frame;

            capture >> first_frame;
            thumbnail = QImage((uchar*) first_frame.data, first_frame.cols, first_frame.rows, first_frame.step, QImage::Format_RGB888);
            item->setIcon(QIcon(QPixmap::fromImage(thumbnail)));
            item_data.type = MOVIE;
        }
        else
        {
            item->setIcon(QIcon(QPixmap::fromImage(image)));
            item_data.type = STILL_IMG;
        }
        if(item_id < 0)
        {
            item_data.sql_id = addItemQuery(item_data.type, item_data.path);
        }
        else
        {
            item_data.sql_id = item_id;
        }
        item->setData(QVariant::fromValue(item_data), Qt::UserRole);
    }
    else
    {
        item_data.extension = info.suffix();
        item_data.path = info.absoluteFilePath();
        item->setData(QVariant::fromValue(item_data), Qt::UserRole);
    }

    model.appendRow(item);
    return true;
}

void BooruMenu::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if(key_pressed != Qt::Key_Clear)
    {
        if(key_pressed == Qt::Key_Up ||
           key_pressed == Qt::Key_Down ||
           key_pressed == Qt::Key_PageUp ||
           key_pressed == Qt::Key_PageDown)
        {
            BooruMenu::viewClickedItemTag(current);
        }
        key_pressed = Qt::Key_Clear;
    }
}

bool BooruMenu::eventFilter(QObject *obj, QEvent *event)
{
    // User clicking item
    if(event->type() == QEvent::MouseButtonPress && obj == ui->listViewFiles->viewport())
    {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        if(e->button() == Qt::RightButton) {
            // Manually set item as selected
            QModelIndex idx = ui->listViewFiles->indexAt(e->pos());

            // Force reset selection
            // Also works with clearSelection() but leaves a light blue marker around previously selected
            ui->listViewFiles->selectionModel()->clear();
            ui->listViewFiles->selectionModel()->select(idx, QItemSelectionModel::Select);

            QVariant item_var = ui->listViewFiles->indexAt(e->pos()).data(Qt::UserRole);
            BooruTypeItem item_data = item_var.value<BooruTypeItem>();
            ItemContextMenu menu(this, e->globalPos(), &item_data);
        }
    }
    // See currentChanged
    // Up/Down keys -> show tags (wait for current index to be updated)
    // Enter key -> same as double click
    else if(event->type() == QEvent::KeyPress && obj == ui->listViewFiles)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Up ||
           keyEvent->key() == Qt::Key_Down ||
           keyEvent->key() == Qt::Key_PageUp ||
           keyEvent->key() == Qt::Key_PageDown)
        {
            key_pressed = keyEvent->key();
        }
        else if(keyEvent->key() == Qt::Key_Return)
        {
            QModelIndex cur_idx = this->ui->listViewFiles->currentIndex();
            BooruMenu::viewDoubleClickedItem(cur_idx);
        }
    }

    // Standard event processing
    return false;
}

void BooruMenu::ClearItemTag(void)
{
    tagModel.setStringList(QStringList());
}

void BooruMenu::SyncItemTag(const QVariant &id_item)
{
    QStringList tags_list;
    getTagsFromItemQuery(id_item, tags_list);
    tagModel.setStringList(tags_list);
}

void BooruMenu::viewClickedItemTag(const QModelIndex& idx)
{
    QVariant item_var = idx.data(Qt::UserRole);
    BooruTypeItem item_data = item_var.value<BooruTypeItem>();
    // Update list of tags in model
    SyncItemTag(item_data.sql_id);
}

void BooruMenu::viewDoubleClickedItem(const QModelIndex& idx)
{
    QVariant item_var = idx.data(Qt::UserRole);
    BooruTypeItem item_data = item_var.value<BooruTypeItem>();

    editor = new ItemEditor(nullptr, &item_data);
    connect(editor, &QDialog::finished, this, &BooruMenu::getUpdatedTagList);

    // Block on dialog window
    editor->setModal(true);
    editor->exec();

    qDebug() << QObject::trUtf8("Item %1 has been double clicked.").arg(idx.data().toString());
}

void BooruMenu::getUpdatedTagList(int state)
{
    // Delete when leaving dialog window event loop
    editor->deleteLater();

    if(state == QDialog::Accepted)
    {
        QModelIndex idx_proxy = this->ui->listViewFiles->currentIndex();
        QModelIndex idx = proxyModel->mapToSource(idx_proxy);
        QVariant item_var = idx.data(Qt::UserRole);
        BooruTypeItem item_data = item_var.value<BooruTypeItem>();

        // Update list of tags in model
        SyncItemTag(item_data.sql_id);
    }
}
