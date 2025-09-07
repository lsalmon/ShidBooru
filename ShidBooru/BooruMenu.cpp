#include "BooruMenu.h"
#include "ui_BooruMenu.h"
#include "QSqlQueryHelper.h"
#include <QImageReader>

static QSqlDatabase db;

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
    ui->listViewFiles->viewport()->installEventFilter(this);
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
    QModelIndex idx_proxy = this->ui->listViewFiles->currentIndex();
    QModelIndex idx = proxyModel->mapToSource(idx_proxy);
    QString item_desc = idx.data().toString();

    if(!idx.isValid())
    {
        DisplayWarningMessage("Could not find the item "+item_desc);
    }

    QMessageBox::StandardButton confirm = QMessageBox::question(this, "Confirm suppression", "Delete item "+item_desc+" ?", QMessageBox::Yes|QMessageBox::No);

    if(confirm == QMessageBox::Yes)
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
                    getItemsFromTagQuery(id_tag, items);

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
            DisplayWarningMessage("Could not delete the item "+item_desc);
            return ;
        }

        // Reset tag window
        tagModel.setStringList(QStringList());
    }
}

void BooruMenu::searchImage(QString tags)
{
    QStringList tag_list = tags.split(" ", Qt::SkipEmptyParts);

    QString search_tag = tag_list[0];
    QVector<int> sql_id_list;

    int tag_id = getIDFromTagQuery(search_tag);
    if(tag_id < 0)
    {
        qDebug() << "Cannot get tag id for tag "+search_tag;
        return ;
    }
    else
    {
        QVector<BooruTypeItem> items;
        if(!getItemsFromTagQuery(tag_id, items))
        {
            DisplayWarningMessage("Cannot get item for tag "+search_tag);
            return ;
        }
        else
        {
            for(int i = 0; i < items.count(); ++i)
            {
                qDebug() << "For search tag "+search_tag+" --> got item "+items[i].path+" ID item "+items[i].sql_id.toString();
                sql_id_list.push_back(items[i].sql_id.toInt());
            }
        }
    }
    proxyModel->setSqlIDFilter(sql_id_list, true);

    // Auto-select first item in list if it exists
    QModelIndex start = proxyModel->index(0,0);

    if(start.isValid() && start.data().type() == (QVariant::Type::String))
    {
        this->ui->listViewFiles->setCurrentIndex(start);
        BooruMenu::viewClickedItemTag(start);
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
        QFileInfo fileinfo(file_path);
        // Just force the .sqlite extension along with the abs path
#ifdef Q_OS_LINUX
        file_path = fileinfo.absolutePath() + "/" + fileinfo.baseName() + ".sqlite";
#else
        file_path = fileinfo.absolutePath() + "\\" + fileinfo.baseName() + ".sqlite";
#endif
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
    qDebug() << q.boundValue(0) << "    " << q.executedQuery().toStdString().c_str();
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

    if(info.completeSuffix() == "gif" || info.completeSuffix() == "png" || info.completeSuffix() == "jpg" || info.completeSuffix() == "jpeg")
    {
        qDebug() << "Load " << info.completeBaseName() << Qt::endl;
        QImage image(info.absoluteFilePath());
        item->setText(info.completeBaseName());
        item_data.extension = info.completeSuffix();
        item_data.path = info.absoluteFilePath();
        if(info.completeSuffix() == "gif")
        {
            item_data.type = GIF;
        }
        item->setIcon(QIcon(QPixmap::fromImage(image)));
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
        item_data.extension = info.completeSuffix();
        item_data.path = info.absoluteFilePath();
        item->setData(QVariant::fromValue(item_data), Qt::UserRole);
    }

    model.appendRow(item);
    return true;
}

bool BooruMenu::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress && obj == ui->listViewFiles->viewport()) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        if(e->button() == Qt::RightButton) {
            // Manually set item as selected
            QModelIndex idx = ui->listViewFiles->indexAt(e->pos());

            // Also works with clearSelection() but leaves a light blue marker around previously selected
            ui->listViewFiles->selectionModel()->clear();
            ui->listViewFiles->selectionModel()->select(idx, QItemSelectionModel::Select);

            QVariant item_var = ui->listViewFiles->indexAt(e->pos()).data(Qt::UserRole);
            BooruTypeItem item_data = item_var.value<BooruTypeItem>();
            ItemContextMenu menu(this, e->globalPos(), &item_data);
        }
    }

    return false;
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
