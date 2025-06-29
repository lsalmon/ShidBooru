#ifndef SEARCHTAGDIALOG_H
#define SEARCHTAGDIALOG_H

#include <QDialog>

namespace Ui {
class SearchTagDialog;
}

class SearchTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchTagDialog(QWidget *parent = nullptr);
    ~SearchTagDialog();

signals:
    void searchRequest(QString tag);

private slots:
    void onSearchButtonClicked(bool checked);

private:
    Ui::SearchTagDialog *ui;
};

#endif // SEARCHTAGDIALOG_H
