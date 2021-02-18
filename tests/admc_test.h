/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#ifndef ADMC_TEST_H
#define ADMC_TEST_H

/**
 * Base test class for testing ADMC. Implements init and
 * cleanup f-ns that create a fresh testing environment for
 * each test.
 */

#include <QObject>

#include <QTest>

class ADMCTest : public QObject {
    Q_OBJECT

public slots:
    // NOTE: initTestCase(), cleanupTestCase(), init() and
    // cleanup() are special slots called by QTest.

    // Called before first test
    void initTestCase();
    // Called after last test
    void cleanupTestCase();

    // Called before and after each test
    virtual void init();
    void cleanup();

protected:
    QWidget *parent_widget = nullptr;

};

#endif /* ADMC_TEST_H */

