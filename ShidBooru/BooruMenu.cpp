#include "BooruMenu.h"
#include "ui_BooruMenu.h"
#include <QImageReader>

BooruMenu::BooruMenu(QWidget *parent, QString _file) :
    QMainWindow(parent),
    ui(new Ui::BooruMenu),
    file(_file)
{
    ui->setupUi(this);

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

    if(confirm == QMessageBox::Yes && !proxyModel->removeRow(idx.row()))
    {
        warning_item_missing.setText("Could not delete the item "+item_desc);
        warning_item_missing.exec();
    }
}

void BooruMenu::searchImage(QString tags)
{
    QStringList tag_list = tags.split(" ", Qt::SkipEmptyParts);
    proxyModel->setSearchTag(tag_list, true);

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
    proxyModel->setSearchTag(QStringList(""), false);
    searchInProgress = false;
}

void BooruMenu::resetSearchImage(void)
{
    proxyModel->setSearchTag(QStringList(""), false);
    searchInProgress = false;
}

BooruMenu::~BooruMenu()
{
    delete ui;
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

    if(info.completeSuffix() == "gif" || info.completeSuffix() == "png" || info.completeSuffix() == "jpg" || info.completeSuffix() == "jpeg")
    {
        qDebug() << "Load " << info.completeBaseName() << Qt::endl;
        QImage image(info.absoluteFilePath());
        item = new QStandardItem(info.completeBaseName());
        item->setText(info.completeBaseName());
        BooruTypeItem item_data = {
            QPixmap::fromImage(image),
            QStringList()
        };
        item_data.extension = info.completeSuffix();
        if(info.completeSuffix() == "gif")
        {
            item_data.type = GIF;
            QFile file(info.absoluteFilePath());
            if(file.open(QIODevice::ReadOnly))
            {
                item_data.gif = file.readAll();
            }
        }
        item->setIcon(QIcon(QPixmap::fromImage(image)));
        item->setData(QVariant::fromValue(item_data), Qt::UserRole);
    }
    else
    {
        item = new QStandardItem(info.completeBaseName());
        BooruTypeItem item_data = {
            QPixmap(),
            QStringList()
        };
        item_data.extension = info.completeSuffix();
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

void BooruMenu::viewClickedItemTag(const QModelIndex& idx)
{
    QVariant item_var = idx.data(Qt::UserRole);
    BooruTypeItem item_data = item_var.value<BooruTypeItem>();
    auto tags = item_data.tags;
    tagModel.setStringList(tags);
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
        QStringList tags = editor->GetUpdatedTags();
        QVariant item_var = idx.data(Qt::UserRole);
        BooruTypeItem item_data = item_var.value<BooruTypeItem>();

        // Update list of tags inside item
        item_data.tags = tags;
        QStandardItem* item = model.itemFromIndex(idx);
        if(item == nullptr) {
            QMessageBox warning_item_missing;
            warning_item_missing.setIcon(QMessageBox::Warning);
            warning_item_missing.setText("Could not find the item");
            warning_item_missing.exec();
        } else {
            // Update list of tags for item on main menu right after edition
            tagModel.setStringList(tags);

            item->setData(QVariant::fromValue(item_data), Qt::UserRole);
        }
    }
}
