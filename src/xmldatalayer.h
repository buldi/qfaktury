/*
 * XmlDataLayer.h
 *
 *  Created on: Apr 18, 2009
 *      Author: moux
 */
#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef XMLDATALAYER_H_
#define XMLDATALAYER_H_

#include "idatalayer.h"

#include <QList>
#include <QMultiHash>
#include <QStringList>

#include "fwd.hpp"

// class for creating, updating and removing xml files with goods/services,
// invoices, warehouse documents and buyers informations
class XmlDataLayer final : public IDataLayer
{
public:
    XmlDataLayer();
    ~XmlDataLayer();

    // methods used for accessing customers
    BuyerData buyersSelectData(QString name, int type) override;
    QVector<BuyerData> buyersSelectAllData() override;
    bool buyersInsertData(BuyerData &buyerData, int type) override;
    bool buyersUpdateData(BuyerData &buyerData, int type, QString name) override;
    bool buyersDeleteData(QString name) override;
    QStringList buyersGetFirmList() override;

    ProductData productsSelectData(QString name, int type) override;
    QVector<ProductData> productsSelectAllData() override;
    bool productsInsertData(ProductData &prodData, int type) override;
    bool productsUpdateData(ProductData &prodData, int type, QString name) override;
    bool productsDeleteData(QString name) override;

    InvoiceData invoiceSelectData(QString name, int type, bool onlyCheck = false) override;
    WarehouseData warehouseSelectData(QString name, int type, bool onlyCheck = false) override;
    QVector<InvoiceData>
    invoiceSelectAllData(QDate start, QDate end, bool onlyCheck = false) override;
    QVector<WarehouseData> warehouseSelectAllData(QDate start, QDate end) override;
    bool invoiceInsertData(InvoiceData &invData, int type) override;
    bool warehouseInsertData(WarehouseData &invData, int type) override;
    bool invoiceUpdateData(InvoiceData &invData, int type, QString name) override;
    bool warehouseUpdateData(WarehouseData &invData, int type, QString name) override;
    bool invoiceDeleteData(QString name) override;
    bool warehouseDeleteData(QString name) override;

    QString const getRet() const override;
    QString getRetWarehouse() const override;
    void checkAllSymbInFiles() override;
    void checkAllSymbWareInFiles() override;
    QList<int> const getAllSymbols() override;
    QList<int> const getAllSymbolsWarehouse() override;
    bool ifThereOldDocuments(QString docname, QString docdir, QStringList filters) override;
    void separateOldDocuments(QString path) override;

    void saveInvoiceData() override;
    void readInvoiceData() override;

private:
    void buyersElemToData(BuyerData &o_buyerData, QDomElement const &i_element);
    void buyersDataToElem(BuyerData &i_buyerData, QDomElement &o_element);

    void productsElemToData(ProductData &o_prodData, QDomElement i_element);
    void productsDataToElem(ProductData &i_prodData, QDomElement &o_element);

    void invoiceSellerDataToElem(DocumentData &i_invData, QDomElement &o_element);
    void invoiceSellerElemToData(InvoiceData &o_invData, QDomElement i_element);
    void invoiceBuyerDataToElem(DocumentData &i_invData, QDomElement &o_element);
    void invoiceBuyerElemToData(InvoiceData &o_invData, QDomElement i_element);
    void
    invoiceProdDataToElem(const ProductData &i_prodData, QDomElement &o_element, int currentRow);
    void warehouseProdDataToElem(const ProductData &i_prodData, QDomElement &o_element, int type);
    void invoiceProdElemToData(InvoiceData &o_invData, QDomElement i_element);

    bool nameFilter(QString nameToCheck, QDate start, QDate end, QString docName, QString path);
    bool ifPersonNodeExists(QDomElement root);
    void addSectionPerson(bool checkedRoot);

    QString ret;
    QString retWarehouse;
    QList<int> allSymbols;
    QList<int> allSymbolsWarehouse;
    QStringList yearsList;
    QMultiHash<QString, QString> categorizedFiles;
};

#endif /* XMLDATALAYER_H_ */
