#include "ItemEditor.h"
#include "ui_ItemEditor.h"

ItemEditor::ItemEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ItemEditor)
{
    ui->setupUi(this);
}

ItemEditor::~ItemEditor()
{
    delete ui;
}
