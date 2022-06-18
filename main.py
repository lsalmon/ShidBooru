import sys
import os
import os.path
from enum import Enum
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

class tagpopupenum(Enum):
    ADD = 1
    DELETE = 2
    SHOW = 3


class mainwindow(QWidget):
    def __init__(self, parent=None):
        super(mainwindow, self).__init__(parent)
        # Class vars
        self.imgs = []
        self.path = "/tmp/samples"
        self.valid_images = [".jpg", ".gif", ".png"]
        self.selected_item = None
        self.list_global_tags = None
        print(self.imgs)

        hbox_main = QHBoxLayout()
        vbox_search = QVBoxLayout()
        vbox_imglist = QVBoxLayout()

        # Search bar and button
        search_label = QLabel(self)
        search_label.setText('Search by tags')
        search_label.setAlignment(Qt.AlignCenter)
        self.search_entry = QLineEdit(self)
        search_btn = QPushButton('Search', self)
        search_btn.clicked.connect(self.searchbytag)
        vbox_search.addWidget(search_label)
        vbox_search.addWidget(self.search_entry)
        vbox_search.addWidget(search_btn)

        # Refresh button 
        # (refresh when images have been added or removed)
        refresh_btn = QPushButton('Refresh', self)
        refresh_btn.clicked.connect(self.refreshlayout)
        vbox_search.addWidget(refresh_btn)

        # Global list of tags
        tag_label = QLabel(self)
        tag_label.setText('List of tags')
        tag_label.setAlignment(Qt.AlignCenter)
        self.list_global_tags = QListWidget(self)
        vbox_search.addWidget(tag_label)
        vbox_search.addWidget(self.list_global_tags)

        # Load initial list of images
        self.list_images = QListWidget(self)
        self.list_images.setViewMode(QListView.ListMode)
        self.list_images.setIconSize(QSize(120,120))
        self.refresh()
        self.list_images.clear()
        for img in self.imgs:
            self.additem(img)
        # Tag context window when clicking on an image item
        self.list_images.setContextMenuPolicy(Qt.CustomContextMenu)
        self.list_images.customContextMenuRequested.connect(self.tagcontextmenu)

        self.action_add = QAction(self)
        self.action_add.setObjectName('add_tag')        
        self.action_add.setText('Add tag')

        self.action_del = QAction(self)
        self.action_del.setObjectName('delete_tag')        
        self.action_del.setText('Delete tag')

        self.action_show = QAction(self)
        self.action_show.setObjectName('show_tags')        
        self.action_show.setText('Show tags')  

        self.custom_menu_img = QMenu('Menu', self.list_images)       
        self.custom_menu_img.addAction(self.action_add)
        self.custom_menu_img.addAction(self.action_del)
        self.custom_menu_img.addAction(self.action_show)

        self.action_add.triggered.connect(self.addtag)
        self.action_del.triggered.connect(self.deletetag)
        self.action_show.triggered.connect(self.showtag)

        # Popup window when double clicking on an image item
        self.list_images.itemDoubleClicked.connect(self.displayimage)
        vbox_imglist.addWidget(self.list_images)

        # Combine layouts
        hbox_main.addLayout(vbox_search, Qt.AlignLeft)
        hbox_main.addLayout(vbox_imglist, Qt.AlignRight)
        self.setLayout(hbox_main)

    def additem(self, img):
        img_icon = QIcon()
        item = QListWidgetItem()
        # Initalize tag array attached to item
        item.setData(Qt.UserRole, [])
        # Set image and image name to item
        img_icon.addPixmap(QPixmap(img))
        item.setIcon(img_icon)
        item.setText(img.rsplit('/', 1)[1])
        # Push item to list
        self.list_images.addItem(item)

    def refreshlayout(self):
        self.refresh()
        # Get all images filename from items
        items = [self.list_images.item(i).text() for i in range(self.list_images.count())]
        imgs = [self.imgs[i].rsplit('/', 1)[1] for i in range(len(self.imgs))]
        # Get images removed from directory
        items_to_remove = list(set(items) - set(imgs))
        for item in items_to_remove:
            for i in range(self.list_images.count()):
                if self.list_images.item(i).text() == item:
                    self.list_images.takeItem(i)
                    break
        
        # Get images added to directory
        items_to_add = list(set(imgs) - set(items))
        for item in items_to_add:
            self.additem(self.path+'/'+item)

    def refresh(self):
        self.imgs = []
        for f in os.listdir(self.path):
            ext = os.path.splitext(f)[1]
            if ext.lower() not in self.valid_images:
                continue
            self.imgs.append(os.path.join(self.path, f))

    def searchbytag(self):
        search_tags = self.search_entry.text().split(',')

        print(search_tags)
        # If user pushed the button without a tag (['']),
        # simply display the whole list
        if len(search_tags) == 1 and '' in search_tags:
            for i in range(self.list_images.count()):
                if self.list_images.item(i).isHidden():
                    self.list_images.item(i).setHidden(False)
        else:
            # Hide widgets who dont contain the tag(s)
            for i in range(self.list_images.count()):
                if set(search_tags).issubset(set(self.list_images.item(i).data(Qt.UserRole))):
                    self.list_images.item(i).setHidden(False)
                else:
                    self.list_images.item(i).setHidden(True)

    def tagcontextmenu(self, event):
        self.selected_item = self.list_images.itemAt(event)
        self.custom_menu_img.popup(QCursor.pos())

    def addtag(self):
        if self.selected_item != None:
            self.popup = tagpopup(self.selected_item, tagpopupenum.ADD, self.list_global_tags, self.list_images)
            self.popup.show()

    def deletetag(self):
        if self.selected_item != None:
            self.popup = tagpopup(self.selected_item, tagpopupenum.DELETE, self.list_global_tags, self.list_images)
            self.popup.show()

    def showtag(self):
        if self.selected_item != None:
            self.popup = tagpopup(self.selected_item, tagpopupenum.SHOW)
            self.popup.show()

    def displayimage(self, item):
        # Create popup with full path to image 
        img_path = self.path+'/'+item.text()
        self.popup = imagepopup(img_path)
        self.popup.show()


# Double click image preview
class imagepopup(QWidget):
    def __init__(self, img_path):
        super().__init__()
        self.label = QLabel(self)
        self.pixmap = QPixmap(img_path)
        self.label.setPixmap(self.pixmap)
        self.label.resize(self.pixmap.width(), self.pixmap.height())

# Right click menu
class tagpopup(QWidget):
    def __init__(self, item, tag_popup_enum, tags_global_list=[], list_images=[]):
        super().__init__()
        self.item = item
        self.tags_global_list = tags_global_list
        self.list_images = list_images
        self.tag_array = item.data(Qt.UserRole)
        main_layout = QVBoxLayout()
        self.setLayout(main_layout)
        if tag_popup_enum == tagpopupenum.ADD:
            add_label = QLabel(self)
            add_label.setText('Add the following tags, separated by a comma')
            self.tag_entry = QLineEdit(self)
            tag_entry_btn = QPushButton('Add', self)
            tag_entry_btn.clicked.connect(self.addtag)
            main_layout.addWidget(add_label)
            main_layout.addWidget(self.tag_entry)
            main_layout.addWidget(tag_entry_btn)
        elif tag_popup_enum == tagpopupenum.DELETE:
            del_label = QLabel(self)
            del_label.setText('Delete the following tags, separated by a comma')
            self.tag_entry = QLineEdit(self)
            tag_entry_btn = QPushButton('Delete', self)
            tag_entry_btn.clicked.connect(self.deltag)
            main_layout.addWidget(del_label)
            main_layout.addWidget(self.tag_entry)
            main_layout.addWidget(tag_entry_btn)
        elif tag_popup_enum == tagpopupenum.SHOW:
            tag_label = QLabel(self)
            tag_label.setText('List of tags for item '+item.text())
            self.list_tags = QListWidget(self)
            item_tag_array = item.data(Qt.UserRole)
            for tag in item_tag_array:
                print("tagpopupenum SHOW "+tag)
            self.list_tags.addItems(item_tag_array)
            main_layout.addWidget(tag_label)
            main_layout.addWidget(self.list_tags)

    def addtag(self):
        added_tags = self.tag_entry.text().split(',')
        item_tag_array = self.item.data(Qt.UserRole)
        for tag in added_tags:
            tag = tag.lstrip()
            if tag not in item_tag_array:
                print("tagpopupenum ADD "+tag)
                self.tag_array.append(tag)
            # Update list of all existing tags
            if len(self.tags_global_list.findItems(tag, Qt.MatchExactly)) == 0:
                self.tags_global_list.addItem(tag)

        self.item.setData(Qt.UserRole, self.tag_array)
        self.close()

    def deltag(self):
        deleted_tags = self.tag_entry.text().split(',')
        selected_item_row = self.list_images.row(self.item)
        for tag in deleted_tags:
            tag = tag.lstrip()
            print("tagpopupenum DEL "+tag)
            self.tag_array = list(filter(lambda elem: elem != tag, self.tag_array))
        
            # Update list of all existing tags
            # if none of the other images have the tag
            tag_removed = True

            for i in range(self.list_images.count()):
                item_tags = self.list_images.item(i).data(Qt.UserRole)

                if selected_item_row != i and tag in item_tags:
                    tag_removed = False
                    break

            if tag_removed:
                items_global_list = self.tags_global_list.findItems(tag, Qt.MatchExactly)
                if len(items_global_list) > 0:
                    # It should contain only one item
                    for item_list in items_global_list:
                        self.tags_global_list.takeItem(self.tags_global_list.row(item_list))

        self.item.setData(Qt.UserRole, self.tag_array)
        self.close()

def main():
    app = QApplication(sys.argv)
    ex = mainwindow()
    ex.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
