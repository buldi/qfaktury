#include "invoicedata.h"

#include <QObject>

QString InvoiceData::getCustomer() const
{
    return customer;
}

QString InvoiceData::getInvoiceTypeAndSaveNr(int invoiceType)
{
    QString ret = "FVAT";

    switch (invoiceType)
    {
        case 1:
            ret = QObject::trUtf8("FVAT");
        break;
        case 2:
            ret = QObject::trUtf8("FPro");
        break;
        case 3:
            ret = QObject::trUtf8("korekta");
        break;
        case 4:
            ret = QObject::trUtf8("FBrutto");
        break;
        case 5:
            ret = QObject::trUtf8("kbrutto");
        break;
        case 6:
            ret = QObject::trUtf8("rachunek");
        break;
        case 7:
            ret = QObject::trUtf8("duplikat");
        break;
        case 8:
            ret = QObject::trUtf8("RR");
        break;
        case 9:
            ret = QObject::trUtf8("FVAT");
        break;
        case 10:
            ret = QObject::trUtf8("RW");
        break;
        default:
            ret = QObject::trUtf8("FVAT");
        break;
    }

    return ret;
}

int InvoiceData::getInvoiceNameReturnType(QString invoiceType)
{
    if (invoiceType == QObject::trUtf8("FVAT"))
        return 1;
    else if (invoiceType == QObject::trUtf8("FPro"))
        return 2;
    else if (invoiceType == QObject::trUtf8("korekta"))
        return 3;
    else if (invoiceType == QObject::trUtf8("FBrutto"))
        return 4;
    else if (invoiceType == QObject::trUtf8("kbrutto"))
        return 5;
    else if (invoiceType == QObject::trUtf8("rachunek"))
        return 6;
    else if (invoiceType == QObject::trUtf8("duplikat"))
        return 7;
    else if (invoiceType == QObject::trUtf8("RR"))
        return 8;
    else if (invoiceType == QObject::trUtf8("FVAT"))
        return 9;
    else if (invoiceType == QObject::trUtf8("RW"))
        return 10;
    else
        return 1;

    return 1;
}
