#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef BACKUP_H
#define BACKUP_H

#include <QWidget>

namespace Ui
{
class Backup;
}

class Backup final : public QWidget
{
    Q_OBJECT

public:
    explicit Backup(
        QString action = QString("create"),
        QWidget *parent = nullptr); // "create" or "load"
    Backup(bool withGui, QWidget *parent = nullptr); // false -> without Gui
    ~Backup();

    const QString getBackupPath() const;
    const QString getDirectoryPath() const;
    const QString getFilename() const;

protected:
    void whenAutoBackup();
    void createBackupWithoutGui();

private slots:
    void choosePathBackup();
    void createBackup();

private:
    Ui::Backup *ui;

    void loadBackup();
    void prepareConnections();
    void putDataToWidgets(const QString dirPath, const QString filename);
};

#endif // BACKUP_H
