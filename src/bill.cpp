/*
 * Rachunek.cpp
 *
 *  Created on: Apr 2, 2009
 *      Author: moux
 */

#include "bill.h"
#include "const.h"
#include "settings.h"

#include "debug_message.h"

Bill::Bill(QWidget *parent, IDataLayer *dl, QString in_form)
    : Invoice(parent, dl, in_form)
{
    StrDebug();
}

Bill::~Bill()
{
    StrDebug();
}

// on start type of document is changed
void Bill::billInit()
{
    StrDebug();
    invoiceType = s_BILL;
}

// outputs type of document
QString Bill::getInvoiceTypeAndSaveNr()
{
    StrDebug();

    QString bill = "rachunek";
    sett().setValue("fvat", invNr->text());
    return bill;
}
