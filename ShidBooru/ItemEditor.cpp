#include "ItemEditor.h"
#include "ui_ItemEditor.h"
#include <QDebug>

ItemEditor::ItemEditor(QWidget *parent,
                       QPixmap _item_pixmap,
                       QStringList _tags) :
    QDialog(parent),
    item_pixmap(_item_pixmap),
    tags(_tags),
    ui(new Ui::ItemEditor)
{
    ui->setupUi(this);
    ui->horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);
    ui->picture->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui->picture->setPixmap(item_pixmap);

    this->adjustSize();

    ui->tagListView->setModel(&default_tag_model);
    // Set tags to a model inside the dialog box and update the QStringList of the model
    this->default_tag_model.setStringList(tags);

    connect(ui->addButton, &QPushButton::clicked, this, &ItemEditor::AddTag);
    connect(ui->removeButton, &QPushButton::clicked, this, &ItemEditor::RemoveSelectedTag);
}

void ItemEditor::AddTag()
{
    QString tag = this->ui->tagLineEdit->text().trimmed();

    if(!tag.isEmpty() && this->ui->tagListView->model() != nullptr) {
        QStringList tag_list = this->default_tag_model.stringList();
        tag_list.append(tag);
        this->default_tag_model.setStringList(tag_list);
        this->ui->tagLineEdit->clear();
    }
}

void ItemEditor::RemoveSelectedTag()
{
    QModelIndex tag_index = this->ui->tagListView->currentIndex();
    if(tag_index.isValid()) {
        QStringList tag_list = this->default_tag_model.stringList();
        tag_list.removeAt(tag_index.row());
        this->default_tag_model.setStringList(tag_list);
    }
}

QStringList ItemEditor::GetUpdatedTags()
{
    return this->default_tag_model.stringList();
}

ItemEditor::~ItemEditor()
{
    delete ui;
}
