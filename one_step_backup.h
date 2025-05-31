#pragma once

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include "ui_one_step_backup.h"

class one_step_backup : public QMainWindow
{
    Q_OBJECT

public:
    one_step_backup(QWidget *parent = nullptr);
    ~one_step_backup();

private slots:
    void browseSourceDirectory();
    void browseDestinationDirectory();
    void startBackup();
    void updateProgress(int value, const QString& message);

private:
    Ui::one_step_backupClass ui;
    QLineEdit* sourceDirEdit;
    QLineEdit* destDirEdit;
    QPushButton* browseSourceBtn;
    QPushButton* browseDestBtn;
    QPushButton* startBackupBtn;
    QProgressBar* progressBar;
    QListWidget* fileListWidget;
    
    QStringList findMediaFiles(const QString& directory);
    bool copyFiles(const QStringList& files, const QString& destination);
    bool isMediaFile(const QString& filePath);
};
