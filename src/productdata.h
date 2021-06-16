#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef PRODUCTDATA_H_
#define PRODUCTDATA_H_

#include <QMap>
#include <QString>

/* Class used in the Products list
 *
 */

// class for holding data about goods/services. One instance -> one type of
// service/good
class ProductData final
{
public:
    ProductData();

    ProductData(QString c1, QString c2, QString c3);

    ProductData(
        int inId,
        QString inName,
        QString inCode,
        QString inPkwiu,
        double inQuantity,
        QString inQuanType,
        double inDiscount,
        double inPrice,
        double inNett,
        int inVat,
        double inGross,
        QString inCurr,
        int reqAmount,
        int givAmount);

    //*************************************************** getters
    //***************************************************
    int getId() const;

    QString getName() const;

    QString getCode() const;

    QString getPkwiu() const;

    double getQuantity() const;

    QString getQuantityType() const;

    double getDiscount() const;

    double getPrice() const;

    double getNett() const;

    int getVat() const;

    double getGross() const;

    QString getCurr() const;

    //*************************************************** setters
    //***************************************************
    void setId(QString inId);

    void setName(QString inName);

    void setCode(QString inCode);

    void setPkwiu(QString inPkwiu);

    void setQuantity(QString quan);

    void setQuanType(QString qType);

    void setDiscount(QString disc);

    void setPrice(QString prc);

    void setNett(QString net);

    void setVat(QString vt);

    void setGross(QString gr);

    void setCurr(QString cr);

    void setRequiredAmount(QString reqAmount);

    void setGivedOutAmount(QString givAmount);

    // ********************************* All TO STRING ********************

    QString toString();

    int id;
    int vat;
    int lastProdId;
    QString name;
    QString code;
    QString pkwiu;
    QString quanType;
    QString curr;
    QString desc;
    QString type;
    double quantity;
    double discount;
    double price;
    double nett;
    double gross;
    QMap<int, double> prices;
    int requiredAmount;
    int givedOutAmount;
};

#endif
