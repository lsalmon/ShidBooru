#include "BooruMenu.h"
#include "ui_boorumenu.h"

BooruMenu::BooruMenu(QWidget *parent, QDir _filesDir, ItemEditorDelegate *_delegate) :
    QFrame(parent),
    ui(new Ui::BooruMenu),
    filesDir(_filesDir),
    delegate(_delegate)
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
        qDebug() << "Load " << path << Qt::endl;
        QFileInfo fileinfo(path);
        LoadFile(path);
    }
    ui->listViewFiles->setModel(&model);
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
        QIcon icon(info.absoluteFilePath());
        item = new QStandardItem(icon, info.completeBaseName());
    }
    else
    {
        item = new QStandardItem(info.completeBaseName());
    }
    model.appendRow(item);

    return true;
}

void BooruMenu::viewDoubleClickedItem(const QModelIndex& idx)
{
    qDebug() << QObject::trUtf8("Item %1 has been double clicked.").arg(idx.data().toString());
}
