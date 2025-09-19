#include "ItemEditor.h"
#include "ui_ItemEditor.h"
#include "QSqlQueryHelper.h"
#include <QDebug>

static QSqlDatabase db;

ItemEditor::ItemEditor(QWidget *parent,
                       BooruTypeItem *_item) :
    QDialog(parent),
    item(_item),
    ui(new Ui::ItemEditor)
{
    db = QSqlDatabase::database();
    ui->setupUi(this);

    if(item->type == GIF)
    {
        QFile file(item->path);
        if(file.open(QIODevice::ReadOnly))
        {
            gif = file.readAll();
            buf = new QBuffer(&gif);
            buf->open(QIODevice::ReadOnly);
            gif_movie = new QMovie(buf, QByteArray(), this);
            ui->picture->setMovie(gif_movie);
            gif_movie->start();
        }
    }
    else
    {
        QImage image(item->path);
        QPixmap picture = QPixmap::fromImage(image);
        ui->picture->setPixmap(picture);
    }

    // Fixed size, use hint size and cap it to 70% of the screen size
    QScreen *main_screen = this->screen();
    QSize hint_size = this->sizeHint();
    QRect avail = main_screen->availableGeometry();
    QSize avail_size = avail.size();

    ui->picture->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->picture->setScaledContents(true);

    if(hint_size.height() > 0.7*avail_size.height()
        || hint_size.width() > 0.7*avail_size.width())
    {
        hint_size.setHeight(0.7*avail_size.height());
        hint_size.setWidth(0.7*avail_size.width());
    }
    ui->picture->setFixedSize(hint_size);

    // Set tags to a model inside the dialog box and update the QStringList of the model
    ui->tagListView->setModel(&default_tag_model);

    QStringList tags_list;
    getTagsFromItemQuery(item->sql_id, tags_list);
    this->default_tag_model.setStringList(tags_list);

    connect(ui->addButton, &QPushButton::clicked, this, &ItemEditor::AddTag);
    connect(ui->removeButton, &QPushButton::clicked, this, &ItemEditor::RemoveSelectedTag);
}

void ItemEditor::AddTag()
{
    QString tag = this->ui->tagLineEdit->text().trimmed();
    // Check if LineEdit not empty and model exists
    if(!tag.isEmpty() && this->ui->tagListView->model() != nullptr)
    {
        QStringList tag_input_list = tag.split(" ", Qt::SkipEmptyParts);
        for(const QString &tag  : tag_input_list)
        {
            // Remove empty strings
            if(!tag.isEmpty())
            {
                // Check if tag not already in list
                QStringList tag_list = this->default_tag_model.stringList();
                if(tag_list.indexOf(tag) != -1) {
                    QMessageBox::warning(this, tr("ShidBooru"), tr("Tag already exists"));
                } else {
                    int id_tag = checkDuplicateTagQuery(tag);
                    if(id_tag < 0) {
                        id_tag = addTagQuery(tag).toInt();
                        qDebug() << "New tag id " << id_tag;
                    } else {
                        id_tag = getIDFromTagQuery(tag);
                        qDebug() << "Got existing tag id " << id_tag;
                    }
                    int id_link = checkDuplicateLinkQuery(QVariant(id_tag), item->sql_id);
                    if(id_link < 0) {
                        qDebug() << "New link id (tag -> item) " << QString(id_tag) << " -> " << item->sql_id.toString();
                        addLinkQuery(item->sql_id, QVariant(id_tag));
                    }
                    // Update list locally
                    tag_list.append(tag);
                    this->default_tag_model.setStringList(tag_list);
                }
                this->ui->tagLineEdit->clear();
            }
        }
    }
}

void ItemEditor::RemoveSelectedTag()
{
    QModelIndex tag_index = this->ui->tagListView->currentIndex();
    if(tag_index.isValid())
    {
        QStringList tag_list = this->default_tag_model.stringList();
        QString tag = tag_list.at(tag_index.row());
        int tag_id = getIDFromTagQuery(tag);
        if(tag_id < 0)
        {
            qDebug() << "Cannot get tag id for tag "+tag;
            return ;
        }

        // Remove link
        if(!removeLinkQuery(QVariant(item->sql_id), QVariant(tag_id)))
        {
            qDebug() << "Cannot remove link to "+tag;
            return ;
        }

        // If link was the last to use the tag, also remove tag
        QVector<BooruTypeItem> items;
        getItemsFromSingleTagQuery(tag, items);

        if(items.empty()) {
            qDebug() << "Remove tag "+tag+" completely";
            removeTagQuery(tag);
        }

        // Update list locally
        tag_list.removeAt(tag_index.row());
        this->default_tag_model.setStringList(tag_list);
    }
}

void ItemEditor::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        ItemContextMenu menu(this, event->globalPos(), item);
    }
}

ItemEditor::~ItemEditor()
{
    ui->picture->clear();
    if(item->type == GIF)
    {
        delete(gif_movie);
        delete(buf);
    }
    delete ui;
}
