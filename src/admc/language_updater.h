#ifndef LANGUAGEUPDATER_H
#define LANGUAGEUPDATER_H

#include <QList>
#include <QTranslator>

class LanguageUpdater
{
public:
    LanguageUpdater();
    static void load_translators();
private:
    static QList<QTranslator*> translators_;
};

#endif // LANGUAGEUPDATER_H
