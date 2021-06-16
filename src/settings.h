#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#include "fwd.hpp"

// class for settings and holding directory paths
// Singleton class to store all the settings and static values
class Settings final : public QSettings
{
    Q_DISABLE_COPY(Settings)

public:
#ifdef Q_OS_MAC
    QString appPath = "~/Library/Application Support/qfaktury";
#endif

#ifdef Q_OS_LINUX
    QString appPath = "/usr/share/qfaktury";
#endif
    // Probably
    // QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(1)
    // returns different values on different distributions, for me that was
    // /usr/share/<APPNAME>

#ifdef Q_OS_WIN32
    QString appPath = QDir::homePath() + "/AppData/Roaming/qfaktury";
#endif

    QString getAppDirs();

    // get date from settings as QDate
    QDate getValueAsDate(QString val);

    // returns date format used for all dates
    QString getDateFormat();

    const QString getPlansDir();

    // returns date format used while saving the file
    QString getFnameDateFormat();

    // returns a translator
    QTranslator *getTranslation();

    /**
     * validate the settings and set them to default values if required.
     */

    void checkSettings();

    /** Reset all settings to default values
     */

    void resetSettings();

    // Compiles version with appName ready for display
    QString getVersion(QString appName);

    // returns working directory
    QString getWorkingDir();

    const QString getCSVDir();

    const QString getJPKDir();

    QString getStyle();

    // returns templates directory
    QString getTemplate();

    QString getEmergTemplate();

    QString getPdfDir();

    // return invoices dir
    QString getDataDir();

    QString getWarehouseDir();

    // return invoices dir
    QString getInvoicesDir();

    // return warehouse dir
    QString getWarehouseFullDir();

    // return gus dir
    QString getGUSDir();

    // return customers xml
    QString getCustomersXml();

    // return customers xml
    QString getProductsXml();

    // returns invoice doc name stored as a DOCTYPE
    QString getInoiveDocName();

    // returns warehouse doc name stored as a DOCTYPE
    QString getWarehouseDocName();

    // returns correction doc name stored as a DOCTYPE
    QString getCorrDocName();

    // returns customers doc name stored as a DOCTYPE
    QString getCustomersDocName();

    // returns products doc name stored as a DOCTYPE
    QString getProdutcsDocName();

    // @TODO enforce that translation won't affect this funcionality
    // converts customer type into int value
    int getCustomerType(QString custType);

    // converts product type into int value
    int getProductType(QString prodName);

    QString getCompanyName();

    QString getNaturalPerson();

    QString getOfficeName();

    QString getCompanyNameTr();

    QString getOfficeNameTr();

    QString getProductName();

    QString getServiceName();

    // Adds Data to input string
    QString getNameWithData(QString in);

    QByteArray getCodecName();

    QString getDecimalPointStr();

    QString getTPointStr();

    QString numberToString(double i, char f = 'f', int prec = 2);

    QString numberToString(int i);

    double stringToDouble(QString s);

private:
    QString dateFormat;
    QString fileNameDateFormat;
    QLocale *locale;
    QTranslator *translator;

    Settings();

    friend Settings &sett();
};

Settings &sett();

#endif
