#include "BooruMenu.h"
#include "ui_BooruMenu.h"
#include <QImageReader>

BooruMenu::BooruMenu(QWidget *parent, QDir _filesDir) :
    QMainWindow(parent),
    ui(new Ui::BooruMenu),
    filesDir(_filesDir)
{
    ui->setupUi(this);

    QStringList files = filesDir.entryList(QDir::Files);
    for(const QString &file : files)
    {
        QString path = filesDir.filePath(file);
        if(path.isEmpty())
        {
            continue;
        }

        QFileInfo fileinfo(path);
        LoadFile(path);
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

void BooruMenu::searchImage(QString tags)
{
    QStringList tag_list = tags.split(" ");
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
        BooruTypeItem item_data = {
            QPixmap::fromImage(image),
            QStringList()
        };
        item->setText(info.completeBaseName());
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
            QMenu menu(this);
            QAction *copy = menu.addAction("Copy picture to clipboard");

            // Manually set item as selected
            QModelIndex idx = ui->listViewFiles->indexAt(e->pos());

            // Also works with clearSelection() but leaves a light blue marker around previously selected
            ui->listViewFiles->selectionModel()->clear();
            ui->listViewFiles->selectionModel()->select(idx, QItemSelectionModel::Select);

            QAction *selected = menu.exec(e->globalPos());

            if(selected == copy) {
                QClipboard *clipboard = QGuiApplication::clipboard();
                QVariant item_var = ui->listViewFiles->indexAt(e->pos()).data(Qt::UserRole);
                BooruTypeItem item_data = item_var.value<BooruTypeItem>();
                clipboard->setPixmap(item_data.picture);
            }
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
    QPixmap item_pixmap = item_data.picture;
    auto tag_model = item_data.tags;

    editor = new ItemEditor(nullptr, item_pixmap, tag_model);
    connect(editor, &QDialog::finished, this, &BooruMenu::getUpdatedTagList);

    // Block on dialog window
    editor->setModal(true);
    editor->show();

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
