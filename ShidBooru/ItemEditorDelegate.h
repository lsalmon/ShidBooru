#ifndef ITEMEDITORDELEGATE_H
#define ITEMEDITORDELEGATE_H

#include "ItemEditor.h"
#include <QStyledItemDelegate>

class ItemEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ItemEditorDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    //void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    //void setModelData(QWidget *editor, QAbstractItemModel *model,
    //                  const QModelIndex &index) const override;

    //void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
    //                          const QModelIndex &index) const override;
};

#endif // ITEMEDITORDELEGATE_H
