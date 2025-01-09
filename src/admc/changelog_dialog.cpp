/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "changelog_dialog.h"
#include "ui_changelog_dialog.h"

#include "config.h"
#include "settings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>

ChangelogDialog::ChangelogDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ChangelogDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    const QString changelog_text = []() {
        const QString fail_text = tr("Failed to open changelog file.");

        const QString changelog_path = []() {
            const QString changelog_file_name = [&]() {
                const QLocale saved_locale = settings_get_variant(SETTING_locale).toLocale();

                if (saved_locale.language() == QLocale::Russian) {
                    return "CHANGELOG_ru.txt";
                } else {
                    return "CHANGELOG.txt";
                }
            }();

#ifdef QT_DEBUG
            return QString("%1/%2").arg(QCoreApplication::applicationDirPath(), changelog_file_name);
#endif

            return QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString("doc/admc-%1/%2").arg(ADMC_VERSION, changelog_file_name));
        }();

        if (changelog_path.isEmpty()) {
            return fail_text;
        }

        QFile file(changelog_path);

        const bool open_success = file.open(QIODevice::ReadOnly);
        if (!open_success) {
            qDebug() << "Failed to open changelog file";

            return fail_text;
        }

        QString out = file.readAll();

        file.close();

        // Remove forced word wrap contained in
        // CHANGELOG.txt so that resizing the dialog
        // expands text width (all wrapped lines start
        // with 2 spaces)
        out.replace("\n  ", " ");

        return out;
    }();

    ui->edit->setPlainText(changelog_text);

    settings_setup_dialog_geometry(SETTING_changelog_dialog_geometry, this);
}

ChangelogDialog::~ChangelogDialog() {
    delete ui;
}
