#include "BooruMenu.h"
#include "ui_boorumenu.h"
#include <QImageReader>

BooruMenu::BooruMenu(QWidget *parent, QDir _filesDir) :
    QFrame(parent),
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
    ui->listViewFiles->setModel(&model);
    ui->listViewFiles->setViewMode(QListView::ListMode);
    ui->listViewFiles->setIconSize(QSize(128,128));
    ui->listViewFiles->setResizeMode(QListView::Adjust);
    ui->listViewTags->setModel(&tagModel);

    connect(ui->listViewFiles, &QListView::clicked, this, &BooruMenu::viewClickedItemTag);
    connect(ui->listViewFiles, &QListView::doubleClicked, this, &BooruMenu::viewDoubleClickedItem);
    ui->listViewFiles->viewport()->installEventFilter(this);
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
    editor->open();

    qDebug() << QObject::trUtf8("Item %1 has been double clicked.").arg(idx.data().toString());
}

void BooruMenu::getUpdatedTagList(int state)
{
    // Delete when leaving dialog window event loop
    editor->deleteLater();

    if(state == QDialog::Accepted)
    {
        QModelIndex idx = this->ui->listViewFiles->currentIndex();
        QStringList tags = editor->GetUpdatedTags();

        QVariant item_var = idx.data(Qt::UserRole);
        BooruTypeItem item_data = item_var.value<BooruTypeItem>();

        // Update list of tags for item on main menu right after edition
        tagModel.setStringList(tags);

        // Update list of tags inside item
        item_data.tags = tags;
        QStandardItem* item = model.itemFromIndex(idx);
        item->setData(QVariant::fromValue(item_data), Qt::UserRole);
    }
}
