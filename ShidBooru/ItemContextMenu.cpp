#include "ItemContextMenu.h"
#include <QDebug>

ItemContextMenu::ItemContextMenu(QWidget* parent, QPoint pos, BooruTypeItem *item_data)
{
    QMenu menu(parent);
    QAction *copy;
    if(item_data->type == GIF)
    {
        copy = menu.addAction("Copy gif to clipboard");
    }
    else if(item_data->type == MOVIE)
    {
        copy = menu.addAction("Copy movie to clipboard");
    }
    else
    {
        copy = menu.addAction("Copy picture to clipboard");
    }
    QAction *save = menu.addAction("Save content to file");

    QAction *selected = menu.exec(pos);

    if(selected == copy)
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        if(item_data->type == GIF || item_data->type == MOVIE)
        {
            QByteArray raw_data;
            QFile file(item_data->path);
            if(file.open(QIODevice::ReadOnly))
            {
                raw_data = file.readAll();
                QMimeData *mimeData = new QMimeData();

                if(item_data->type == GIF)
                {
                    mimeData->setData("image/gif", raw_data);
                }
                else if(item_data->type == MOVIE)
                {
                    QString mime_type = "video/"+item_data->extension;
                    mimeData->setData(mime_type, raw_data);
                }
                clipboard->setMimeData(mimeData, QClipboard::Clipboard);
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
        else if(item_data->type == MOVIE)
        {
            file_path = QFileDialog::getSaveFileName(this, "Save movie", QDir::homePath(), "Movies (*.mp4 *.webm)");
        }
        else
        {
            file_path = QFileDialog::getSaveFileName(this, "Save picture", QDir::homePath(), "Images (*.png *.jpg *.jpeg *.webp)");
        }
        if(!file_path.isEmpty() && !file_path.isNull())
        {
            if(item_data->type == GIF || item_data->type == MOVIE)
            {
                QFile save_file(file_path);
                if(save_file.open(QIODevice::WriteOnly))
                {
                    QByteArray raw_data;
                    QFile file(item_data->path);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        raw_data = file.readAll();
                        save_file.write(raw_data);
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
