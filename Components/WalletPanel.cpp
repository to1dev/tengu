#include "WalletPanel.h"

namespace Daitengu::Components {

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
        QString(":/%1/%2").arg(RandomAvatars[index].first).arg(range), this, 3);
    svgAvatar->setFixedSize(AVATAR_SIZE, AVATAR_SIZE);

    topLayout->setSpacing(6);
    topLayout->addWidget(svgAvatar);

    QVBoxLayout* textLayout = new QVBoxLayout();

    nameLabel_ = new QLabel(this);
    nameLabel_->setText("Satoshi Nakamoto");
    nameLabel_->setObjectName(STR_NAME_LABEL);

    QHBoxLayout* addressLayout = new QHBoxLayout();

    addressLabel_ = new ClickableLabel(this);
    addressLabel_->setObjectName(STR_ADDRESS_LABEL);
    QString address("So11111111111111111111111111111111111111112");
    addressLabel_->setCursor(Qt::PointingHandCursor);
    addressLabel_->setText(hideAddress(address));

    addressLayout->addWidget(addressLabel_);
    addressLayout->addStretch(1);

    textLayout->setSpacing(6);
    textLayout->addWidget(nameLabel_);
    textLayout->addLayout(addressLayout);

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

ValueCard::ValueCard(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("valueCard");
    auto* mainLayout = new QVBoxLayout(this);
    auto* valueLayout = new QVBoxLayout();

    valueLabel_ = new QLabel(this);
    valueLabel_->setObjectName(STR_VALUE_LABEL);
    valueLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    valueLabel_->setText("123,456.7890 SOL");

    valueLayout->addWidget(valueLabel_);
    mainLayout->addLayout(valueLayout);
}

std::uint64_t ValueCard::value() const
{
    return value_;
}

void ValueCard::setValue(std::uint64_t newValue)
{
    value_ = newValue;
}

ObjectsCard::ObjectsCard(QWidget* parent)
    : AnimatedTabWidget(parent)
{
    setObjectName("objectsCard");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

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
    valueCard_ = new ValueCard(frameValue);
    layoutValue->addWidget(valueCard_);
    frameValue->setLayout(layoutValue);

    QWidget* frameObjects = new QWidget(this);
    frameObjects->setObjectName("objectsFrame");

    QVBoxLayout* layoutObjects = new QVBoxLayout(frameObjects);
    layoutObjects->setSpacing(9);

    LineEditEx* editFilter = new LineEditEx(frameObjects);
    editFilter->setObjectName("editFilter");
    editFilter->setMaxLength(64);

    objectsCard_ = new ObjectsCard(this);

    layoutObjects->addWidget(editFilter);
    layoutObjects->addWidget(objectsCard_, 1);
    frameObjects->setLayout(layoutObjects);

    layout->addWidget(frameUser);
    layout->addWidget(frameValue);
    layout->addWidget(frameObjects, 1);
    setLayout(layout);
}

UserCard* WalletPanel::userCard() const
{
    return userCard_;
}

ValueCard* WalletPanel::valueCard() const
{
    return valueCard_;
}

ObjectsCard* WalletPanel::objectsCard() const
{
    return objectsCard_;
}

}
