#ifndef ADMC_TRANSLATOR_H
#define ADMC_TRANSLATOR_H

#include <QObject>
#include <QLocale>
#include <QTranslator>

/**
 * This class implements a global application translator using a singleton
 * pattern.
 */
class AdmcTranslator : public QObject {
    Q_OBJECT

public:
    /**
     * Get the global instance of the ADMC translator.
     */
    static AdmcTranslator& get_instance() {
        static AdmcTranslator instance;
        return instance;
    }

    AdmcTranslator(AdmcTranslator const&)  = delete;
    void operator=(AdmcTranslator const&)  = delete;

    bool load_saved_locale();
    bool load_locale(QLocale locale);
    bool save_current_locale();
    QLocale get_current_locale();

private:
    QTranslator* translator;
    QTranslator* adldap_translator;
    QTranslator* qt_translator;
    QTranslator* qtbase_translator;
    QLocale current_locale;

    AdmcTranslator();
};

#endif  /* ifndef ADMC_TRANSLATOR_H */
