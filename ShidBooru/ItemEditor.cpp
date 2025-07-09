#include "ItemEditor.h"
#include "ui_ItemEditor.h"
#include <QDebug>

ItemEditor::ItemEditor(QWidget *parent,
                       BooruTypeItem *_item) :
    QDialog(parent),
    item(_item),
    ui(new Ui::ItemEditor)
{
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
    this->default_tag_model.setStringList(item->tags);

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
                };
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
        tag_list.removeAt(tag_index.row());
        this->default_tag_model.setStringList(tag_list);
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
