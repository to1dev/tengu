#include "ComboBoxEx.h"

PopupListWidget::PopupListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setObjectName("popupList");
}

void PopupListWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (currentItem()) {
            Q_EMIT enterKeyPressed(currentItem());
            event->accept();
            return;
        }
    }

    QListWidget::keyPressEvent(event);
}

ComboBoxEx::ComboBoxEx(QWidget* parent)
    : QLineEdit(parent)
    , listWidget_(new PopupListWidget(nullptr))
    , currentIndex_(-1)
{
    setReadOnly(true);

    listWidget_->setWindowFlags(Qt::Popup);
    listWidget_->setFocusPolicy(Qt::NoFocus);
    listWidget_->setAttribute(Qt::WA_DeleteOnClose, false);

    connect(listWidget_, &QListWidget::itemClicked, this,
        &ComboBoxEx::itemSelected);
    connect(listWidget_, &PopupListWidget::enterKeyPressed, this,
        &ComboBoxEx::handleEnterKeyPressed);
    connect(this, &QLineEdit::textEdited, this, &ComboBoxEx::filterList);

    setupEventFilter();
}

ComboBoxEx::~ComboBoxEx()
{
    if (listWidget_) {
        listWidget_->removeEventFilter(this);
        listWidget_->deleteLater();
    }
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

void ComboBoxEx::setCurrentIndex(int index)
{
    if (index < -1 || index >= items_.size()) {
        return;
    }

    if (index == -1) {
        currentIndex_ = -1;
        setText("");
    } else {
        currentIndex_ = index;
        setText(items_[index]);
    }

    Q_EMIT currentIndexChanged(currentIndex_);
}

void ComboBoxEx::mousePressEvent(QMouseEvent* event)
{
    QLineEdit::mousePressEvent(event);
    showPopup();
}

void ComboBoxEx::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Down:
    case Qt::Key_Up:
        showPopup();
        if (listWidget_->count() > 0) {
            listWidget_->setCurrentRow(
                listWidget_->currentRow() < 0 ? 0 : listWidget_->currentRow());
            listWidget_->setFocus();
        }
        event->accept();
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (listWidget_->isVisible() && listWidget_->currentItem()) {
            handleEnterKeyPressed(listWidget_->currentItem());
            event->accept();
        } else {
            QLineEdit::keyPressEvent(event);
        }
        break;

    default:
        QLineEdit::keyPressEvent(event);
        break;
    }
}

bool ComboBoxEx::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (listWidget_->isVisible()) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

            QPoint globalPos;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            globalPos = mouseEvent->globalPosition().toPoint();
#else
            globalPos = mouseEvent->globalPos();
#endif

            if (!listWidget_->geometry().contains(globalPos)
                && !QRect(mapToGlobal(QPoint(0, 0)), size())
                    .contains(globalPos)) {
                hidePopup();
                return true;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void ComboBoxEx::itemSelected(QListWidgetItem* item)
{
    if (!item)
        return;

    currentIndex_ = listWidget_->row(item);
    setText(item->text());
    Q_EMIT currentIndexChanged(currentIndex_);
    hidePopup();
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
        hidePopup();
    }
}

void ComboBoxEx::handleEnterKeyPressed(QListWidgetItem* item)
{
    itemSelected(item);
}

void ComboBoxEx::showPopup()
{
    if (listWidget_->count() == 0)
        return;

    QPoint pos = mapToGlobal(QPoint(0, height() + 3));
    listWidget_->move(pos);
    listWidget_->resize(width(), 150);

    if (listWidget_->currentRow() < 0 && listWidget_->count() > 0) {
        listWidget_->setCurrentRow(0);
    }

    if (currentIndex_ >= 0 && currentIndex_ < listWidget_->count()) {
        for (int i = 0; i < listWidget_->count(); ++i) {
            if (listWidget_->item(i)->text() == items_[currentIndex_]) {
                listWidget_->setCurrentRow(i);
                break;
            }
        }
    }

    listWidget_->show();
}

void ComboBoxEx::hidePopup()
{
    listWidget_->hide();
    setFocus();
}

void ComboBoxEx::setupEventFilter()
{
    listWidget_->installEventFilter(this);
}
