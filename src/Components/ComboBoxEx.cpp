#include "ComboBoxEx.h"

ComboBoxEx::ComboBoxEx(QWidget* parent)
    : QLineEdit(parent)
    , listWidget_(new QListWidget)
{
    listWidget_->setWindowFlags(Qt::Popup);
    connect(listWidget_, &QListWidget::itemClicked, this,
        &ComboBoxEx::itemSelected);
    connect(this, &QLineEdit::textEdited, this, &ComboBoxEx::filterList);
}

void ComboBoxEx::setItems(const QStringList& items)
{
    items_ = items;
    listWidget_->clear();
    listWidget_->addItems(items_);
}

void ComboBoxEx::addItem(const QString& item)
{
    items_.append(item);
    filterList(text());
}

void ComboBoxEx::mousePressEvent(QMouseEvent* event)
{
    QLineEdit::mousePressEvent(event);
    showPopup();
}

void ComboBoxEx::itemSelected(QListWidgetItem* item)
{
    setText(item->text());
    listWidget_->hide();
}

void ComboBoxEx::filterList(const QString& text)
{
    listWidget_->clear();
    for (const QString& item : items_) {
        if (item.contains(text, Qt::CaseInsensitive)) {
            listWidget_->addItem(item);
        }
    }

    if (listWidget_->count() > 0) {
        showPopup();
    } else {
        listWidget_->hide();
    }
}

void ComboBoxEx::showPopup()
{
    if (listWidget_->count() == 0)
        return;

    QPoint pos = mapToGlobal(QPoint(0, height()));
    listWidget_->move(pos);
    listWidget_->resize(width(), 150);
    listWidget_->show();
}
