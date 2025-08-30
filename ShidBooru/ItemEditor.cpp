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
    ui->horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);
    ui->picture->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    if(item->type == GIF)
    {
        QBuffer *buf = new QBuffer(&item->gif);
        buf->open(QIODevice::ReadOnly);
        QMovie *gif_movie = new QMovie(buf, QByteArray(), this);
        ui->picture->setMovie(gif_movie);
        gif_movie->start();
    }
    else
    {
        ui->picture->setPixmap(item->picture);
    }

    this->adjustSize();

    ui->tagListView->setModel(&default_tag_model);

    // Set tags to a model inside the dialog box and update the QStringList of the model
    QStringList tags_list;
qDebug() << "AAAAAAAAAAAAAAAAA";
    getTagsFromItemQuery(item->sql_id.toInt(), tags_list);
    this->default_tag_model.setStringList(tags_list);
qDebug() << "BBBBBBBBBBBBBBBBB";
    for(const QString &tag : tags_list)
    {
        qDebug() << "=====> tags for item "+item->sql_id.toString()+" : "+tag;
    }
qDebug() << "CCCCCCCCCCCCCCCCCC";

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
                    tag_list.append(tag);
                    this->default_tag_model.setStringList(tag_list);
                }
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
        tag_list.removeAt(tag_index.row());
        this->default_tag_model.setStringList(tag_list);
        qDebug() <<"Remove tag "+tag;
        removeTagQuery(tag);
    }
}

QStringList ItemEditor::GetUpdatedTags()
{
    return this->default_tag_model.stringList();
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
    delete ui;
}
