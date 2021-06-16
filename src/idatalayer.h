/*
 * IDataLayer.h
 *
 *  Created on: Apr 18, 2009
 *      Author: moux
 */
#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef IDATALAYER_H_
#define IDATALAYER_H_

#include <QList>
#include <QVector>

#include "fwd.hpp"

// class as bridge between XmlDataLayer class and others
class IDataLayer
{
public:
    virtual ~IDataLayer() = default;

    virtual void saveInvoiceData() = 0;

    virtual void readInvoiceData() = 0;

    virtual const QString getRet() const = 0;

    virtual QString getRetWarehouse() const = 0;

    virtual void checkAllSymbInFiles() = 0;

    virtual void checkAllSymbWareInFiles() = 0;

    virtual const QList<int> getAllSymbols() = 0;

    virtual const QList<int> getAllSymbolsWarehouse() = 0;

    virtual BuyerData buyersSelectData(QString, int) = 0;

    virtual QVector<BuyerData> buyersSelectAllData() = 0;

    virtual bool buyersInsertData(BuyerData &, int) = 0;

    virtual bool buyersUpdateData(BuyerData &, int, QString) = 0;

    virtual bool buyersDeleteData(QString) = 0;

    virtual QStringList buyersGetFirmList() = 0;

    virtual ProductData productsSelectData(QString, int) = 0;

    virtual QVector<ProductData> productsSelectAllData() = 0;

    virtual bool productsInsertData(ProductData &, int) = 0;

    virtual bool productsUpdateData(ProductData &, int, QString) = 0;

    virtual bool productsDeleteData(QString) = 0;

    virtual InvoiceData invoiceSelectData(QString, int, bool onlyCheck = false) = 0;

    virtual WarehouseData warehouseSelectData(QString, int, bool onlyCheck = false) = 0;

    virtual QVector<InvoiceData> invoiceSelectAllData(QDate, QDate, bool onlyCheck = false) = 0;

    virtual QVector<WarehouseData> warehouseSelectAllData(QDate, QDate) = 0;

    virtual bool invoiceInsertData(InvoiceData &, int) = 0;

    virtual bool warehouseInsertData(WarehouseData &, int) = 0;

    virtual bool invoiceUpdateData(InvoiceData &, int, QString) = 0;

    virtual bool warehouseUpdateData(WarehouseData &, int, QString) = 0;

    virtual bool invoiceDeleteData(QString) = 0;

    virtual bool warehouseDeleteData(QString) = 0;

    virtual bool ifThereOldDocuments(QString, QString, QStringList) = 0;

    virtual void separateOldDocuments(QString) = 0;
};

#endif /* IDATALAYER_H_ */
