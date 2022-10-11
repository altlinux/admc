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

}
