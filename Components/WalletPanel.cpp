#include "WalletPanel.h"

namespace Daitengu::Components {

WalletPanel::WalletPanel(QWidget* parent)
    : QFrame(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumWidth(MIN_WIDTH);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(QMargins(0, 0, 0, 0));
    layout->setSpacing(9);

    QWidget* frameUser = new QWidget(this);
    frameUser->setObjectName("walletFrame");

    QHBoxLayout* layoutUser = new QHBoxLayout(frameUser);
    userCard_ = new UserCard(frameUser);
    layoutUser->addWidget(userCard_);
    frameUser->setLayout(layoutUser);

    QWidget* frameValue = new QWidget(this);
    frameValue->setObjectName("walletFrame");

    QVBoxLayout* layoutValue = new QVBoxLayout(frameValue);
    frameValue->setLayout(layoutValue);

    layout->addWidget(frameUser);
    layout->addWidget(frameValue, 1);
    setLayout(layout);
}

UserCard* WalletPanel::userCard() const
{
    return userCard_;
}

UserCard::UserCard(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("userCard");
    auto* mainLayout = new QVBoxLayout(this);
    auto* topLayout = new QHBoxLayout();
    auto* bottomLayout = new QHBoxLayout();

    int index = randomIndex(0, RandomAvatars.size() - 1);
    int range = randomIndex(
        RandomAvatars[index].second.start, RandomAvatars[index].second.end);
    SVGWidget* svgAvatar = new SVGWidget(
        QString(":/%1/%2").arg(RandomAvatars[index].first).arg(range), this);
    svgAvatar->setFixedSize(AVATAR_SIZE, AVATAR_SIZE);
    topLayout->setSpacing(6);
    topLayout->addWidget(svgAvatar);

    QVBoxLayout* textLayout = new QVBoxLayout();
    nameLabel_ = new QLabel(this);
    nameLabel_->setObjectName(STR_NAME_LABEL);
    addressLabel_ = new ClickableLabel(this);
    addressLabel_->setObjectName(STR_ADDRESS_LABEL);
    QString address("So11111111111111111111111111111111111111112");
    addressLabel_->setCursor(Qt::PointingHandCursor);
    addressLabel_->setText(address);
    textLayout->setSpacing(6);
    textLayout->addWidget(nameLabel_);
    textLayout->addWidget(addressLabel_);
    topLayout->addLayout(textLayout);

    QPushButton* selectButton = new QPushButton(STR_SELECT_BUTTON_TEXT, this);
    selectButton->setObjectName(STR_SELECT_BUTTON);
    selectButton->setMinimumHeight(30);
    selectButton->setMaximumHeight(30);
    bottomLayout->setSpacing(6);
    bottomLayout->addWidget(selectButton);

    mainLayout->setSpacing(20);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
}

}
