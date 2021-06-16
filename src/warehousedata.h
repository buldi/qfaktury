#if _pragma_once_support
#pragma once
#endif
#ifndef WAREHOUSEDATA_H
#define WAREHOUSEDATA_H

#include "documentdata.h"
#include "productdata.h"

#include <QDate>

// class for holding informations about warehouse documents. One instance -> one
// document
class WarehouseData final : public DocumentData
{
public:
    QString getCustomer() const override;

    /**
     *  Return invoice type
     */

    QString getInvoiceTypeAndSaveNr(int invoiceType);

    QString customer; // buyer
    QMap<int, ProductData> products;
    QDate liabDate;
    QDate sellingDate;
    QDate productDate;
    QDate endTransDate;
    QDate duplDate;
    QString sellerAddress;
    QString invNr;
    QString paymentType;
    QString additText;

    QString type;
    QString id; // in case of xml, here goes filename
    bool ifInvForDelNote; // for delivery note (WZ)

    QString custStreet;
    QString custTic;
    QString custCity;
    QString custName;
    QDate issueDate;
    int invoiceType; // 9 - WZ, 10 - RW

    // *START* for good issue note (RW)
    QString goodFromPlace;
    QString goodToPlace;
    QString departmentCost;
    QDate goodFromDate;
    QDate goodToDate;
    // *END*
};

#endif
