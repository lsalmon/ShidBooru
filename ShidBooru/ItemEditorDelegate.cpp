#include "ItemEditorDelegate.h"

ItemEditorDelegate::ItemEditorDelegate(QObject *parent)
{

}

QWidget *ItemEditorDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    ItemEditor *editor = new ItemEditor(parent);
    editor->show();

    return editor;
}
