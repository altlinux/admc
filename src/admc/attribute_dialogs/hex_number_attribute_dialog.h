#ifndef HEX_NUMBER_ATTRIBUTE_DIALOG_H
#define HEX_NUMBER_ATTRIBUTE_DIALOG_H

#include "attribute_dialogs/attribute_dialog.h"

namespace Ui {
class HexNumberAttributeDialog;
}

class HexNumberAttributeDialog final : public AttributeDialog
{
    Q_OBJECT

public:
    HexNumberAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent);
    ~HexNumberAttributeDialog();

    QList<QByteArray> get_value_list() const override;

private:
    Ui::HexNumberAttributeDialog *ui;
};

#endif // HEX_NUMBER_ATTRIBUTE_DIALOG_H
