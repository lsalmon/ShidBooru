#include "SearchTagDialog.h"
#include "ui_SearchTagDialog.h"

SearchTagDialog::SearchTagDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchTagDialog)
{
    this->setWindowFlag(Qt::Tool);
    ui->setupUi(this);
    connect(ui->searchButton, &QPushButton::clicked, this, &SearchTagDialog::onSearchButtonClicked);
}

SearchTagDialog::~SearchTagDialog()
{
    delete ui;
}

void SearchTagDialog::onSearchButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    QString tags = ui->tagLineInput->text();

    emit searchRequest(tags);
}
