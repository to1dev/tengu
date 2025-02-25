#ifndef NEWWALLETFORM_H
#define NEWWALLETFORM_H

#include <QClipboard>
#include <QDialog>

#include "Consts.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Utils/Encryption.h"
#include "Utils/NameGenerator.h"

#include "Components/LineEditEx.h"
#include "Components/MnemonicView.h"

#include "Wallets/Core/BaseMnemonic.h"
#include "Wallets/Core/SolanaWallet.h"

#include "Databases/Database.h"

#include "Forms/MessageForm.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::UI;
using namespace Daitengu::Utils;
using namespace Daitengu::Wallets;

namespace Ui {
class NewWalletForm;
}

class NewWalletForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("New Wallet");

public:
    explicit NewWalletForm(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~NewWalletForm();

    std::shared_ptr<Wallet> walletRecord() const;

private:
    void refresh();

private Q_SLOTS:
    void ok();

private:
    Ui::NewWalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<WindowManager> windowManager_ {
        std::make_unique<WindowManager>()
    };
    std::unique_ptr<Frameless> frameless_;

    MnemonicView* view_;
    LineEditEx* editName_;

    std::shared_ptr<Wallet> walletRecord_;
};

#endif // NEWWALLETFORM_H
