/*
 * InvoiceData.h
 *
 *  Created on: Mar 12, 2009
 *      Author: moux
 */
#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef INVOICEDATA_H
#define INVOICEDATA_H

#include "custompaymdata.h"
#include "documentdata.h"
#include "productdata.h"

// class for holding data about invoices. One instance -> one invoice.
class InvoiceData final : public DocumentData
{
public:
    virtual ~InvoiceData() = default;

    QString getCustomer() const override;

    /**
     *  Return invoice type
     */
    QString getInvoiceTypeAndSaveNr(int invoiceType);

    int getInvoiceNameReturnType(QString invoiceType);


    QString customer; // buyer
    QMap<int, ProductData> products;
    QDate liabDate;
    QDate sellingDate;
    QDate productDate;
    QDate endTransDate;
    QDate duplDate;
    QString invNr;
    QString origInvNr;
    QString paymentType;
    QString currencyType;
    QString reason;
    QString additText;
    QString payment1;
    QDate date1;
    double amount1;

    QString payment2;
    QDate date2;
    double amount2;

    QString type;
    QString id; // in case of xml, here goes filename
    bool ifpVAT;
    bool ifInvForDelNote;

    QString custStreet;
    QString custTic;
    QString custCity;
    QString custName;
    QString sellerAddress;
    QString sellerName;
    QString sellerTic;
    QString sellerCity;

    int currencyTypeId;
    int discount;
    CustomPaymData custPaym;
    QDate issueDate;
    int invoiceType; // 1 - FVAT, 2 - FPro, 3 - corr, 4 - FBrutto

    QString jpkFieldText;
};

#endif
