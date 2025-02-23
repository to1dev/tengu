#include "MessageForm.h"
#include "ui_MessageForm.h"

MessageForm::MessageForm(QWidget* parent, const QString& text, int buttons,
    const QString& title, int emoji)
    : QDialog(parent)
    , ui(new Ui::MessageForm)
{
    ui->setupUi(this);

    setWindowTitle(title);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init();

    SVGWidget* icon = nullptr;
    if (emoji > 0 && emoji < 21) {
        icon = new SVGWidget(QString(":/Emoji/%1").arg(emoji), ui->labelIcon);
    } else {
        icon = new SVGWidget(
            QString(":/Emoji/%1").arg(randomIndex(1, 20)), ui->labelIcon);
    }
    if (icon) {
        icon->setImageSize(QSize(LOGO_SIZE, LOGO_SIZE));
        ui->labelIcon->setFixedWidth(128);
    }

    ui->labelText->setText(text);

    if (buttons & MessageButton::Ok) {
        connect(
            ui->ButtonOK, &QPushButton::clicked, this, &MessageForm::accept);
    } else {
        ui->ButtonOK->setVisible(false);
    }

    if (buttons & MessageButton::Cancel) {
        connect(ui->ButtonCancel, &QPushButton::clicked, this,
            &MessageForm::reject);
    } else {
        ui->ButtonCancel->setVisible(false);
    }
}

MessageForm::~MessageForm()
{
    delete ui;
}
