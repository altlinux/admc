#ifndef SUBNETSLISTEDIT_H
#define SUBNETSLISTEDIT_H

#include "attribute_edit.h"

class QListWidget;

class SubnetsListEdit : public AttributeEdit {
    Q_OBJECT

public:
    SubnetsListEdit(QListWidget *subnets_list, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;

private:
    QListWidget *subnets_list_wget;
};

#endif // SUBNETSLISTEDIT_H
