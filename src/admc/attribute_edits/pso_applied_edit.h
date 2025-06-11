#ifndef PSOAPPLIEDEDIT_H
#define PSOAPPLIEDEDIT_H

/**
 * Edit for displaying name of object in a label. Used
 * in general tabs of the properties dialog
 */

#include "attribute_edits/attribute_edit.h"

class QLabel;

class PSOAppliedEdit final : public AttributeEdit {
    Q_OBJECT

public:
    PSOAppliedEdit(QLabel *label, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;

private:
    QLabel *applied_pso_label;
};

#endif // PSOAPPLIEDEDIT_H
