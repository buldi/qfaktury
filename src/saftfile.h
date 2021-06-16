#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef SAFTFILE_H
#define SAFTFILE_H

#include "invoicedata.h"

#include <QWidget>

#include "fwd.hpp"

namespace Ui
{
class Saftfile;
}

class Saftfile : public QWidget
{
    Q_OBJECT

public:
    explicit Saftfile(QWidget *parent = nullptr);
    Saftfile(IDataLayer *dl, QWidget *parent = nullptr);
    ~Saftfile();

private slots:
    void initInvoicesRange();
    void prepareNextStep();

protected:
    QVector<InvoiceData> getInvFromRange();
    const QString getFromDateJPK();
    const QString getToDateJPK();
    const QString getApplicationPurpose();
    const QString getJpkFileArt();
    const QString getJpkFileArtWithVersion();
    const QString getDefaultCur();
    const QString getTaxOfficeNr();
    const QString getCorrectionNr();

private:
    IDataLayer *dlSaftfile;
    Ui::Saftfile *ui;
    QVector<InvoiceData> invs;
    QHash<QString, QString> data;
    QButtonGroup *groupAppPurp;
    QButtonGroup *groupArtFiles;

    bool toDateisLower();
    void putIntoTable(QVector<InvoiceData> invoices);
    void insertRowToTable(QTableWidget *t, int row);
    void putBtnToGroup();
    void showConnections();
    QVector<InvoiceData> removeUnusedInvoices(QVector<InvoiceData> inv);
    QVector<InvoiceData> addSAFTFieldsToList(QVector<InvoiceData> invoices);
};

#endif // SAFTFILE_H
