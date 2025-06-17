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

    connect(ui->listViewFiles, &QListView::doubleClicked, this, &BooruMenu::viewDoubleClickedItem);
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
            QSharedPointer<QStringListModel>(new QStringListModel())
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
            QSharedPointer<QStringListModel>(new QStringListModel())
        };
        item->setData(QVariant::fromValue(item_data), Qt::UserRole);
    }
    model.appendRow(item);

    return true;
}

void BooruMenu::viewDoubleClickedItem(const QModelIndex& idx)
{
    QVariant item_var = idx.data(Qt::UserRole);
    BooruTypeItem item_data = item_var.value<BooruTypeItem>();
    QPixmap item_pixmap = item_data.picture;
    auto tag_model = item_data.tags;

    ItemEditor* editor = new ItemEditor(nullptr, item_pixmap, tag_model);
    editor->show();
    qDebug() << QObject::trUtf8("Item %1 has been double clicked.").arg(idx.data().toString());
}
