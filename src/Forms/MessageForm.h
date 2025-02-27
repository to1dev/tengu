#ifndef MESSAGEFORM_H
#define MESSAGEFORM_H

#include <QDialog>

#include "Components/SVGWidget.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class MessageForm;
}

enum MessageButton {
    NoButton = 0x00000000,
    Ok = 0x00000400,
    Save = 0x00000800,
    Yes = 0x00004000,
    No = 0x00010000,
    Abort = 0x00040000,
    Retry = 0x00080000,
    Ignore = 0x00100000,
    Close = 0x00200000,
    Cancel = 0x00400000,
    Discard = 0x00800000,
    Help = 0x01000000,
    Apply = 0x02000000,
    Reset = 0x04000000,
};

class MessageForm : public QDialog {
    Q_OBJECT

public:
    explicit MessageForm(QWidget* parent = nullptr,
        const QString& text = QString(),
        const QString& title = QObject::tr("Tips"),
        int buttons = MessageButton::Ok, int emoji = -1);
    ~MessageForm();

private:
    Ui::MessageForm* ui;

    std::unique_ptr<Frameless> frameless_;
};

#endif // MESSAGEFORM_H
