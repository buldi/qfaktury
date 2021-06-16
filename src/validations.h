#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef VALIDATIONS_H
#define VALIDATIONS_H

#include <QObject>

#include "fwd.hpp"

// class for data validation, putted into widgets
class Validations final : public QObject
{
    Q_OBJECT

public:
    explicit Validations(QObject *parent = nullptr);
    ~Validations();

    static Validations *instance();
    bool validateNIP(QString);
    bool validatePESEL(QString);
    bool validatePass(QString);
    bool validateAccount(QString);
    bool validateEmail(QString);
    bool validateWebsite(QString);
    bool validateTel(QString);
    bool validatePkwiu(QString);
    bool validateIDCard(QString);
    bool validateRegon(QString);
    bool validateZip(QString);
    bool isEmptyField(QLineEdit *, QString);
    bool isEmptyField(QString, QString);
    bool checkSumNIP(QString);
    bool checkSumREGON(QString);
    bool checkSumPESEL(QString);
    bool checkSumAccount(QString);
    bool checkSumIDCard(QString);
    bool checkSumPass(QString);

private:
    static Validations *m_instance;

    static QVector<int> validateNumb(
        int i1,
        int i2,
        int i3,
        int i4,
        int i5,
        int i6,
        int i7 = -1,
        int i8 = -1,
        int i9 = -1,
        int i10 = -1,
        int i11 = -1);

    static const QHash<QChar, int>& symbols();
};

#endif // Validations_H
