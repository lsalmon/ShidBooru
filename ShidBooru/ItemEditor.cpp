#include "ItemEditor.h"
#include "ui_ItemEditor.h"
#include <QDebug>

ItemEditor::ItemEditor(QWidget *parent,
                       QPixmap _item_pixmap,
                       QSharedPointer<QStringListModel> _tags_model) :
    QDialog(parent),
    item_pixmap(_item_pixmap),
    tags_model(_tags_model),
    ui(new Ui::ItemEditor)
{
    ui->setupUi(this);
    ui->picture->setPixmap(item_pixmap);
    if(tags_model) {
        saved_tag_list = tags_model.get()->stringList();
        ui->tagListView->setModel(tags_model.get());
    }

    connect(ui->addButton, &QPushButton::clicked, this, &ItemEditor::AddTag);
    connect(ui->removeButton, &QPushButton::clicked, this, &ItemEditor::RemoveSelectedTag);
}

void ItemEditor::AddTag()
{
    QString tag = this->ui->tagLineEdit->text().trimmed();
qDebug() << "tag :" << tag;
    if(!tag.isEmpty() && this->tags_model.get()) {
        QStringList tag_list = this->tags_model.get()->stringList();
        tag_list.append(tag);
        this->tags_model.get()->setStringList(tag_list);
        this->ui->tagLineEdit->clear();
    }
}

void ItemEditor::RemoveSelectedTag()
{
    QModelIndex tag_index = this->ui->tagListView->currentIndex();
    if(tag_index.isValid()) {
        QStringList tag_list = tags_model.get()->stringList();
        tag_list.removeAt(tag_index.row());
        tags_model.get()->setStringList(tag_list);
    }
}

ItemEditor::~ItemEditor()
{
    delete ui;
}
