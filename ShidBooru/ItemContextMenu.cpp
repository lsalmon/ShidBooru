#include "ItemContextMenu.h"
#include <QDebug>

ItemContextMenu::ItemContextMenu(QWidget* parent, QPoint pos, BooruTypeItem *item_data)
{
    QMenu menu(parent);
    QAction *copy = menu.addAction("Copy picture to clipboard");
    QAction *save = menu.addAction("Save content of picture to file");

    QAction *selected = menu.exec(pos);

    if(selected == copy)
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        if(item_data->type == GIF)
        {
            QByteArray gif;
            QFile file(item_data->path);
            if(file.open(QIODevice::ReadOnly))
            {
                gif = file.readAll();
                QMimeData *mimeData = new QMimeData();
                mimeData->setData("image/gif", gif);
                clipboard->setMimeData(mimeData);
            }
        }
        else
        {
            QImage image(item_data->path);
            QPixmap picture = QPixmap::fromImage(image);
            clipboard->setPixmap(picture);
        }
    }
    else if(selected == save)
    {
        QString file_path;

        if(item_data->type == GIF)
        {
            file_path = QFileDialog::getSaveFileName(this, "Save gif", QDir::homePath(), "GIF Images (*.gif)");
        }
        else
        {
            file_path = QFileDialog::getSaveFileName(this, "Save picture", QDir::homePath(), "Images (*.png *.jpg *.bmp)");
        }
        if(!file_path.isEmpty() && !file_path.isNull())
        {
            if(item_data->type == GIF)
            {
                QFile save_file(file_path);
                if(save_file.open(QIODevice::WriteOnly))
                {
                    QByteArray gif;
                    QFile file(item_data->path);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        gif = file.readAll();
                        save_file.write(gif);
                    }
                    else
                    {
                        DisplayWarningMessage("Failed to save "+file_path+'.'+item_data->extension);
                    }
                    save_file.close();
                }
            }
            else
            {
                QImage image(item_data->path);
                QPixmap picture = QPixmap::fromImage(image);
                if(!picture.save(file_path+'.'+item_data->extension, item_data->extension.toUpper().toUtf8().constData()))
                {
                    DisplayWarningMessage("Failed to save "+file_path+'.'+item_data->extension);
                }
            }
        }
        else
        {
            DisplayInfoMessage("Save cancelled by user");
        }
    }
}
