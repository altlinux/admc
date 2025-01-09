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

#ifndef SEARCH_THREAD_H
#define SEARCH_THREAD_H

/**
 * A thread that performs an AD search operation. Useful for
 * searches that are expected to take a long time. For
 * regular small searches this is overkill. results_ready()
 * signal returns search results as they arrive. If search
 * has multiple pages, then results_ready() will be emitted
 * multiple times. Use stop() to stop search. Note that search is
 * not stopped immediately but when current results page is
 * done processing. Note that creator of thread should call
 * thread's deleteLater() in the finished() slot.
 */

#include <QThread>

#include "ad_defines.h"

class AdObject;
class AdMessage;

class SearchThread final : public QThread {
    Q_OBJECT

public:
    SearchThread(const QString base, const SearchScope scope, const QString &filter, const QList<QString> attributes);

    void stop();
    int get_id() const;
    bool failed_to_connect() const;
    bool hit_object_display_limit() const;
    QList<AdMessage> get_ad_messages() const;

signals:
    void results_ready(const QHash<QString, AdObject> &results);
    void over_object_display_limit();

private:
    bool stop_flag;
    QString base;
    SearchScope scope;
    QString filter;
    QList<QString> attributes;
    int id;
    bool m_failed_to_connect;
    bool m_hit_object_display_limit;
    QList<AdMessage> ad_messages;

    void run() override;
};

// Call this in your finished() slot to display any
// error dialogs. Search thread can't display them
// because it is run in non-GUI thread.
void search_thread_display_errors(SearchThread *thread, QWidget *parent);

#endif /* SEARCH_THREAD_H */
