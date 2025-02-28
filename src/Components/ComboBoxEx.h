#pragma once

#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QVBoxLayout>

class PopupListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit PopupListWidget(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

Q_SIGNALS:
    void enterKeyPressed(QListWidgetItem* currentItem);
};

class ComboBoxEx : public QLineEdit {
    Q_OBJECT

public:
    explicit ComboBoxEx(QWidget* parent = nullptr);
    ~ComboBoxEx() override;

    void setItems(const QStringList& items);
    void addItem(const QString& item);

    [[nodiscard]] int currentIndex() const
    {
        return currentIndex_;
    }

    [[nodiscard]] QString currentText() const
    {
        return text();
    }

    void setCurrentIndex(int index);

Q_SIGNALS:
    void currentIndexChanged(int index);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
    void itemSelected(QListWidgetItem* item);
    void filterList(const QString& text);
    void handleEnterKeyPressed(QListWidgetItem* item);

private:
    void showPopup();
    void hidePopup();
    void setupEventFilter();

    PopupListWidget* listWidget_;
    QStringList items_;
    int currentIndex_;
};
