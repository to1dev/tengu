#pragma once

#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QVBoxLayout>

class ComboBoxEx : public QLineEdit {
    Q_OBJECT

public:
    explicit ComboBoxEx(QWidget* parent = nullptr);

    void setItems(const QStringList& items);
    void addItem(const QString& item);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private Q_SLOTS:
    void itemSelected(QListWidgetItem* item);
    void filterList(const QString& text);

private:
    void showPopup();

    QListWidget* listWidget_;
    QStringList items_;
};
