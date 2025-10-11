// one_step_backup.h
// Licensed under Apache 2.0

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
#include <QMap>
#include <QSet>
#include "file_type_selection.h"
#include "ui_one_step_backup.h"
#include "ui_about.h"

class one_step_backup : public QMainWindow
{
    Q_OBJECT

public:
    one_step_backup(QWidget *parent = nullptr);
    ~one_step_backup();

private slots:
    // Top menu
    void about();
    void aboutQt();

    // Main UI
    void browseSourceDirectory();
    void browseDestinationDirectory();
    void startBackup();
    void updateProgress(int value, const QString& message);
    void openFileTypeSelection();

private:
    // Top menu
    void createActions();
    void createMenus();

    QMenu* helpMenu;
    QAction* aboutAct;
    QAction* aboutQtAct;
    Ui::About aboutDialog;

    // Main UI
    Ui::one_step_backupClass ui;
    QLineEdit* sourceDirEdit;
    QLineEdit* destDirEdit;
    QPushButton* browseSourceBtn;
    QPushButton* browseDestBtn;
    QPushButton* selectFileTypesBtn;
    QPushButton* startBackupBtn;
    QProgressBar* progressBar;
    QListWidget* fileListWidget;

    QMap<QString, QStringList> fileTypeCategories;
    QSet<QString> selectedExtensions;

    void initializeFileTypeCategories();
    void applySelectedExtensions(const QSet<QString>& extensions);
    void refreshFileList();

    QStringList findMediaFiles(const QString& directory);
    bool copyFiles(const QStringList& files, const QString& destination);
    bool isMediaFile(const QString& filePath) const;
};
