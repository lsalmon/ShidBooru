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

BooruMenu::BooruMenu(QWidget *parent, QString _file) :
    QMainWindow(parent),
    ui(new Ui::BooruMenu),
    file(_file)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    //db.setDatabaseName("BooruInstance");
    // Store table in RAM
    db.setDatabaseName(":memory:");

    if(!db.open())
    {
        DisplayWarningMessage("Creating db failed with "+db.lastError().text());
        return ;
    }

    // Support for foreign keys
    QSqlQuery setup;
    setup.exec("PRAGMA foreign_keys = ON;");

    QStringList tables = db.tables();
    if (tables.contains("items", Qt::CaseInsensitive)
        && tables.contains("tags", Qt::CaseInsensitive)
        && tables.contains("links", Qt::CaseInsensitive))
    {
        DisplayWarningMessage("db is already full, abort");
        removeDb();
        return ;
    }

    QSqlQuery q;
    if (!q.exec(ITEM_SQL))
    {
        DisplayWarningMessage("Creating Item table failed with "+q.lastError().text());
        removeDb();
        return ;
    }
    if (!q.exec(TAG_SQL))
    {
        DisplayWarningMessage("Creating Tag table failed with "+q.lastError().text());
        removeDb();
        return ;
    }
    if (!q.exec(LINK_SQL))
    {
        DisplayWarningMessage("Creating Item<->Tag table failed with "+q.lastError().text());
        removeDb();
        return ;
    }

    // Load items from user directory or from single file
    QFileInfo fileinfo(file);
    if(fileinfo.isDir())
    {
        BrowseFiles(QDir(file));
    }
    else
    {
        LoadFile(fileinfo);
    }

    proxyModel = new TagFilterProxyModel(this);
    //proxyModel->setFilterRole(Qt::UserRole);
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
    }

    connect(ui->listViewFiles, &QListView::clicked, this, &BooruMenu::viewClickedItemTag);
    connect(ui->listViewFiles, &QListView::doubleClicked, this, &BooruMenu::viewDoubleClickedItem);
    ui->listViewFiles->viewport()->installEventFilter(this);
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
            LoadFile(fileinfo);
        }
    }
}

void BooruMenu::removeImage(void)
{
    QModelIndex idx_proxy = this->ui->listViewFiles->currentIndex();
    QModelIndex idx = proxyModel->mapToSource(idx_proxy);
    QString item_desc = idx.data().toString();
    QMessageBox warning_item_missing;
    warning_item_missing.setIcon(QMessageBox::Warning);

    if(!idx.isValid())
    {
        warning_item_missing.setText("Could not find the item "+item_desc);
        warning_item_missing.exec();
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
            warning_item_missing.setText("Could not delete the item "+item_desc);
            warning_item_missing.exec();
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
        LoadFile(path);
    }
}

bool BooruMenu::LoadFile(QFileInfo info)
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
        item_data.sql_id = addItemQuery(item_data.type, item_data.path);
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
