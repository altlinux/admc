/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef CONNECTION_OPTIONS_DIALOG_H
#define CONNECTION_OPTIONS_DIALOG_H

/**
 * Dialog that allows user to change connection options.
 */

#include <QDialog>

extern const QString CERT_STRATEGY_NEVER;
extern const QString CERT_STRATEGY_HARD;
extern const QString CERT_STRATEGY_DEMAND;
extern const QString CERT_STRATEGY_ALLOW;
extern const QString CERT_STRATEGY_TRY;

namespace Ui {
    class ConnectionOptionsDialog;
}

class ConnectionOptionsDialog : public QDialog {
    Q_OBJECT

public:
    Ui::ConnectionOptionsDialog *ui;

    ConnectionOptionsDialog(QWidget *parent);

    void accept() override;
    void reject() override;

private:    
    void reset();
    void restore_defaults();
};

#endif /* CONNECTION_OPTIONS_DIALOG_H */
