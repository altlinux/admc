#include "language_updater.h"
#include "settings.h"
#include "ad_utils.h"

#include <QLocale>
#include <QCoreApplication>
#include <QDebug>
#include <QLibraryInfo>

QList<QTranslator*> LanguageUpdater::translators_;

LanguageUpdater::LanguageUpdater()
{

}

void LanguageUpdater::load_translators()
{
    const QLocale saved_locale = settings_get_variant(SETTING_locale).toLocale();

    for (int i = 0; i < translators_.size(); ++i)
    {
        qApp->removeTranslator(translators_.at(i));
    }

    translators_.clear();

    QTranslator* translator(new QTranslator());
    const bool loaded_admc_translation = translator->load(saved_locale, "admc", "_", ":/admc");
    qApp->installTranslator(translator);

    if (!loaded_admc_translation) {
        qDebug() << "Failed to load admc translation";
    }
    else {
        translators_.append(translator);
    }

        QTranslator* adldap_translator(new QTranslator());
    const bool loaded_adldap_translation = load_adldap_translation(*adldap_translator, saved_locale);
    qApp->installTranslator(adldap_translator);

    if (!loaded_adldap_translation) {
        qDebug() << "Failed to load adldap translation";
    }
    else {
        translators_.append(adldap_translator);
    }

    // NOTE: these translations are for qt-defined text, like standard dialog buttons
    QTranslator* qt_translator(new QTranslator());
    const bool loaded_qt_translation = qt_translator->load(saved_locale, "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(qt_translator);

    if (!loaded_qt_translation) {
        qDebug() << "Failed to load qt translation";
    }
    else {
        translators_.append(qt_translator);
    }

    QTranslator* qtbase_translator(new QTranslator());
    const bool loaded_qtbase_translation = qtbase_translator->load(saved_locale, "qtbase", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(qtbase_translator);

    if (!loaded_qtbase_translation) {
        qDebug() << "Failed to load qt base translation";
    }
    else {
        translators_.append(qtbase_translator);
    }



}
