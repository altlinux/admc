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

#include "find_widget.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/select_base_widget.h"
#include "find_results.h"
#include "globals.h"
#include "search_thread.h"
#include "settings.h"
#include "utils.h"

#include <QFormLayout>
#include <QFrame>
#include <QList>
#include <QPushButton>

FindWidget::FindWidget(const QList<QString> classes, const QString &default_base)
: QWidget() {
    select_base_widget = new SelectBaseWidget(g_adconfig, default_base);

    filter_widget = new FilterWidget(g_adconfig, classes);

    find_button = new QPushButton(tr("Find"));
    find_button->setDefault(true);
    find_button->setObjectName("find_button");

    stop_button = new QPushButton(tr("Stop"));
    stop_button->setAutoDefault(false);

    find_results = new FindResults();

    auto filter_widget_frame = new QFrame();
    filter_widget_frame->setFrameStyle(QFrame::Raised);
    filter_widget_frame->setFrameShape(QFrame::Box);

    {
        auto select_base_layout_inner = new QFormLayout();
        select_base_layout_inner->addRow(tr("Search in:"), select_base_widget);

        auto select_base_layout = new QHBoxLayout();
        select_base_layout->addLayout(select_base_layout_inner);
        select_base_layout->addStretch();

        auto button_layout = new QHBoxLayout();
        button_layout->addWidget(find_button);
        button_layout->addWidget(stop_button);
        button_layout->addStretch();

        auto layout = new QVBoxLayout();
        filter_widget_frame->setLayout(layout);
        layout->addLayout(select_base_layout);
        layout->addWidget(filter_widget);
        layout->addLayout(button_layout);
    }

    {
        auto layout = new QHBoxLayout();
        setLayout(layout);
        layout->addWidget(filter_widget_frame);
        layout->addWidget(find_results);
    }

    // Keep filter widget compact, so that when user
    // expands find dialog horizontally, filter widget will
    // keep it's size, find results will get expanded
    filter_widget_frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    find_results->setMinimumSize(500, 0);

    connect(
        find_button, &QPushButton::clicked,
        this, &FindWidget::find);

    // NOTE: need this for the case where dialog is closed
    // while a search is in progress. Without this busy
    // indicator stays on.
    connect(
        this, &QObject::destroyed,
        []() {
            hide_busy_indicator();
        });
}

ConsoleWidget *FindWidget::get_console() const {
    return find_results->get_console();
}

void FindWidget::find() {
    // Prepare search args
    const QString filter = filter_widget->get_filter();
    const QString base = select_base_widget->get_base();
    const QList<QString> search_attributes = console_object_search_attributes();

    auto find_thread = new SearchThread(base, SearchScope_All, filter, search_attributes);

    connect(
        find_thread, &SearchThread::results_ready,
        this, &FindWidget::handle_find_thread_results);
    connect(
        this, &QObject::destroyed,
        find_thread, &SearchThread::stop);
    connect(
        stop_button, &QPushButton::clicked,
        find_thread, &SearchThread::stop);
    connect(
        find_thread, &SearchThread::finished,
        this, &FindWidget::on_thread_finished);

    show_busy_indicator();

    // NOTE: disable find button, otherwise another find
    // process can start while this one isn't finished!
    find_button->setEnabled(false);

    find_results->clear();

    find_thread->start();
}

void FindWidget::handle_find_thread_results(const QHash<QString, AdObject> &results) {
    find_results->load(results);
}

void FindWidget::on_thread_finished() {
    find_button->setEnabled(true);

    hide_busy_indicator();
}

QList<QString> FindWidget::get_selected_dns() const {
    return find_results->get_selected_dns();
}
