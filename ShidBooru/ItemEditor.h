#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include <QDialog>
#include <QListWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QDebug>
#include "BooruItemType.h"

namespace Ui {
class ItemEditor;
}

class ItemEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ItemEditor(QWidget *parent = nullptr,
                        QPixmap _item_pixmap = QPixmap(),
                        QSharedPointer<QStringListModel> _tags_model = nullptr);
    ~ItemEditor();

private slots:
    void AddTag();
    void RemoveSelectedTag();

private:
    QPixmap item_pixmap;
    QSharedPointer<QStringListModel> tags_model;
    QStringList saved_tag_list;
    Ui::ItemEditor *ui;
};

#endif // ITEMEDITOR_H
