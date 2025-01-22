#include "sddl_view_dialog.h"
#include "ui_sddl_view_dialog.h"

#include "samba/security_descriptor.h"
#include "samba/sddl.h"
#include "samba/dom_sid.h"
#include "globals.h"
#include "ad_config.h"
#include "status.h"
#include "ad_security.h"

SDDLViewDialog::SDDLViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDDLViewDialog),
    sd(nullptr) {

    ui->setupUi(this);

    ui->SDDL_edit->setAlignment(Qt::AlignJustify);
    ui->SDDL_edit->setWordWrapMode(QTextOption::WrapAnywhere);

    connect(ui->show_trustee_checkbox, &QCheckBox::clicked, [this](bool) {
        update(sd);
    });
}

SDDLViewDialog::~SDDLViewDialog() {
    delete ui;
}

void SDDLViewDialog::update(security_descriptor *sd_arg) {
    sd = sd_arg;
    update();
}

void SDDLViewDialog::set_trustee(const QByteArray &trustee_arg) {
    trustee = trustee_arg;

    if (ui->show_trustee_checkbox->isChecked()) {
        update();
    }
}

QString SDDLViewDialog::get_sddl() const {
    QString error = tr("Failed to get SDDL formatted security descriptor");
    if (sd == nullptr) {
        return error;
    }

    TALLOC_CTX *mem_ctx = talloc_new(NULL);
    dom_sid domain_sid;
    const char *domain_sid_str = g_adconfig->domain_sid().toUtf8().constData();
    if (!dom_sid_parse(domain_sid_str, &domain_sid)) {
        error += tr(": Domain sid parse failed");
        talloc_free(mem_ctx);
        return error;
    }

    QString sddl_out;
    if (ui->show_trustee_checkbox->isChecked() && !trustee.isEmpty()) {
        security_descriptor *sd_to_encode = security_descriptor_copy(sd);
        const QList<security_ace> old_dacl = security_descriptor_get_dacl(sd_to_encode);
        QList<security_ace> new_dacl;
        const dom_sid trustee_sid = dom_sid_from_bytes(trustee);
        for (const security_ace &ace : old_dacl) {
            const bool trustee_match = (dom_sid_compare(&ace.trustee, &trustee_sid) == 0);
            if (trustee_match) {
                new_dacl.append(ace);
            }
        }

        ad_security_replace_dacl(sd_to_encode, new_dacl);
        sddl_out = sddl_encode(mem_ctx, sd_to_encode, &domain_sid);
        security_descriptor_free(sd_to_encode);
    }
    else {
        sddl_out = sddl_encode(mem_ctx, sd, &domain_sid);
    }

    talloc_free(mem_ctx);

    if (sddl_out.isEmpty()) {
        error += tr(": SDDL encode failed");
        return error;
    }

    return sddl_out;
}

void SDDLViewDialog::update() {
    const QString sddl_sd = get_sddl();
    ui->SDDL_edit->setPlainText(sddl_sd);
}
