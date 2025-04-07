// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "WalletPanel.h"

namespace Daitengu::Components {

UserCard::UserCard(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("userCard");

    auto mainLayout = new QVBoxLayout(this);
    auto topLayout = new QHBoxLayout();
    auto bottomLayout = new QHBoxLayout();

    // mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));

    int index = randomIndex(0, RandomAvatars.size() - 1);
    int range = randomIndex(
        RandomAvatars[index].second.start, RandomAvatars[index].second.end);

    SVGWidget* svgAvatar = new SVGWidget(
        QString(":/%1/%2")
            .arg(QString::fromUtf8(RandomAvatars[index].first.data(),
                RandomAvatars[index].first.size()))
            .arg(range),
        this, 6);
    svgAvatar->setImageSize(QSize(AVATAR_SIZE, AVATAR_SIZE));

    topLayout->setSpacing(9);
    topLayout->addWidget(svgAvatar);

    QVBoxLayout* textLayout = new QVBoxLayout();

    nameLabel_ = new QLabel(this);
    nameLabel_->setText(DEFAULT_ADDRESS_NAME);
    nameLabel_->setObjectName(STR_NAME_LABEL);

    QHBoxLayout* addressLayout = new QHBoxLayout();

    addressLabel_ = new ClickableLabel(this);
    addressLabel_->setObjectName(STR_ADDRESS_LABEL);
    QString address(DEFAULT_ADDRESS);
    addressLabel_->setText(hideAddress(address));

    connect(addressLabel_, &ClickableLabel::clicked,
        [&]() { clip::set_text(record_.second.address); });

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
    connect(selectButton, &QPushButton::clicked, this, &UserCard::doSelect);

    bottomLayout->setSpacing(6);
    bottomLayout->addWidget(selectButton);

    mainLayout->setSpacing(20);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    // setLayout(mainLayout);
}

void UserCard::reset(int walletId, int id)
{
    const Address& address = record_.second;
    if (address.walletId == walletId || address.id == id) {
        record_ = Record {};
        nameLabel_->setText(DEFAULT_ADDRESS_NAME);
        addressLabel_->setText(hideAddress(QString(DEFAULT_ADDRESS)));
    }
}

void UserCard::update(
    const std::optional<Wallet>& wallet, const std::optional<Address>& address)
{
    if (wallet.has_value()) {
        const Wallet& _wallet = *wallet;
        if (_wallet.id == record_.second.walletId) {
            record_.first = *wallet;
        }
    }

    if (address.has_value()) {
        const Address& _address = *address;
        if (_address.id == record_.second.id) {
            record_.second = *address;
            nameLabel_->setText(QString::fromStdString(record_.second.name));
            addressLabel_->setText(
                QString::fromStdString(hideAddress(record_.second.address)));
        }
    }
}

void UserCard::setRecord(Record&& record)
{
    record_ = std::move(record);
    if (!record_.second.name.empty() && !record_.second.address.empty()) {
        nameLabel_->setText(QString::fromStdString(record_.second.name));
        addressLabel_->setText(
            QString::fromStdString(hideAddress(record_.second.address)));
    }
}

const Record& UserCard::record_ref() const
{
    return record_;
}

Record UserCard::record() const
{
    return record_;
}

ValueCard::ValueCard(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("valueCard");

    auto mainLayout = new QVBoxLayout(this);
    auto valueLayout = new QVBoxLayout();

    // mainLayout->setContentsMargins(QMargins(0, 9, 0, 9));

    valueLabel_ = new QLabel(this);
    valueLabel_->setObjectName(STR_VALUE_LABEL);
    valueLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

     valueLabel_->setText("0.00000000");

    valueLayout->addWidget(valueLabel_);
    mainLayout->addLayout(valueLayout);

    // setLayout(mainLayout);
}

QString ValueCard::value() const
{
    return value_;
}

void ValueCard::setValue(const QString& newValue)
{
    value_ = newValue;
    valueLabel_->setText(value_);
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
    setObjectName("walletPanel");

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setMinimumWidth(MIN_WIDTH);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(QMargins(0, 0, 0, 0));
    layout->setSpacing(20);

    QWidget* frameUser = new QWidget(this);
    frameUser->setObjectName("walletFrame");

    QVBoxLayout* layoutUser = new QVBoxLayout(frameUser);
    userCard_ = new UserCard(frameUser);
    layoutUser->addWidget(userCard_);
    // frameUser->(layoutUser);

    QWidget* frameValue = new QWidget(this);
    frameValue->setObjectName("walletFrame");

    QVBoxLayout* layoutValue = new QVBoxLayout(frameValue);
    valueCard_ = new ValueCard(frameValue);
    layoutValue->addWidget(valueCard_);
    // frameValue->setLayout(layoutValue);

    LineEditEx* editFilter = new LineEditEx(this);
    editFilter->setObjectName("editFilter");
    editFilter->setMaxLength(64);

    objectsCard_ = new ObjectsCard(this);

    layout->addWidget(frameUser);
    layout->addWidget(frameValue);
    layout->addWidget(editFilter);
    layout->addWidget(objectsCard_, 1);
    // setLayout(layout);
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
