#include "choose_theme_dialog.h"
#include "ui_choose_theme_dialog.h"
#include "console_impls/select_theme_impl.h"

ChooseThemeDialog::ChooseThemeDialog(QWidget * parent):
    QDialog(parent)
{
    ui = new Ui::ChooseThemeDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    setup_theme_list();

    connect(ui->button_box, &QDialogButtonBox::accepted, this, &ChooseThemeDialog::accepted);
}

void ChooseThemeDialog::setup_theme_list()
{
    auto themes = SelectThemeImpl().get_available_theme_list();

    theme_model = new QStringListModel(this);

    theme_model->setStringList(themes);

    ui->theme_list->setModel(theme_model);
}

void ChooseThemeDialog::accepted()
{
    SelectThemeImpl().apply_theme(ui->theme_list->currentIndex().data().toString());
}
