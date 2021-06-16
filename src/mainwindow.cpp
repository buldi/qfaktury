#include "backup.h"
#include "bill.h"
#include "buyers.h"
#include "config.h"
#include "const.h"
#include "correctgross.h"
#include "csvexport.h"
#include "datewidgetitem.h"
#include "deliverynote.h"
#include "duplicate.h"
#include "goods.h"
#include "goodsissuednotes.h"
#include "invoice.h"
#include "invoicegross.h"
#include "invoicerr.h"
#include "mainwindow.h"
#include "organizer.h"
#include "owncalendar.h"
#include "runguard.h"
#include "saftfile.h"
#include "send.h"
#include "setting.h"
#include "user.h"
#include "warehousedata.h"
#include "xmldatalayer.h"

#include "debug_message.h"

#if QUAZIP_FOUND
#include "quazip5/JlCompress.h"
#include "quazip5/quazipdir.h"
#endif

#include <QDateEdit>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPdfWriter>
#include <QPointer>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProcess>
#include <QTextStream>
#include <QTimer>

#include <QDebug>

#include <memory>

MainWindow *MainWindow::m_instance = nullptr;
bool MainWindow::shouldHidden = false;

/** Constructor
 */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    StrDebug();

    ui->setupUi(this);
    m_instance = this;
    init();
}

MainWindow *MainWindow::instance()
{
    StrDebug();

    return m_instance;
}

/** Destructor
 */

MainWindow::~MainWindow()
{
    StrDebug();

    saveAllSett();

    if (nullptr != backupTimerOften)
    {
        backupTimerOften = nullptr;
    }

    delete backupTimerOften;

    if (nullptr != dl)
        dl = nullptr;
    delete dl;

    if (nullptr != ui)
        ui = nullptr;
    delete ui;

    m_instance = nullptr;
}

/**
 * init() method
 */

void MainWindow::init()
{
    StrDebug();

    // first run
    if (firstRun())
    {
        // towary/uslugi - wymiary
        ui->tableT->setColumnWidth(0, 50);
        ui->tableT->setColumnWidth(1, 140);
        ui->tableT->setColumnWidth(3, 50);
        ui->tableT->setColumnWidth(4, 70);
        ui->tableT->setColumnWidth(5, 70);
        ui->tableT->setColumnWidth(6, 65); // netto1
        ui->tableT->setColumnWidth(7, 65); // netto2
        ui->tableT->setColumnWidth(8, 65); // netto3
        ui->tableT->setColumnWidth(9, 65); // netto4
        ui->tableT->setColumnWidth(10, 65);

        saveAllSettAsDefault();

        QMessageBox::information(
            this,
            "QFaktury",
            trUtf8("Program zawiera konwersję walut w oknie edycji faktury lub "
                   "podczas jej tworzenia. By jej używać, powinieneś mieć dostęp "
                   "do internetu oraz poprawnie ustawiony czas systemowy."));
        QMessageBox::information(
            this,
            "QFaktury",
            trUtf8("W przypadku zmiany lokalizacji systemu, sposób formatowania "
                   "liczb może się zmienić. Efekt ten może być widoczny po "
                   "restarcie programu."));

        if (QMessageBox::information(
                this,
                "QFaktury",
                trUtf8("Czy chcesz skonfigurować firmę? Opcja ta "
                       "przy starcie programu będzie widoczna "
                       "tylko przy pierwszym uruchomieniu."),
                trUtf8("Tak"),
                trUtf8("Nie"),
                nullptr,
                0,
                1)
            == 0)
        {
            userDataClick();
        }
    }
    else
    {
        ui->filtrStart->setDisplayFormat(sett().getDateFormat());
        ui->filtrStart->setDate(sett().getValueAsDate("filtrStart"));
        ui->filtrEnd->setDisplayFormat(sett().getDateFormat());
        ui->filtrEnd->setDate(sett().getValueAsDate("filtrEnd"));

        ui->warehouseFromDate->setDisplayFormat(sett().getDateFormat());
        ui->warehouseFromDate->setDate(sett().getValueAsDate("filtrStartWarehouse"));
        ui->warehouseToDate->setDisplayFormat(sett().getDateFormat());
        ui->warehouseToDate->setDate(sett().getValueAsDate("filtrEndWarehouse"));
    }

    setupDir();

    if (!ifEmergTemplateExists())
        createEmergTemplate();

    if (ui->tableH->rowCount() != 0)
    {
        ui->invoiceEdAction->setEnabled(true);
        ui->invoiceDelAction->setEnabled(true);

        // if (ui->tableK->rowCount() != 0)
        // ui->sendEmailAction->setEnabled(true);
        // else
        //  ui->sendEmailAction->setDisabled(true);
    }
    else
    {
        ui->invoiceEdAction->setDisabled(true);
        ui->invoiceDelAction->setDisabled(true);
        // ui->sendEmailAction->setDisabled(true);
    }

    if (ui->tableK->rowCount() != 0)
    {
        ui->editBuyersAction->setEnabled(true);
        ui->delBuyersAction->setEnabled(true);
        ui->actionPrintBuyer->setEnabled(true);

        // if (ui->tableH->rowCount() != 0) {
        //   ui->sendEmailAction->setEnabled(true);
        // } else {
        //   ui->sendEmailAction->setDisabled(true);
        //}
    }
    else
    {
        ui->editBuyersAction->setDisabled(true);
        ui->delBuyersAction->setDisabled(true);
        ui->actionPrintBuyer->setDisabled(true);
        // ui->sendEmailAction->setDisabled(true);
    }

    if (ui->tableT->rowCount() != 0)
    {
        ui->editGoodsAction->setEnabled(true);
        ui->delGoodsAction->setEnabled(true);
    }
    else
    {
        ui->editGoodsAction->setDisabled(true);
        ui->delGoodsAction->setDisabled(true);
    }

    // choose data access mode
    dl = new XmlDataLayer();

    // towary/uslugi -
    ui->tableT->setColumnWidth(0, sett().value("towCol0", QVariant(50)).toInt());
    ui->tableT->setColumnWidth(1, sett().value("towCol1", QVariant(140)).toInt());
    ui->tableT->setColumnWidth(3, sett().value("towCol2", QVariant(50)).toInt());
    ui->tableT->setColumnWidth(4, sett().value("towCol3", QVariant(70)).toInt());
    ui->tableT->setColumnWidth(5, sett().value("towCol4", QVariant(70)).toInt());
    ui->tableT->setColumnWidth(6, sett().value("towCol5", QVariant(65)).toInt()); // net1
    ui->tableT->setColumnWidth(7, sett().value("towCol6", QVariant(65)).toInt()); // net2
    ui->tableT->setColumnWidth(8, sett().value("towCol7", QVariant(65)).toInt()); // net3
    ui->tableT->setColumnWidth(9, sett().value("towCol8", QVariant(65)).toInt()); // net4
    ui->tableT->setColumnWidth(10, sett().value("towCol9", QVariant(65)).toInt());

    ui->tableH->setColumnWidth(0, sett().value("histCol0", QVariant(0)).toInt());
    ui->tableH->setColumnWidth(1, sett().value("histCol1", QVariant(120)).toInt());
    ui->tableH->setColumnWidth(3, sett().value("histCol2", QVariant(120)).toInt());
    ui->tableH->setColumnWidth(4, sett().value("histCol3", QVariant(140)).toInt());
    ui->tableH->setColumnWidth(5, sett().value("histCol4", QVariant(120)).toInt());

    ui->tableM->setColumnWidth(0, sett().value("wareCol0", QVariant(0)).toInt());
    ui->tableM->setColumnWidth(1, sett().value("wareCol1", QVariant(120)).toInt());
    ui->tableM->setColumnWidth(3, sett().value("wareCol2", QVariant(120)).toInt());
    ui->tableM->setColumnWidth(4, sett().value("wareCol3", QVariant(140)).toInt());
    ui->tableM->setColumnWidth(5, sett().value("wareCol4", QVariant(120)).toInt());

    ui->tableK->setColumnWidth(0, sett().value("custCol0", QVariant(140)).toInt());
    ui->tableK->setColumnWidth(1, sett().value("custCol1", QVariant(140)).toInt());
    ui->tableK->setColumnWidth(2, sett().value("custCol2", QVariant(100)).toInt());
    ui->tableK->setColumnWidth(3, sett().value("custCol3", QVariant(120)).toInt());
    ui->tableK->setColumnWidth(4, sett().value("custCol4", QVariant(140)).toInt());
    ui->tableK->setColumnWidth(5, sett().value("custCol5", QVariant(170)).toInt());
    ui->tableK->setColumnWidth(6, sett().value("custCol6", QVariant(170)).toInt());

    // add Icon
    QIcon icon;
    icon.addPixmap(QPixmap(":/res/icons/qfaktury_48.png"), QIcon::Normal, QIcon::Off);

    this->setWindowIcon(icon);
    this->setWindowTitle(sett().getVersion(qAppName()));

    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(tabChanged()));
    timer->start(1000);

    qDebug() << "application path: "
             << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(1);

    calendar = new ownCalendarWidget;
    ui->calendarLayout->addWidget(calendar);

    if (!QUAZIP_FOUND)
    {
        ui->actionCreateBackup->deleteLater();
        ui->actionLoadBackup->deleteLater();
    }

    // connect slots
    connect(ui->actionBug, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/juliagoda/qfaktury/issues"));
    });

    connect(ui->applyFiltrBtn, SIGNAL(clicked(bool)), this, SLOT(rereadHist(bool)));
    connect(ui->findWarehouses, SIGNAL(clicked(bool)), this, SLOT(rereadWarehouses(bool)));
    connect(ui->fileData_companyAction, SIGNAL(triggered()), this, SLOT(userDataClick()));
    connect(ui->fileEndAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->addBuyersAction, SIGNAL(triggered()), this, SLOT(buyerClick()));
    connect(ui->delBuyersAction, SIGNAL(triggered()), this, SLOT(buyerDel()));
    connect(ui->editBuyersAction, SIGNAL(triggered()), this, SLOT(buyerEd()));
    connect(ui->addInvoiceAction, SIGNAL(triggered()), this, SLOT(newInv()));
    connect(ui->invoiceDelAction, SIGNAL(triggered()), this, SLOT(delFHist()));
    connect(ui->warehouseDelAction, SIGNAL(triggered()), this, SLOT(delMHist()));
    connect(ui->invoiceEdAction, SIGNAL(triggered()), this, SLOT(editFHist()));
    connect(ui->warehouseEdAction, SIGNAL(triggered()), this, SLOT(warehouseEdit()));
    connect(ui->invoiceDuplAction, SIGNAL(triggered()), this, SLOT(newDuplicate()));
    connect(ui->invoiceGrossAction, SIGNAL(triggered()), this, SLOT(newInvGross()));
    connect(ui->invoiceBillAction, SIGNAL(triggered()), this, SLOT(newInvBill()));
    connect(ui->invoiceCorrAction, SIGNAL(triggered()), this, SLOT(newCor()));
    connect(ui->invoiceRRAction, SIGNAL(triggered()), this, SLOT(newInvRR()));
    connect(ui->invoiceProFormaAction, SIGNAL(triggered()), this, SLOT(newPForm()));
    connect(ui->addGoodsAction, SIGNAL(triggered()), this, SLOT(goodsAdd()));
    connect(ui->actionPrintBuyer, SIGNAL(triggered()), this, SLOT(printBuyerList()));
    connect(ui->editGoodsAction, SIGNAL(triggered()), this, SLOT(goodsEdit()));
    connect(ui->delGoodsAction, SIGNAL(triggered()), this, SLOT(goodsDel()));
    connect(ui->findInvoiceAction, SIGNAL(triggered()), this, SLOT(findInvoicePdf()));
    connect(ui->findJPKAction, SIGNAL(triggered()), this, SLOT(openJPKDirectory()));
    connect(
        ui->filtrEnd,
        SIGNAL(dateChanged(const QDate &)),
        this,
        SLOT(checkDateRange(const QDate &)));
    connect(
        ui->warehouseToDate,
        SIGNAL(dateChanged(const QDate &)),
        this,
        SLOT(checkDateRange(const QDate &)));

    /** Slot used to display aboutQt informations.
     */

    connect(ui->action_Qt, &QAction::triggered, [this]() {
        QMessageBox::aboutQt(this, sett().getVersion(qAppName()));
    });

    connect(ui->helpAbout_appAction, SIGNAL(triggered()), this, SLOT(aboutProg()));
    connect(ui->fileSettingsAction, SIGNAL(triggered()), this, SLOT(settClick()));

    /** Slot help
     */

    connect(ui->helpAction, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/juliagoda/qfaktury"));
    });

    connect(ui->generateJPKbtn, SIGNAL(clicked(bool)), this, SLOT(openJPKGenerator()));
    connect(ui->fileExportCSVAction, SIGNAL(triggered()), this, SLOT(exportCsv()));
#if QUAZIP_FOUND
    qDebug() << "QUAZIP_FOUND -> It's possible to run connections for Backup";
    connect(ui->actionCreateBackup, SIGNAL(triggered()), this, SLOT(createBackupMode()));
    connect(ui->actionLoadBackup, SIGNAL(triggered()), this, SLOT(loadBackupMode()));
    checkIntervalsForBackup();
#endif
    connect(ui->hideOrganizer, SIGNAL(clicked(bool)), this, SLOT(openHideOrganizer()));
    connect(calendar, SIGNAL(activated(const QDate &)), this, SLOT(noteDownTask(const QDate &)));
    connect(ui->tableH, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(editFHist()));
    connect(
        ui->tableH, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTableMenuH(QPoint)));
    connect(ui->tableK, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(buyerEd()));
    connect(
        ui->tableK, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTableMenuK(QPoint)));
    connect(
        ui->tableM, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTableMenuM(QPoint)));
    connect(ui->tableM, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(warehouseEdit()));
    connect(ui->tableT, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(goodsEdit()));
    connect(
        ui->tableT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTableMenuT(QPoint)));
    connect(ui->sendEmailAction, SIGNAL(triggered()), this, SLOT(sendEmailToBuyer()));

    connect(
        ui->tableH,
        SIGNAL(itemClicked(QTableWidgetItem *)),
        this,
        SLOT(mainUpdateStatus(QTableWidgetItem *)));
    connect(
        ui->tableK,
        SIGNAL(itemClicked(QTableWidgetItem *)),
        this,
        SLOT(mainUpdateStatus(QTableWidgetItem *)));
    connect(
        ui->tableT,
        SIGNAL(itemClicked(QTableWidgetItem *)),
        this,
        SLOT(mainUpdateStatus(QTableWidgetItem *)));
    connect(
        ui->tableM,
        SIGNAL(itemClicked(QTableWidgetItem *)),
        this,
        SLOT(mainUpdateStatus(QTableWidgetItem *)));
    connect(ui->tableK, SIGNAL(cellClicked(int, int)), this, SLOT(openWebTableK(int, int)));

    readBuyer();
    readHist();
    readWarehouses();
    readGoods();
    categorizeYears();

    if (!ifpdfDirExists())
    {
        createPdfDir();
    }
    else
    {
        QDir _dir(sett().getPdfDir());
        _dir.setFilter(QDir::Files);
        QStringList filters;
        filters << "*.pdf";

        _dir.setNameFilters(filters);
        QStringList files = _dir.entryList();

        if (files.isEmpty())
            generatePdfFromList();
    }
}

#if QUAZIP_FOUND
void MainWindow::checkIntervalsForBackup()
{
    QSettings settings("elinux", "qfaktury");

    settings.beginGroup("backup_settings");
    if (settings.value("regular_backup").toBool())
    {
        if (!settings.value("backup_interval").toString().isEmpty()
            && !settings.value("backup_interval").toString().isNull())
        {
            backupTimerOften = new QTimer(this);
            QString num = settings.value("backup_interval").toString();
            num.chop(1);

            if (settings.value("backup_interval").toString().back() == 'h')
            {
                backupTimerOften->setInterval(
                    num.toInt() * 60 * 60 * 1000); // checks regularly after chosen hours
                backupTimerOften->setSingleShot(false);
            }
            else if (settings.value("backup_interval").toString().back() == 'm')
            {
                backupTimerOften->setInterval(
                    num.toInt() * 60 * 1000); // checks regularly after chosen minutes
                backupTimerOften->setSingleShot(false);
            }
            else
            {
                backupTimerOften->setInterval(
                    60000); // checks once after 1 minute for days, weeks or months
                backupTimerOften->setSingleShot(true);
            }

            connect(backupTimerOften, SIGNAL(timeout()), this, SLOT(intervalBackup()));
            backupTimerOften->start();
        }
    }
    settings.endGroup();
}
#endif

bool MainWindow::ifpdfDirExists()
{
    StrDebug();

    QDir mainPath(sett().getPdfDir());

    if (!mainPath.exists())
    {
        return false;
    }
    else
    {
        return true;
    }
}

void MainWindow::createPdfDir()
{
    StrDebug();

    QDir mainPath;
    mainPath.mkpath(sett().getPdfDir());
    generatePdfFromList();
}

void MainWindow::generatePdfFromList()
{
    StrDebug();

    shouldHidden = true;

    for (int i = 0; i < ui->tableH->rowCount(); i++)
    {
        ui->tableH->setCurrentCell(i, 0);
        editFHist();
    }

    for (int i = 0; i < ui->tableM->rowCount(); i++)
    {
        ui->tableM->setCurrentCell(i, 0);
        warehouseEdit();
    }

    shouldHidden = false;
}

bool MainWindow::ifEmergTemplateExists()
{
    StrDebug();

    QFileInfo fileInfo(sett().getEmergTemplate());

    if (fileInfo.exists() && fileInfo.isFile())
        return true;
    else
        return false;
}

void MainWindow::createEmergTemplate()
{
    StrDebug();

    QDir mainPath(QDir::homePath() + "/.local/share/data/elinux/template");

    if (!mainPath.exists())
    {
        mainPath.mkpath(QDir::homePath() + "/.local/share/data/elinux/template");
    }

    QFile file(sett().getEmergTemplate());

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);

        QStringList::const_iterator constIterator;
        for (constIterator = blackEmergTemplate.constBegin();
             constIterator != blackEmergTemplate.constEnd();
             ++constIterator)
            stream << (*constIterator).toLocal8Bit().constData() << endl;

        file.close();
    }
}

/**
 * firstRun setup()
 */

bool MainWindow::firstRun()
{
    StrDebug();

    bool ok = sett().value("firstrun", true).toBool();

    ui->filtrStart->setDate(QDate(QDate::currentDate().year(), 1, 1));
    ui->filtrEnd->setDate(QDate(QDate::currentDate().year(), 12, 31));

    ui->warehouseFromDate->setDate(QDate(QDate::currentDate().year(), 1, 1));
    ui->warehouseToDate->setDate(QDate(QDate::currentDate().year(), 12, 31));

    sett().checkSettings();

    return ok;
}

/**
 * categorizing files according to the years and putting them to separated
 * folders
 */

void MainWindow::categorizeYears()
{
    StrDebug();

    if (dl->ifThereOldDocuments(
            sett().getInoiveDocName(),
            sett().getInvoicesDir(),
            QStringList("h*.xml"
                        "k*.xml")))
        dl->separateOldDocuments(sett().getInvoicesDir());

    if (dl->ifThereOldDocuments(
            sett().getWarehouseDocName(), sett().getWarehouseFullDir(), QStringList("m*.xml")))
        dl->separateOldDocuments(sett().getWarehouseFullDir());
}

void MainWindow::openWebTableK(int row, int column)
{
    StrDebug();

    if (column == 6)
    {
        if ((!ui->tableK->item(row, column)->text().isEmpty())
            && (ui->tableK->item(row, column)->text() != "-"))
        {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            QDesktopServices::openUrl(
                QUrl(ui->tableK->item(row, column)->text(), QUrl::TolerantMode));
            QApplication::restoreOverrideCursor();
        }
    }
}

/** save sett() before quit
 * save column width
 */

void MainWindow::saveColumnWidth()
{
    StrDebug();

    // width of the columns in the towary "goods" tab
    sett().setValue("towCol0", ui->tableT->columnWidth(0));
    sett().setValue("towCol1", ui->tableT->columnWidth(1));
    sett().setValue("towCol2", ui->tableT->columnWidth(2));
    sett().setValue("towCol3", ui->tableT->columnWidth(3));
    sett().setValue("towCol4", ui->tableT->columnWidth(4));
    sett().setValue("towCol5", ui->tableT->columnWidth(5));
    sett().setValue("towCol6", ui->tableT->columnWidth(6));
    sett().setValue("towCol7", ui->tableT->columnWidth(7));
    sett().setValue("towCol8", ui->tableT->columnWidth(8));
    sett().setValue("towCol9", ui->tableT->columnWidth(9));
    sett().setValue("towCol10", ui->tableT->columnWidth(10));

    // width of the columns in the history tab
    sett().setValue("histCol0", ui->tableH->columnWidth(0));
    sett().setValue("histCol1", ui->tableH->columnWidth(1));
    sett().setValue("histCol2", ui->tableH->columnWidth(2));
    sett().setValue("histCol3", ui->tableH->columnWidth(3));
    sett().setValue("histCol4", ui->tableH->columnWidth(4));
    sett().setValue("histCol5", ui->tableH->columnWidth(5));

    // width of the columns in the customers tab
    sett().setValue("custCol0", ui->tableK->columnWidth(0));
    sett().setValue("custCol1", ui->tableK->columnWidth(1));
    sett().setValue("custCol2", ui->tableK->columnWidth(2));
    sett().setValue("custCol3", ui->tableK->columnWidth(3));
    sett().setValue("custCol4", ui->tableK->columnWidth(4));
    sett().setValue("custCol5", ui->tableK->columnWidth(5));
    sett().setValue("custCol6", ui->tableK->columnWidth(6));

    sett().setValue("wareCol0", ui->tableM->columnWidth(0));
    sett().setValue("wareCol1", ui->tableM->columnWidth(1));
    sett().setValue("wareCol2", ui->tableM->columnWidth(2));
    sett().setValue("wareCol3", ui->tableM->columnWidth(3));
    sett().setValue("wareCol4", ui->tableM->columnWidth(4));
    sett().setValue("wareCol5", ui->tableM->columnWidth(5));
}

/** Saves all sett() as default - first run
 */

void MainWindow::saveAllSettAsDefault()
{
    StrDebug();
    sett().resetSettings();
}

/** Saves all sett()
 */

void MainWindow::saveAllSett()
{
    StrDebug();

    // saves filtr
    sett().setValue("filtrStart", ui->filtrStart->text());
    sett().setValue("filtrEnd", ui->filtrEnd->text());

    sett().setValue("filtrStartWarehouse", ui->warehouseFromDate->text());
    sett().setValue("filtrEndWarehouse", ui->warehouseToDate->text());

    saveColumnWidth();

    // saves unsaved
    sett().sync();
}

/** Clears content of the QTableWidget passed in the input
 *  @param QTableWidget
 */

void MainWindow::tableClear(QTableWidget *tab)
{
    StrDebug();
    tab->setRowCount(0);
}

/** Used while adding new row
 */

void MainWindow::insertRow(QTableWidget *t, int row)
{
    StrDebug();

    t->insertRow(row);

    for (int i = 0; i < t->columnCount(); i++)
    {
        t->setItem(row, i, new QTableWidgetItem());
    }
}

int MainWindow::getMaxSymbol() const
{
    StrDebug();

    int max = *std::max_element(allSymbols.begin(), allSymbols.end());

    return max;
}

int MainWindow::getMaxSymbolWarehouse() const
{
    StrDebug();

    int max = *std::max_element(allSymbolsWarehouse.begin(), allSymbolsWarehouse.end());
    return max;
}

/** Reads the invoices from the directory passed in the input.
 *  @param QString - directory from where the invoices should be read
 */

void MainWindow::readHist()
{
    StrDebug();

    QVector<InvoiceData> invoicesVec;
    invoicesVec = dl->invoiceSelectAllData(ui->filtrStart->date(), ui->filtrEnd->date());
    allSymbols = dl->getAllSymbols();
    ui->tableH->setSortingEnabled(false);

    for (int i = 0; i < invoicesVec.size(); ++i)
    {
        insertRow(ui->tableH, ui->tableH->rowCount());
        QString text = invoicesVec.at(i).id;
        ui->tableH->item(ui->tableH->rowCount() - 1, 0)->setText(text);
        qDebug("Added ID of file");
        text = invoicesVec.at(i).invNr;
        ui->tableH->item(ui->tableH->rowCount() - 1, 1)->setText(text);
        qDebug("Added file name");
        ui->tableH->setItem(
            ui->tableH->rowCount() - 1, 2, new DateWidgetItem(invoicesVec.at(i).sellingDate));
        qDebug("Added file date");
        text = invoicesVec.at(i).type;
        ui->tableH->item(ui->tableH->rowCount() - 1, 3)->setText(text);
        qDebug("Added file type");
        text = invoicesVec.at(i).custName;
        ui->tableH->item(ui->tableH->rowCount() - 1, 4)->setText(text);
        qDebug("Added buyer's name");
        text = invoicesVec.at(i).custTic;
        ui->tableH->item(ui->tableH->rowCount() - 1, 5)->setText(text);
        qDebug("Added file NIP");
    }

    ui->tableH->setSortingEnabled(true);
}

/** Reads the invoices from the directory passed in the input.
 *  @param QString - directory from where the invoices should be read
 */

void MainWindow::readWarehouses()
{
    StrDebug();

    QVector<WarehouseData> wareVec;
    wareVec =
        dl->warehouseSelectAllData(ui->warehouseFromDate->date(), ui->warehouseToDate->date());
    allSymbolsWarehouse = dl->getAllSymbolsWarehouse();
    ui->tableM->setSortingEnabled(false);

    for (int i = 0; i < wareVec.size(); ++i)
    {
        insertRow(ui->tableM, ui->tableM->rowCount());
        QString text = wareVec.at(i).id;
        ui->tableM->item(ui->tableM->rowCount() - 1, 0)->setText(text);
        qDebug("Added ID of file");
        text = wareVec.at(i).invNr;
        ui->tableM->item(ui->tableM->rowCount() - 1, 1)->setText(text);
        qDebug("Added file name");
        ui->tableM->setItem(
            ui->tableM->rowCount() - 1, 2, new DateWidgetItem(wareVec.at(i).sellingDate));
        qDebug("Added file date");
        text = wareVec.at(i).type;
        ui->tableM->item(ui->tableM->rowCount() - 1, 3)->setText(text);
        qDebug("Added file type");
        text = wareVec.at(i).custName;
        ui->tableM->item(ui->tableM->rowCount() - 1, 4)->setText(text);
        qDebug("Added buyer's name");
        text = wareVec.at(i).custTic;
        ui->tableM->item(ui->tableM->rowCount() - 1, 5)->setText(text);
        qDebug("Added file NIP");
    }

    ui->tableM->setSortingEnabled(true);
}

/** Reads customers from the XML
 */

void MainWindow::readBuyer()
{
    StrDebug();

    tableClear(ui->tableK);
    ui->tableK->setSortingEnabled(false);
    QVector<BuyerData> buyerVec = dl->buyersSelectAllData();

    for (int i = 0; i < buyerVec.size(); ++i)
    {
        qDebug() << "liczba kolumn: " << ui->tableK->columnCount();
        insertRow(ui->tableK, ui->tableK->rowCount());
        QString text = buyerVec.at(i).name;
        ui->tableK->item(ui->tableK->rowCount() - 1, 0)->setText(text);
        text = buyerVec.at(i).type;
        ui->tableK->item(ui->tableK->rowCount() - 1, 1)->setText(text);
        text = buyerVec.at(i).place;
        ui->tableK->item(ui->tableK->rowCount() - 1, 2)->setText(text);
        text = buyerVec.at(i).address;
        ui->tableK->item(ui->tableK->rowCount() - 1, 3)->setText(text);
        text = buyerVec.at(i).phone;
        ui->tableK->item(ui->tableK->rowCount() - 1, 4)->setText(text);
        text = buyerVec.at(i).email;
        qDebug() << "liczba kolumn: " << ui->tableK->columnCount();
        ui->tableK->item(ui->tableK->rowCount() - 1, 5)->setText(text);
        text = buyerVec.at(i).www;
        ui->tableK->item(ui->tableK->rowCount() - 1, 6)->setText(text);
        ui->tableK->item(ui->tableK->rowCount() - 1, 6)->setTextColor(QColor(30, 144, 255));
    }

    ui->tableK->setSortingEnabled(true);
}

/** Reads goods from the XML
 */

void MainWindow::readGoods()
{
    StrDebug();

    tableClear(ui->tableT);
    QVector<ProductData> prodVec = dl->productsSelectAllData();

    for (int i = 0; i < prodVec.size(); ++i)
    {
        insertRow(ui->tableT, ui->tableT->rowCount());
        QString text = QString::number(prodVec.at(i).id);
        ui->tableT->item(ui->tableT->rowCount() - 1, 0)->setText(text);
        text = prodVec.at(i).name;
        ui->tableT->item(ui->tableT->rowCount() - 1, 1)->setText(text);
        text = prodVec.at(i).desc;
        ui->tableT->item(ui->tableT->rowCount() - 1, 2)->setText(text);
        text = prodVec.at(i).code;
        ui->tableT->item(ui->tableT->rowCount() - 1, 3)->setText(text);
        text = prodVec.at(i).pkwiu;
        ui->tableT->item(ui->tableT->rowCount() - 1, 4)->setText(text);
        text = prodVec.at(i).type;
        ui->tableT->item(ui->tableT->rowCount() - 1, 5)->setText(text);
        text = prodVec.at(i).quanType;
        ui->tableT->item(ui->tableT->rowCount() - 1, 6)->setText(text);
        text = sett().numberToString(prodVec.at(i).prices[0]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 7)->setText(text);
        text = sett().numberToString(prodVec.at(i).prices[0]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 8)->setText(text);
        text = sett().numberToString(prodVec.at(i).prices[0]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 9)->setText(text);
        text = sett().numberToString(prodVec.at(i).prices[0]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 10)->setText(text);
        text = QString::number(prodVec.at(i).vat);
        ui->tableT->item(ui->tableT->rowCount() - 1, 11)->setText(text);
    }

    ui->tableT->setSortingEnabled(true);
}

/** Creates directories if required
 */

void MainWindow::setupDir()
{
    StrDebug();

    workingDir = sett().getWorkingDir();
    QDir dir(workingDir);
    QDir mainPath(QDir::homePath() + "/.local/share/data");

    if (!mainPath.exists())
    {
        mainPath.mkpath(QDir::homePath() + "/.local/share/data");
    }

    if (!dir.exists())
    {
        dir.mkdir(workingDir);
        dir.mkdir(workingDir + sett().getDataDir());
        dir.mkdir(workingDir + sett().getWarehouseDir());
    }

    if (!dir.exists(workingDir + sett().getDataDir()))
    {
        dir.mkdir(workingDir + sett().getDataDir());
    }

    if (!dir.exists(workingDir + sett().getWarehouseDir()))
    {
        dir.mkdir(workingDir + sett().getWarehouseDir());
    }
}

// ----------------------------------------  SLOTS
// ---------------------------------//

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    StrDebug();

    if (event->key() == Qt::Key_F5)
    {
        newInv();
    }

    if (event->key() == Qt::Key_Return)
    {
        switch (ui->tabWidget2->currentIndex())
        {
        case 0: {
            editFHist();
            break;
        }
        case 1: {
            buyerEd();
            break;
        }
        case 2: {
            goodsEdit();
            break;
        }
        }
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_PageUp)
    {
        if (ui->tabWidget2->currentIndex() != ui->tabWidget2->count() - 1)
        {
            ui->tabWidget2->setCurrentIndex(ui->tabWidget2->currentIndex() + 1);
        }
        else
        {
            ui->tabWidget2->setCurrentIndex(0);
        }
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_PageDown)
    {
        if (ui->tabWidget2->currentIndex() != 0)
        {
            ui->tabWidget2->setCurrentIndex(ui->tabWidget2->currentIndex() - 1);
        }
        else
        {
            ui->tabWidget2->setCurrentIndex(ui->tabWidget2->count() - 1);
        }
    }
}

/** Slot
 *  Shows context menu
 */

void MainWindow::showTableMenuT(QPoint p)
{
    StrDebug();

    QMenu *menuTableT = new QMenu(ui->tableT);
    menuTableT->addAction(ui->addGoodsAction);
    menuTableT->addAction(ui->delGoodsAction);
    menuTableT->addAction(ui->editGoodsAction);
    menuTableT->exec(ui->tableT->mapToGlobal(p));

    delete menuTableT;
    menuTableT = nullptr;
}

/** Slot
 *  Show context menu
 */
void MainWindow::showTableMenuK(QPoint p)
{
    StrDebug();

    auto menuTable = std::make_unique<QMenu>(ui->tableK);

    menuTable->addAction(ui->addBuyersAction);
    menuTable->addAction(ui->delBuyersAction);
    menuTable->addAction(ui->editBuyersAction);
    menuTable->exec(ui->tableK->mapToGlobal(p));
}

void MainWindow::showTableMenuM(QPoint p)
{
    StrDebug();

    auto menuTable = std::make_unique<QMenu>(ui->tableM);

    menuTable->addAction(ui->WZAction);
    menuTable->addAction(ui->RWAction);
    menuTable->addAction(ui->warehouseEdAction);
    menuTable->addAction(ui->warehouseDelAction);
    menuTable->exec(ui->tableM->mapToGlobal(p));
}

/** Slot
 *  Show context menu
 */
void MainWindow::showTableMenuH(QPoint p)
{
    StrDebug();

    auto menuTable = std::make_unique<QMenu>(ui->tableH);

    menuTable->addAction(ui->addInvoiceAction);
    menuTable->addAction(ui->invoiceGrossAction);
    menuTable->addAction(ui->invoiceRRAction);
    menuTable->addAction(ui->invoiceBillAction);
    menuTable->addSeparator();
    menuTable->addAction(ui->invoiceProFormaAction);
    menuTable->addAction(ui->invoiceCorrAction);
    menuTable->addAction(ui->invoiceDuplAction);
    menuTable->addSeparator();
    menuTable->addAction(ui->invoiceEdAction);
    menuTable->addAction(ui->invoiceDelAction);
    menuTable->exec(ui->tableH->mapToGlobal(p));
}

/** Slot
 *  StatusBar slot
 */

void MainWindow::mainUpdateStatus(QTableWidgetItem *item)
{
    // cast is required since the names of method and objects inside MainWindow
    // class and
    // QMainWindow and UiMainWindow are the same... I guess there is a way to
    // avoid it.

    StrDebug();

    QStatusBar *stat = qobject_cast<QMainWindow *>(this)->statusBar();

    QTableWidget *table = item->tableWidget();
    QString message = QString();

    message +=
        table->horizontalHeaderItem(1)->text() + " : " + table->item(item->row(), 1)->text() + ", ";
    message +=
        table->horizontalHeaderItem(2)->text() + " : " + table->item(item->row(), 2)->text() + ", ";
    message += table->horizontalHeaderItem(3)->text() + " : " + table->item(item->row(), 3)->text();

    stat->showMessage(trUtf8("Wybrana pozycja: ") + message);
}

/** Slot which enables/disables menu. It's possible to add/remove
 * goods/customers
 *  only if this is the current tab.
 */

void MainWindow::tabChanged()
{
    // history
    if (ui->tableH->rowCount() != 0)
    {
        ui->invoiceEdAction->setEnabled(true);
        ui->invoiceDelAction->setEnabled(true);
    }
    else
    {
        ui->invoiceEdAction->setDisabled(true);
        ui->invoiceDelAction->setDisabled(true);
    }

    // buyers
    if (ui->tableK->rowCount() != 0)
    {
        ui->editBuyersAction->setEnabled(true);
        ui->delBuyersAction->setEnabled(true);
    }
    else
    {
        ui->editBuyersAction->setDisabled(true);
        ui->delBuyersAction->setDisabled(true);
    }

    // goods
    if (ui->tableT->rowCount() != 0)
    {
        ui->editGoodsAction->setEnabled(true);
        ui->delGoodsAction->setEnabled(true);
    }
    else
    {
        ui->editGoodsAction->setDisabled(true);
        ui->delGoodsAction->setDisabled(true);
    }
}

/** Slot used to read the invoices, calls readHist.
 */
void MainWindow::rereadHist(bool)
{
    StrDebug();

    if (ui->filtrStart->date() > ui->filtrEnd->date())
    {
        QMessageBox::information(
            this,
            trUtf8("Filtr dat"),
            trUtf8("Data początkowa nie może być większa od daty końcowej"));
    }
    else
    {
        tableClear(ui->tableH);
        ui->tableH->setSortingEnabled(false);
        readHist();
    }
}

/** Slot used to read the invoices, calls readHist.
 */
void MainWindow::rereadWarehouses(bool)
{
    StrDebug();

    if (ui->warehouseFromDate->date() > ui->warehouseToDate->date())
    {
        QMessageBox::information(
            this,
            trUtf8("Filtr dat"),
            trUtf8("Data początkowa nie może być większa od daty końcowej"));
    }
    else
    {
        tableClear(ui->tableM);
        ui->tableM->setSortingEnabled(false);
        readWarehouses();
    }
}

/** Slot used to display information about QFaktury
 */

void MainWindow::aboutProg()
{
    StrDebug();

    QMessageBox::about(
        this,
        trUtf8("O programie"),
        trUtf8("<h2>Program do wystawiania faktur</h2><br/><p>") + sett().getVersion(qAppName())
            + trUtf8("<br/>Wymagane Qt >= 5.10.0") + trUtf8("<br/>Kompilowane z Qt ")
            + QT_VERSION_STR + trUtf8("<br/>Twoja aktualna wersja - Qt ") + qVersion()
            + trUtf8("</p><p>Dawni koordynatorzy projektu: Grzegorz Rękawek, "
                     "Jagoda \"juliagoda\" "
                     "Górska</p>"
                     "<p>Dawni programiści: Tomasz Pielech, Rafał Rusin "
                     "http://people.apache.org/~rr/, Sławomir Patyk, Jagoda \"juliagoda\" "
                     "Górska</p>")
            + trUtf8("<p>Ikony: Dariusz Arciszewski, Jagoda \"juliagoda\" Górska </p>"
                     "<p>Portowanie na Qt5: "
                     "Jagoda \"juliagoda\" Górska</p><br/>")
            + trUtf8("<p>Testy w środowisku Arch Linux: Piotr \"sir_lucjan\" "
                     "Górski && Paweł \"pavbaranov\" Baranowski</p>")
            + trUtf8("<p>Tworzenie plików PKGBUILD: Piotr \"sir_lucjan\" "
                     "Górski</p>")
            + trUtf8("<h2><center>UWAGA!!!</center></h2>")
            + trUtf8("<p align=\"justify\">Ten program komputerowy dostarczany "
                     "jest przez autora w formie \"takiej, jaki jest\". ")
            + trUtf8("Autor nie udziela żadnej gwarancji oraz rękojmi, że program "
                     "będzie działał ")
            + trUtf8("prawidłowo, jest odpowiedniej jakości oraz że spełni "
                     "oczekiwania ")
            + trUtf8("użytkownika. Autor nie odpowiada za jakiekolwiek straty "
                     "wynikające z użytkowania ")
            + trUtf8("programu, w tym utratą spodziewanych korzyści, danych, "
                     "informacji ")
            + trUtf8("gospodarczych lub koszt urządzeń lub programów "
                     "zastępczych.</p>"));
}

/** Slot used to edit the invoice from list of invoices.
 */

void MainWindow::editFHist()
{
    StrDebug();

    if (ui->tableH->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Faktura nie wybrana. Nie mozna edytować."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    ui->tableH->setSortingEnabled(false);

    int row { 0 };

    row = ui->tableH->selectedItems().at(0)->row();

    if (ui->tableH->item(row, 3)->text() == trUtf8("korekta"))
    {
        auto corWindow = std::make_unique<Correction>(this, dl, s_WIN_CORRECT_EDIT, true);

        corWindow->correctionInit(true);
        corWindow->readCorrData(ui->tableH->item(row, 0)->text());

        if (corWindow->exec() == QDialog::Accepted)
        {
            rereadHist(true);
        }

        if (corWindow->getKAdded())
            readBuyer();
    }

    if (ui->tableH->item(row, 3)->text() == trUtf8("kbrutto"))
    {
        auto corWindow = std::make_unique<CorrectGross>(this, dl, s_WIN_CORRECT_EDIT, true);

        corWindow->correctionInit(true);
        corWindow->readCorrData(ui->tableH->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = corWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            corWindow->setSizePolicy(sp_retain);
            corWindow->hide();
            corWindow->makeInvoice(false);
        }
        else
        {
            if (corWindow->exec() == QDialog::Accepted)
            {
                rereadHist(true);
            }

            if (corWindow->getKAdded())
                readBuyer();
        }
    }

    if (ui->tableH->item(row, 3)->text() == trUtf8("rachunek"))
    {
        auto billWindow = std::make_unique<Bill>(this, dl, s_BILL_EDIT);

        billWindow->readData(ui->tableH->item(row, 0)->text());
        billWindow->setfName(ui->tableH->item(row, 0)->text());
        billWindow->billInit();
        billWindow->setWindowTitle(trUtf8("Edytuje Rachunek"));

        if (shouldHidden)
        {
            QSizePolicy sp_retain = billWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            billWindow->setSizePolicy(sp_retain);
            billWindow->hide();
            billWindow->makeInvoice(false);
        }
        else
        {
            if (billWindow->exec() == QDialog::Accepted)
            {
                rereadHist(true);
            }

            if (billWindow->getKAdded())
                readBuyer();
        }
    }

    if (ui->tableH->item(row, 3)->text() == trUtf8("FVAT"))
    {
        auto invWindow = std::make_unique<Invoice>(this, dl, s_WIN_INVOICE_EDIT);

        invWindow->readData(ui->tableH->item(row, 0)->text());
        invWindow->setfName(ui->tableH->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = invWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            invWindow->setSizePolicy(sp_retain);
            invWindow->hide();
            invWindow->makeInvoice(false);
        }
        else
        {
            if (invWindow->exec() == QDialog::Accepted)
            {
                rereadHist(true);
            }

            if (invWindow->getKAdded())
                readBuyer();
        }
    }

    if (ui->tableH->item(row, 3)->text() == trUtf8("RR"))
    {
        auto invWindow = std::make_unique<InvoiceRR>(this, dl, s_RR);

        invWindow->invoiceRRInit();
        invWindow->readData(ui->tableH->item(row, 0)->text());
        invWindow->setfName(ui->tableH->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = invWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            invWindow->setSizePolicy(sp_retain);
            invWindow->hide();
            invWindow->makeInvoice(false);
        }
        else
        {
            if (invWindow->exec() == QDialog::Accepted)
            {
                rereadHist(true);
            }

            if (invWindow->getKAdded())
                readBuyer();
        }
    }

    if (ui->tableH->item(row, 3)->text() == trUtf8("FPro"))
    {
        auto invWindow = std::make_unique<Invoice>(this, dl, s_WIN_PROFORMA_EDIT);

        invWindow->readData(ui->tableH->item(row, 0)->text());
        invWindow->setfName(ui->tableH->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = invWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            invWindow->setSizePolicy(sp_retain);
            invWindow->hide();
            invWindow->makeInvoice(false);
        }
        else
        {
            if (invWindow->exec() == QDialog::Accepted)
            {
                rereadHist(true);
            }

            if (invWindow->getKAdded())
                readBuyer();
        }
    }

    if (ui->tableH->item(row, 3)->text() == trUtf8("FBrutto"))
    {
        auto invWindow = std::make_unique<InvoiceGross>(this, dl, s_BR_INVOICE_EDIT);

        invWindow->readData(ui->tableH->item(row, 0)->text());
        invWindow->setfName(ui->tableH->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = invWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            invWindow->setSizePolicy(sp_retain);
            invWindow->hide();
            invWindow->makeInvoice(false);
        }
        else
        {
            if (invWindow->exec() == QDialog::Accepted)
            {
                rereadHist(true);
            }

            if (invWindow->getKAdded())
                readBuyer();
        }
    }

    if (ui->tableH->item(row, 3)->text() == trUtf8("duplikat"))
    {
        auto dupWindow = std::make_unique<Duplicate>(this, dl, s_WIN_DUPLICATE_LOOK, true);

        dupWindow->readData(ui->tableH->item(row, 0)->text());
        dupWindow->duplicateInit();
        dupWindow->setIsEditAllowed(false);
        dupWindow->setfName(ui->tableH->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = dupWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            dupWindow->setSizePolicy(sp_retain);
            dupWindow->hide();
            dupWindow->makeInvoice(false);
        }
        else
        {
            if (dupWindow->exec() == QDialog::Accepted)
            {
                rereadHist(true);
            }

            if (dupWindow->getKAdded())
                readBuyer();
        }
    }
    ui->tableH->setSortingEnabled(true);
}

void MainWindow::warehouseEdit()
{
    StrDebug();

    if (ui->tableM->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Dokument magazynu nie został wybrany. Nie można edytować."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    ui->tableM->setSortingEnabled(false);

    int row { 0 };

    row = ui->tableM->selectedItems().at(0)->row();

    if (ui->tableM->item(row, 3)->text() == trUtf8("WZ"))
    {
        auto delivNoteWindow = std::make_unique<DeliveryNote>(this, dl, s_WZ);

        delivNoteWindow->readWarehouseData(ui->tableM->item(row, 0)->text());
        delivNoteWindow->setfName(ui->tableM->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = delivNoteWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            delivNoteWindow->setSizePolicy(sp_retain);
            delivNoteWindow->hide();
            delivNoteWindow->makeInvoice(false);
        }
        else
        {
            if (delivNoteWindow->exec() == QDialog::Accepted)
            {
                rereadWarehouses(true);
                rereadHist(true);
            }

            if (delivNoteWindow->getKAdded())
                readBuyer();
        }
    }

    if (ui->tableM->item(row, 3)->text() == trUtf8("RW"))
    {
        auto goodsNoteWindow = std::make_unique<GoodsIssuedNotes>(this, dl, s_RW);

        goodsNoteWindow->readWarehouseData(ui->tableM->item(row, 0)->text());
        goodsNoteWindow->setfName(ui->tableM->item(row, 0)->text());

        if (shouldHidden)
        {
            QSizePolicy sp_retain = goodsNoteWindow->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            goodsNoteWindow->setSizePolicy(sp_retain);
            goodsNoteWindow->hide();
            goodsNoteWindow->makeInvoice(false);
        }
        else
        {
            if (goodsNoteWindow->exec() == QDialog::Accepted)
            {
                rereadWarehouses(true);
            }

            if (goodsNoteWindow->getKAdded())
                readBuyer();
        }
    }
    ui->tableM->setSortingEnabled(true);
}

void MainWindow::delFHist()
{
    StrDebug();

    if (ui->tableH->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Faktura nie została wybrana. Nie można usuwać."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    if (QMessageBox::warning(
            this,
            sett().getVersion(qAppName()),
            trUtf8("Czy napewno chcesz usunąć tą fakturę z historii?"),
            trUtf8("Tak"),
            trUtf8("Nie"),
            nullptr,
            0,
            1)
        == 0)
    {
        QString name = ui->tableH->item(ui->tableH->currentRow(), 0)->text();
        dl->invoiceDeleteData(name);
        ui->tableH->removeRow(ui->tableH->currentRow());
        allSymbols = dl->getAllSymbols();
    }
}

void MainWindow::delMHist()
{
    StrDebug();

    if (ui->tableM->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Dokument magazynu nie został wybrany. Nie można usuwać."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    if (QMessageBox::warning(
            this,
            sett().getVersion(qAppName()),
            trUtf8("Czy napewno chcesz usunąć ten dokument magazynu z historii?"),
            trUtf8("Tak"),
            trUtf8("Nie"),
            nullptr,
            0,
            1)
        == 0)
    {
        QString name = ui->tableM->item(ui->tableM->currentRow(), 0)->text();
        dl->warehouseDeleteData(name);
        ui->tableM->removeRow(ui->tableM->currentRow());
        allSymbolsWarehouse = dl->getAllSymbolsWarehouse();
    }
}

/** Slot used to edit data of the current company
 */

void MainWindow::userDataClick()
{
    StrDebug();

    auto userDataWindow = std::make_unique<User>(this);
    userDataWindow->exec();
}

/** Slot used to edit sett()
 */

void MainWindow::settClick()
{
    StrDebug();

    auto settWindow = std::make_unique<Setting>(this);
    settWindow->exec();
}

/** Slot used to add new customer
 */

void MainWindow::buyerClick()
{
    StrDebug();

    auto buyersWindow = std::make_unique<Buyers>(this, 0, dl);

    if (buyersWindow->exec() == QDialog::Accepted)
    {
        ui->tableK->setSortingEnabled(false);
        insertRow(ui->tableK, ui->tableK->rowCount());

        QStringList row = buyersWindow->getRetBuyer().split("|");

        ui->tableK->item(ui->tableK->rowCount() - 1, 0)->setText(row[0]); // name
        ui->tableK->item(ui->tableK->rowCount() - 1, 1)->setText(row[1]); // type
        ui->tableK->item(ui->tableK->rowCount() - 1, 2)->setText(row[2]); // place
        ui->tableK->item(ui->tableK->rowCount() - 1, 3)->setText(row[3]); // address
        ui->tableK->item(ui->tableK->rowCount() - 1, 4)->setText(row[4]); // telefon
        ui->tableK->item(ui->tableK->rowCount() - 1, 5)->setText(row[8]); // email
        ui->tableK->item(ui->tableK->rowCount() - 1, 6)->setText(row[9]); // www*/
        ui->tableK->setSortingEnabled(true);
    }
}

/** Slot used to delete current customer
 */

void MainWindow::buyerDel()
{
    StrDebug();

    if (ui->tableK->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Kontrahent nie został wybrany. Nie można usuwac."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    if (QMessageBox::warning(
            this,
            trUtf8("QFaktury"),
            trUtf8("Czy napewno chcesz usunąć kontrahenta: ")
                + ui->tableK->item(ui->tableK->currentRow(), 0)->text() + trUtf8(" ?"),
            trUtf8("Tak"),
            trUtf8("Nie"),
            nullptr,
            0,
            1)
        == 0)
    {
        dl->buyersDeleteData(ui->tableK->item(ui->tableK->currentRow(), 0)->text());
        ui->tableK->removeRow(ui->tableK->currentRow());
    }
}

/** Slot used to edit customer
 */

void MainWindow::buyerEd()
{
    StrDebug();

    if (ui->tableK->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Kontrahent nie został wybrany."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    int row = ui->tableK->selectedItems()[0]->row();

    auto buyersWindow = std::make_unique<Buyers>(this, 1, dl);

    buyersWindow->selectData(
        ui->tableK->item(row, 0)->text(), sett().getCustomerType(ui->tableK->item(row, 1)->text()));

    if (buyersWindow->exec() == QDialog::Accepted)
    {
        ui->tableK->setSortingEnabled(false);
        QStringList rowTxt = buyersWindow->getRetBuyer().split("|");
        ui->tableK->item(row, 0)->setText(rowTxt[0]); // name
        ui->tableK->item(row, 1)->setText(rowTxt[1]); // type
        ui->tableK->item(row, 2)->setText(rowTxt[2]); // place
        ui->tableK->item(row, 3)->setText(rowTxt[3]); // address
        ui->tableK->item(row, 4)->setText(rowTxt[4]); // telefon
        ui->tableK->item(row, 5)->setText(rowTxt[8]); // email
        ui->tableK->item(row, 6)->setText(rowTxt[9]); // www
        ui->tableK->setSortingEnabled(true);
    }
}

void MainWindow::printBuyerList()
{
    StrDebug();

    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);

    preview.setWindowFlags(Qt::Window);
    preview.setWindowTitle(trUtf8("Lista kontrahentów"));

    connect(&preview, SIGNAL(paintRequested(QPrinter *)), this, SLOT(printList(QPrinter *)));

    if (preview.exec() == 1)
    {
        QMessageBox::warning(
            this,
            trUtf8("Drukowanie"),
            trUtf8("Prawdopobnie nie masz skonfigurowanej drukarki. Wykrywana "
                   "nazwa domyślnej drukarki to: ")
                + printer.printerName()
                + trUtf8(". Status domyślnej drukarki (poprawny o ile drukarka ma "
                         "możliwość raportowania statusu do systemu): ")
                + printer.printerState());
    }
}

/** Slot print
 *  Helper slot used to display print preview
 */

void MainWindow::printList(QPrinter *printer)
{
    StrDebug();

    if (ui->tableK->rowCount() != 0)
    {
        QTextDocument doc(trUtf8("Lista kontrahentów"));
        QStringList list = QStringList();
        list << "<!doctype html>"
             << "<head>"
             << "<meta charset=\"utf-8\" />"
             << "</head>";
        list << "<style type=\"text/css\">";
        list << "strong { font-weight: 500; }";
        list << ".page_break {page-break-inside: avoid;}";
        list << "</style>";
        list << "<body>";

        QVector<BuyerData> buyerVec = dl->buyersSelectAllData();

        for (int i = 0; i < buyerVec.size(); ++i)
        {
            QString text = buyerVec.at(i).name;
            list << "<strong>" + trUtf8("Nazwa:  ") + "</strong>" + changeIfEmpty(text);
            list << "<br/>";
            text = buyerVec.at(i).type;
            list << "<strong>" + trUtf8("Rodzaj:  ") + "</strong>" + changeIfEmpty(text);
            list << "<br/>";
            text = buyerVec.at(i).place;
            list << "<strong>" + trUtf8("Miejscowość:  ") + "</strong>" + changeIfEmpty(text);
            list << "<br/>";
            text = buyerVec.at(i).address;
            list << "<strong>" + trUtf8("Adres:  ") + "</strong>" + changeIfEmpty(text);
            list << "<br/>";
            text = buyerVec.at(i).phone;
            list << "<strong>" + trUtf8("Tel:  ") + "</strong>" + changeIfEmpty(text);
            list << "<br/>";
            text = buyerVec.at(i).account;
            list << "<strong>" + trUtf8("Nr konta:  ") + "</strong>" + changeIfEmpty(text);
            list << "<br/>";
            text = buyerVec.at(i).email;
            list << "<strong>" + trUtf8("Email:  ") + "</strong>" + changeIfEmpty(text);
            list << "<br/>";
            text = buyerVec.at(i).www;
            list << "<strong>" + trUtf8("Strona:  ") + "</strong>" + changeIfEmpty(text);
            list << "<p> --------------------------------------------- </p>";
            list << "<p class=\"page_break\"></p>";
        }

        list << "</body>";
        list << "</html>";

        QFile file(sett().getWorkingDir() + "/buyerContacts.html");

        bool removed { false };

        if (file.exists())
            removed = file.remove();

        if (!removed)
        {
            QFile(file.fileName()).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
            file.remove();
        }

        if (!file.open(QIODevice::WriteOnly))
        {
            QFile(file.fileName()).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }

        QTextStream stream(&file);

        for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
            stream << *it << "\n";

        file.close();

        doc.setHtml(list.join(" "));
        doc.print(printer);
    }
    else
    {
        if (QMessageBox::warning(
                this,
                trUtf8("Brak kontrahentów"),
                trUtf8("Aby móc wydrukować listę kontaktów, musisz mieć "
                       "wprowadzonego co najmniej jednego kontrahenta do tabeli. "
                       "Czy chcesz dodać teraz dane twojego kontrahenta?"),
                trUtf8("Tak"),
                trUtf8("Nie"),
                nullptr,
                0,
                1)
            == 0)
        {
            buyerClick();
        }
    }
}

QString MainWindow::changeIfEmpty(QString text)
{
    StrDebug();

    QString result = QString();

    if (text.isEmpty())
    {
        result = "-";
    }
    else
    {
        result = text;
    }

    return result;
}

/** Slot used for creating new invoices
 */

void MainWindow::newInv()
{
    StrDebug();

    auto invWindow = std::make_unique<Invoice>(this, dl, s_INVOICE);

    if (QDialog::Accepted == invWindow->exec())
    {
        ui->tableH->setSortingEnabled(false);
        insertRow(ui->tableH, ui->tableH->rowCount());

        QStringList row = invWindow->getRet().split("|");

        ui->tableH->item(ui->tableH->rowCount() - 1, 0)->setText(row[0]); // file name
        ui->tableH->item(ui->tableH->rowCount() - 1, 1)->setText(row[1]); // symbol
        ui->tableH->item(ui->tableH->rowCount() - 1, 2)->setText(row[2]); // date
        ui->tableH->item(ui->tableH->rowCount() - 1, 3)->setText(row[3]); // type
        ui->tableH->item(ui->tableH->rowCount() - 1, 4)->setText(row[4]); // buyer
        ui->tableH->item(ui->tableH->rowCount() - 1, 5)->setText(row[5]); // NIP
        ui->tableH->setSortingEnabled(true);
    }
    else
    {
        rereadHist(true);
    }

    if (invWindow->getKAdded())
        readBuyer();

    dl->checkAllSymbInFiles();
    allSymbols = dl->getAllSymbols();
}

void MainWindow::newInvRR()
{
    StrDebug();

    auto invWindow = std::make_unique<InvoiceRR>(this, dl, s_RR);

    invWindow->invoiceRRInit();

    if (invWindow->exec() == QDialog::Accepted)
    {
        ui->tableH->setSortingEnabled(false);
        insertRow(ui->tableH, ui->tableH->rowCount());

        QStringList row = invWindow->getRet().split("|");

        ui->tableH->item(ui->tableH->rowCount() - 1, 0)->setText(row[0]); // file name
        ui->tableH->item(ui->tableH->rowCount() - 1, 1)->setText(row[1]); // symbol
        ui->tableH->item(ui->tableH->rowCount() - 1, 2)->setText(row[2]); // date
        ui->tableH->item(ui->tableH->rowCount() - 1, 3)->setText(row[3]); // type
        ui->tableH->item(ui->tableH->rowCount() - 1, 4)->setText(row[4]); // buyer
        ui->tableH->item(ui->tableH->rowCount() - 1, 5)->setText(row[5]); // NIP
        ui->tableH->setSortingEnabled(true);
    }
    else
    {
        rereadHist(true);
    }

    if (invWindow->getKAdded())
        readBuyer();

    dl->checkAllSymbInFiles();
    allSymbols = dl->getAllSymbols();
}

/** Slot used for creating new invoices
 */
void MainWindow::newInvBill()
{
    StrDebug();

    auto billWindow = std::make_unique<Bill>(this, dl, s_BILL);

    billWindow->setWindowTitle(trUtf8("Rachunek"));
    billWindow->billInit();

    if (billWindow->exec() == QDialog::Accepted)
    {
        ui->tableH->setSortingEnabled(false);
        insertRow(ui->tableH, ui->tableH->rowCount());

        QStringList row = billWindow->getRet().split("|");

        ui->tableH->item(ui->tableH->rowCount() - 1, 0)->setText(row[0]); // file name
        ui->tableH->item(ui->tableH->rowCount() - 1, 1)->setText(row[1]); // symbol
        ui->tableH->item(ui->tableH->rowCount() - 1, 2)->setText(row[2]); // date
        ui->tableH->item(ui->tableH->rowCount() - 1, 3)->setText(row[3]); // type
        ui->tableH->item(ui->tableH->rowCount() - 1, 4)->setText(row[4]); // buyer
        ui->tableH->item(ui->tableH->rowCount() - 1, 5)->setText(row[5]); // NIP
        ui->tableH->setSortingEnabled(true);
    }
    else
    {
        rereadHist(true);
    }

    if (billWindow->getKAdded())
        readBuyer();

    dl->checkAllSymbInFiles();
    allSymbols = dl->getAllSymbols();
}

/** Slot used for creating new invoices
 */
void MainWindow::newInvGross()
{
    StrDebug();

    auto invWindow = std::make_unique<InvoiceGross>(this, dl, s_FBRUTTO);

    invWindow->setWindowTitle(trUtf8("Faktura VAT Brutto"));

    if (invWindow->exec() == QDialog::Accepted)
    {
        ui->tableH->setSortingEnabled(false);
        insertRow(ui->tableH, ui->tableH->rowCount());

        QStringList row = invWindow->getRet().split("|");

        ui->tableH->item(ui->tableH->rowCount() - 1, 0)->setText(row[0]); // file name
        ui->tableH->item(ui->tableH->rowCount() - 1, 1)->setText(row[1]); // symbol
        ui->tableH->item(ui->tableH->rowCount() - 1, 2)->setText(row[2]); // date
        ui->tableH->item(ui->tableH->rowCount() - 1, 3)->setText(row[3]); // type
        ui->tableH->item(ui->tableH->rowCount() - 1, 4)->setText(row[4]); // buyer
        ui->tableH->item(ui->tableH->rowCount() - 1, 5)->setText(row[5]); // NIP
        ui->tableH->setSortingEnabled(true);
    }
    else
    {
        rereadHist(true);
    }

    if (invWindow->getKAdded())
        readBuyer();

    dl->checkAllSymbInFiles();
    allSymbols = dl->getAllSymbols();
}

/** Slot used to create new ProForma Invoice
 */

void MainWindow::newPForm()
{
    StrDebug();

    auto invWindow = std::make_unique<Invoice>(this, dl, s_PROFORMA);

    invWindow->setWindowTitle(trUtf8("Faktura Pro Forma"));
    invWindow->backBtnClick();

    if (invWindow->exec() == QDialog::Accepted)
    {
        ui->tableH->setSortingEnabled(false);
        insertRow(ui->tableH, ui->tableH->rowCount());

        QStringList row = invWindow->getRet().split("|");

        ui->tableH->item(ui->tableH->rowCount() - 1, 0)->setText(row[0]); // file name
        ui->tableH->item(ui->tableH->rowCount() - 1, 1)->setText(row[1]); // symbol
        ui->tableH->item(ui->tableH->rowCount() - 1, 2)->setText(row[2]); // date
        ui->tableH->item(ui->tableH->rowCount() - 1, 3)->setText(row[3]); // type
        ui->tableH->item(ui->tableH->rowCount() - 1, 4)->setText(row[4]); // buyer
        ui->tableH->item(ui->tableH->rowCount() - 1, 5)->setText(row[5]); // NIP
        ui->tableH->setSortingEnabled(true);
    }
    else
    {
        rereadHist(true);
    }

    if (invWindow->getKAdded())
        readBuyer();

    dl->checkAllSymbInFiles();
    allSymbols = dl->getAllSymbols();
}

/** Slot used to create new Correction
 */

void MainWindow::newCor()
{
    StrDebug();

    if (ui->tableH->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Faktura nie została wybrana. Wybierz fakurę, do "
                   "której chcesz wystawić korektę."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    int row = ui->tableH->selectedItems()[0]->row();

    QStringList invTypes = QStringList("FVAT");
    invTypes << "FBrutto";

    if (invTypes.contains(ui->tableH->item(row, 3)->text()))
    {
        ui->tableH->setSortingEnabled(false);

        std::unique_ptr<Correction> corWindow;

        if (ui->tableH->item(row, 3)->text().contains("FVAT"))
        {
            corWindow = std::make_unique<Correction>(this, dl, s_CORRECT_TITLE, false);
        }
        else
        {
            corWindow = std::make_unique<CorrectGross>(this, dl, s_CORRECT_BRUTTO, false);
        }

        corWindow->correctionInit(false);
        corWindow->readData(ui->tableH->item(row, 0)->text());
        corWindow->setWindowTitle(trUtf8("Nowa korekta"));

        if (corWindow->exec() == QDialog::Accepted)
        {
            insertRow(ui->tableH, ui->tableH->rowCount());
            QStringList row = corWindow->getRet().split("|");
            int newRow = ui->tableH->rowCount() - 1;
            ui->tableH->item(newRow, 0)->setText(row[0]); // file name
            ui->tableH->item(newRow, 1)->setText(row[1]); // symbol
            ui->tableH->item(newRow, 2)->setText(row[2]); // date
            ui->tableH->item(newRow, 3)->setText(row[3]); // type
            ui->tableH->item(newRow, 4)->setText(row[4]); // nabywca
            ui->tableH->item(newRow, 5)->setText(row[5]); // NIP
        }
        else
        {
            rereadHist(true);
        }

        if (corWindow->getKAdded())
            readBuyer();

        dl->checkAllSymbInFiles();
        allSymbols = dl->getAllSymbols();
        ui->tableH->setSortingEnabled(true);
    }
    else
    {
        QMessageBox::information(
            this,
            "QFaktury",
            trUtf8("Do faktury typu ") + ui->tableH->item(row, 3)->text()
                + (" nie wystawiamy korekt."),
            QMessageBox::Ok);
    }
}

/** Slot
 *  Creates duplicate
 */

void MainWindow::newDuplicate()
{
    StrDebug();

    if (ui->tableH->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Faktura nie wybrana. Wybierz fakurę, do "
                   "której chcesz wystawić duplikat."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    int row { ui->tableH->selectedItems()[0]->row() };

    // types of invoices for which it's ok to issue a duplicate
    QStringList invTypes = QStringList("FVAT");
    invTypes << "FBrutto";

    if (invTypes.contains(ui->tableH->item(row, 3)->text()))
    {
        auto dupWindow = std::make_unique<Duplicate>(this, dl, s_DUPLICATE, false);

        dupWindow->readData(ui->tableH->item(row, 0)->text());
        dupWindow->setWindowTitle(trUtf8("Nowy duplikat"));
        dupWindow->duplicateInit();

        if (dupWindow->exec() == QDialog::Accepted)
        {
            insertRow(ui->tableH, ui->tableH->rowCount());

            QStringList row = dupWindow->getRet().split("|");

            int newRow { ui->tableH->rowCount() - 1 };

            ui->tableH->item(newRow, 0)->setText(row[0]); // file name
            ui->tableH->item(newRow, 1)->setText(row[1]); // symbol
            ui->tableH->item(newRow, 2)->setText(row[2]); // date
            ui->tableH->item(newRow, 3)->setText(row[3]); // type
            ui->tableH->item(newRow, 4)->setText(row[4]); // nabywca
            ui->tableH->item(newRow, 5)->setText(row[5]); // NIP
        }
        else
        {
            rereadHist(true);
        }

        if (dupWindow->getKAdded())
            readBuyer();

        dl->checkAllSymbInFiles();
        allSymbols = dl->getAllSymbols();
        ui->tableH->setSortingEnabled(true);
    }
    else
    {
        QMessageBox::information(
            this,
            "QFaktury",
            trUtf8("Do faktury typu ") + ui->tableH->item(row, 3)->text()
                + (" nie wystawiamy duplikatów."),
            QMessageBox::Ok);
    }
}

/** Slot used to add goods
 */

void MainWindow::goodsAdd()
{
    StrDebug();

    auto goodsWindow = std::make_unique<Goods>(this, 0, dl);

    if (goodsWindow->exec() == QDialog::Accepted)
    {
        ui->tableT->setSortingEnabled(false);
        insertRow(ui->tableT, ui->tableT->rowCount());

        QStringList row = goodsWindow->getRetGoods().split("|");

        ui->tableT->item(ui->tableT->rowCount() - 1, 0)->setText(row[0]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 1)->setText(row[1]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 2)->setText(row[2]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 3)->setText(row[3]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 4)->setText(row[4]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 5)->setText(row[5]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 6)->setText(row[6]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 7)->setText(row[7]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 8)->setText(row[8]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 9)->setText(row[9]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 10)->setText(row[10]);
        ui->tableT->item(ui->tableT->rowCount() - 1, 11)->setText(row[11]);
        ui->tableT->setSortingEnabled(true);
    }
}

/** Slot used to delete goods
 */
void MainWindow::goodsDel()
{
    StrDebug();

    if (ui->tableT->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Towar nie został wybrany. Nie można usuwać."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    int row = ui->tableT->currentRow();

    if (QMessageBox::warning(
            this,
            trUtf8("QFaktury"),
            trUtf8("Czy napewno chcesz usunąć towar ") + ui->tableT->item(row, 0)->text() + "/"
                + ui->tableT->item(row, 1)->text() + "?",
            trUtf8("Tak"),
            trUtf8("Nie"),
            nullptr,
            0,
            1)
        == 0)
    {
        dl->productsDeleteData(ui->tableT->item(row, 0)->text());
        ui->tableT->removeRow(row);
    }
}

/** Slot used for editing goods
 */

void MainWindow::goodsEdit()
{
    StrDebug();

    if (ui->tableT->selectedItems().count() <= 0)
    {
        QMessageBox::information(
            this,
            trUtf8("QFaktury"),
            trUtf8("Towar nie został wybrany. Nie można edytować."),
            trUtf8("Ok"),
            nullptr,
            nullptr,
            1);
        return;
    }

    int row { ui->tableT->selectedItems()[0]->row() };

    auto goodsWindow = std::make_unique<Goods>(this, 1, dl);

    goodsWindow->selectData(
        ui->tableT->item(row, 0)->text(), sett().getProductType(ui->tableT->item(row, 5)->text()));

    if (goodsWindow->exec() == QDialog::Accepted)
    {
        ui->tableT->setSortingEnabled(false);
        QStringList rowTxt = goodsWindow->getRetGoods().split("|");

        ui->tableT->item(row, 0)->setText(rowTxt[0]);
        ui->tableT->item(row, 1)->setText(rowTxt[1]);
        ui->tableT->item(row, 2)->setText(rowTxt[2]);
        ui->tableT->item(row, 3)->setText(rowTxt[3]);
        ui->tableT->item(row, 4)->setText(rowTxt[4]);
        ui->tableT->item(row, 5)->setText(rowTxt[5]);
        ui->tableT->item(row, 6)->setText(rowTxt[6]);
        ui->tableT->item(row, 7)->setText(rowTxt[7]);
        ui->tableT->item(row, 8)->setText(rowTxt[8]);
        ui->tableT->item(row, 9)->setText(rowTxt[9]);
        ui->tableT->item(row, 10)->setText(rowTxt[10]);
        ui->tableT->item(row, 11)->setText(rowTxt[11]);
        ui->tableT->setSortingEnabled(true);
    }
}

void MainWindow::findInvoicePdf()
{
    QDesktopServices::openUrl(QUrl(sett().getPdfDir(), QUrl::TolerantMode));
}

void MainWindow::openJPKDirectory()
{
    QFileInfo jpkInfoDir(sett().getJPKDir());

    if (jpkInfoDir.isDir() && jpkInfoDir.exists())
        QDesktopServices::openUrl(QUrl(sett().getJPKDir(), QUrl::TolerantMode));
    else
        QMessageBox::information(
            this,
            "Brak katalogu",
            "Aby otworzyć katalog, musi zostać wygenerowany chociaż jeden "
            "jednolity plik kontrolny dowolnego typu (JPK)");
}

void MainWindow::checkDateRange(const QDate &date)
{
    if (date.year() > QDate::currentDate().year())
    {
        QMessageBox::information(
            this,
            "Filtr faktur",
            "Rok dla wyszukiwanej faktury nie powinien być większy niż aktualny.");

        // e.g. casting to the class you know its connected with
        QDateEdit *dateEd = qobject_cast<QDateEdit *>(sender());
        if (nullptr != dateEd)
        {
            dateEd->setDate(QDate::currentDate());
        }
    }
}

void MainWindow::noteDownTask(const QDate &date)
{
    StrDebug();

    // is automatically set to 0 when the referenced object is destroyed
    QPointer<Organizer> organizer = new Organizer(ui->todayExercise, date);

    if (organizer.isNull())
        delete organizer;
}

void MainWindow::openHideOrganizer()
{
    StrDebug();

    if (ui->organizer->isHidden())
    {
        ui->organizer->show();
        ui->hideOrganizer->setText(">>");
    }
    else
    {
        ui->organizer->hide();
        ui->hideOrganizer->setText("<<");
    }
}

/** Slot close
 */

bool MainWindow::close()
{
    StrDebug();

    if (QMessageBox::question(
            this,
            trUtf8("Potwierdź"),
            trUtf8("Czy chcesz wyjść z programu?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes)
        == QMessageBox::Yes)
    {
        saveAllSett();
        return QMainWindow::close();
    }
    else
    {
        return false;
    }
}

/** Slot for sending email to buyers
 */

void MainWindow::sendEmailToBuyer()
{
    StrDebug();

    QPointer<Send> sendEmailWidget = new Send(
        dl->buyersSelectAllData(),
        dl->invoiceSelectAllData(ui->filtrStart->date(), ui->filtrEnd->date()));
    sendEmailWidget.data()->show();

    if (sendEmailWidget.isNull())
        delete sendEmailWidget;
}

/*
 *  Slot for delivery note creation (WZ)
 */

void MainWindow::on_WZAction_triggered()
{
    StrDebug();

    auto noteWindow = std::make_unique<DeliveryNote>(this, dl, s_WZ);

    noteWindow->setWindowTitle(trUtf8("WZ"));
    noteWindow->backBtnClick();

    if (noteWindow->exec() == QDialog::Accepted)
    {
        ui->tableM->setSortingEnabled(false);
        insertRow(ui->tableM, ui->tableM->rowCount());

        QStringList row = noteWindow->getRetWarehouse().split("|");

        ui->tableM->item(ui->tableM->rowCount() - 1, 0)->setText(row[0]); // file name
        ui->tableM->item(ui->tableM->rowCount() - 1, 1)->setText(row[1]); // symbol
        ui->tableM->item(ui->tableM->rowCount() - 1, 2)->setText(row[2]); // date
        ui->tableM->item(ui->tableM->rowCount() - 1, 3)->setText(row[3]); // type
        ui->tableM->item(ui->tableM->rowCount() - 1, 4)->setText(row[4]); // buyer
        ui->tableM->item(ui->tableM->rowCount() - 1, 5)->setText(row[5]); // NIP
        ui->tableM->setSortingEnabled(true);

        rereadWarehouses(true);
        rereadHist(true);
    }
    else
    {
        rereadWarehouses(true);
        rereadHist(true);
    }

    dl->checkAllSymbWareInFiles();
    allSymbolsWarehouse = dl->getAllSymbolsWarehouse();
}

// ----------------------------------------  SLOTS
// ---------------------------------//

void MainWindow::on_RWAction_triggered()
{
    StrDebug();

    auto noteWindow = std::make_unique<GoodsIssuedNotes>(this, dl, s_RW);

    noteWindow->setWindowTitle(trUtf8("RW"));
    noteWindow->backBtnClick();

    if (noteWindow->exec() == QDialog::Accepted)
    {
        ui->tableM->setSortingEnabled(false);
        insertRow(ui->tableM, ui->tableM->rowCount());

        QStringList row = noteWindow->getRetWarehouse().split("|");

        ui->tableM->item(ui->tableM->rowCount() - 1, 0)->setText(row[0]); // file name
        ui->tableM->item(ui->tableM->rowCount() - 1, 1)->setText(row[1]); // symbol
        ui->tableM->item(ui->tableM->rowCount() - 1, 2)->setText(row[2]); // date
        ui->tableM->item(ui->tableM->rowCount() - 1, 3)->setText(row[3]); // type
        ui->tableM->item(ui->tableM->rowCount() - 1, 4)->setText(row[4]); // buyer
        ui->tableM->item(ui->tableM->rowCount() - 1, 5)->setText(row[5]); // NIP
        ui->tableM->setSortingEnabled(true);

        rereadWarehouses(true);
    }
    else
    {
        rereadWarehouses(true);
    }

    dl->checkAllSymbWareInFiles();
    allSymbolsWarehouse = dl->getAllSymbolsWarehouse();
}

void MainWindow::openJPKGenerator()
{
    StrDebug();

    RunGuard guard("saftfile_run_protection");

    if (guard.tryToRun())
    {
        // is automatically set to 0 when the referenced object is destroyed
        QPointer<Saftfile> saftfileWindow = new Saftfile(dl);

        if (saftfileWindow.isNull())
            delete saftfileWindow;
    }
}

void MainWindow::createBackupMode()
{
    StrDebug();

    QPointer<Backup> backup = new Backup(QString("create"));
}

void MainWindow::loadBackupMode()
{
    StrDebug();

    QPointer<Backup> backup = new Backup(QString("load"));
}

void MainWindow::intervalBackup()
{
    StrDebug();

    QPointer<Backup> backup = new Backup(false);
}

void MainWindow::exportCsv()
{
    StrDebug();

    QPointer<CsvExport> csvexport = new CsvExport(dl);
}
