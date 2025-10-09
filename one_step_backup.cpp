// one_step_backup.cpp
// Licensed under Apache 2.0

#include "one_step_backup.h"

#include <QApplication>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

// Define a default "home" directory based on OS and Qt's implementation of standard paths
#ifdef Q_OS_WIN
#define DEFAULT_DIRECTORY QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
#elif defined Q_OS_MAC
#define DEFAULT_DIRECTORY QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
#elif defined Q_OS_LINUX
#define DEFAULT_DIRECTORY QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
#else
#define DEFAULT_DIRECTORY QDir::homePath()
#endif

one_step_backup::one_step_backup(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowTitle("Media File Backup Tool");
    
    initializeFileTypeCategories();

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Source directory selection
    QHBoxLayout* sourceLayout = new QHBoxLayout();
    QLabel* sourceLabel = new QLabel("Source Directory:", this);
    sourceDirEdit = new QLineEdit(this);
    browseSourceBtn = new QPushButton("Browse...", this);
    sourceLayout->addWidget(sourceLabel);
    sourceLayout->addWidget(sourceDirEdit);
    sourceLayout->addWidget(browseSourceBtn);
    mainLayout->addLayout(sourceLayout);

    // Destination directory selection
    QHBoxLayout* destLayout = new QHBoxLayout();
    QLabel* destLabel = new QLabel("Destination Directory:", this);
    destDirEdit = new QLineEdit(this);
    browseDestBtn = new QPushButton("Browse...", this);
    destLayout->addWidget(destLabel);
    destLayout->addWidget(destDirEdit);
    destLayout->addWidget(browseDestBtn);
    mainLayout->addLayout(destLayout);

    // File type selection
    selectFileTypesBtn = new QPushButton("Select file types", this);
    mainLayout->addWidget(selectFileTypesBtn);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    mainLayout->addWidget(progressBar);

    // File list
    fileListWidget = new QListWidget(this);
    mainLayout->addWidget(fileListWidget);

    // Start button
    startBackupBtn = new QPushButton("Start Backup", this);
    mainLayout->addWidget(startBackupBtn);

    // Connect signals and slots
    connect(browseSourceBtn, &QPushButton::clicked, this, &one_step_backup::browseSourceDirectory);
    connect(browseDestBtn, &QPushButton::clicked, this, &one_step_backup::browseDestinationDirectory);
    connect(selectFileTypesBtn, &QPushButton::clicked, this, &one_step_backup::openFileTypeSelection);
    connect(startBackupBtn, &QPushButton::clicked, this, &one_step_backup::startBackup);

    QSet<QString> defaultSelection;

    /* //Select a single category by default
    if (fileTypeCategories.contains("Photos")) {
        for (const QString& ext : fileTypeCategories.value("Photos")) {
            defaultSelection.insert(ext);
        }
    }
    */

    // Select all extensions by default
    if (defaultSelection.isEmpty()) {
        for (auto it = fileTypeCategories.cbegin(); it != fileTypeCategories.cend(); ++it) {
            for (const QString& ext : it.value()) {
                defaultSelection.insert(ext);
            }
        }
    }

    applySelectedExtensions(defaultSelection);
    refreshFileList();
}

one_step_backup::~one_step_backup() = default;

// Opens OS native dialog to select source directory
void one_step_backup::browseSourceDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Source Directory",
        DEFAULT_DIRECTORY,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        sourceDirEdit->setText(dir);
        refreshFileList();
    }
}

// Opens OS native dialog for directory selection
void one_step_backup::browseDestinationDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Destination Directory",
        DEFAULT_DIRECTORY,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        destDirEdit->setText(dir);
    }
}

// Opens the file type selection dialog
void one_step_backup::openFileTypeSelection()
{
    FileTypeSelectionDialog dialog(fileTypeCategories, selectedExtensions, this);
    if (dialog.exec() == QDialog::Accepted) {
        applySelectedExtensions(dialog.selectedExtensions());
        refreshFileList();
    }
}

// Returns true if filePath matches one of the selected extensions
bool one_step_backup::isMediaFile(const QString& filePath) const
{
    if (selectedExtensions.isEmpty()) {
        return false;
    }

    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix.isEmpty()) {
        return false;
    }

    return selectedExtensions.contains("." + suffix);
}

// Recursively finds all media files in the given directory matching the selected extensions
// Returns absolute paths to all files matching selectedExtensions using isMediaFile()
// This function is called every time a source directory is selected or file types are changed
// Current pain point: while scanning large directories, the UI stalls until the scan is complete
QStringList one_step_backup::findMediaFiles(const QString& directory)
{
    QStringList mediaFiles;

    if (selectedExtensions.isEmpty()) {
        return mediaFiles;
    }

    QDirIterator it(directory, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString filePath = it.next();
        if (isMediaFile(filePath)) {
            mediaFiles.append(filePath);
        }
    }

    return mediaFiles;
}

// Copies the given list of files to the destination directory
// Returns true if all files were copied successfully, false if any error occurred
bool one_step_backup::copyFiles(const QStringList& files, const QString& destination)
{
    QDir destDir(destination);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    const int totalFiles = files.size();
    int currentFile = 0;

    for (const QString& filePath : files) {
        QFileInfo fileInfo(filePath);
        QString destPath = destDir.filePath(fileInfo.fileName());

        int counter = 1;
        while (QFile::exists(destPath)) {
            const QString baseName = fileInfo.baseName();
            const QString extension = fileInfo.completeSuffix();
            destPath = destDir.filePath(QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension));
            ++counter;
        }

        if (!QFile::copy(filePath, destPath)) {
            QMessageBox::warning(this, "Error", QString("Failed to copy file: %1").arg(filePath));
            return false;
        }

        ++currentFile;
        updateProgress((currentFile * 100) / totalFiles, QString("Copying: %1").arg(fileInfo.fileName()));
    }

    return true;
}

// Updates the progress bar 
void one_step_backup::updateProgress(int value, const QString& message)
{
    progressBar->setValue(value);
    fileListWidget->addItem(message);
    fileListWidget->scrollToBottom();
    QApplication::processEvents();
}

// Does file selection again and starts the backup process
void one_step_backup::startBackup()
{
    const QString sourceDir = sourceDirEdit->text();
    const QString destDir = destDirEdit->text();

    if (sourceDir.isEmpty() || destDir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select both source and destination directories.");
        return;
    }

    if (selectedExtensions.isEmpty()) {
        QMessageBox::information(this, "Information", "No file types are selected. Please choose file types before starting the backup.");
        return;
    }

    fileListWidget->clear();
    progressBar->setValue(0);
    updateProgress(0, "Searching for matching files...");

    const QStringList mediaFiles = findMediaFiles(sourceDir);
    if (mediaFiles.isEmpty()) {
        QMessageBox::information(this, "Information", "No files matching the selected file types were found in the source directory.");
        return;
    }

    updateProgress(0, QString("Found %1 media files. Starting backup...").arg(mediaFiles.size()));

    if (copyFiles(mediaFiles, destDir)) {
        QMessageBox::information(this, "Success", "Backup completed successfully!");
    }
}

// Initializes the fileTypeCategories map with predefined categories and extensions
void one_step_backup::initializeFileTypeCategories()
{
    fileTypeCategories = {
        {"Photos", {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".webp"}},
        {"Videos", {".mp4", ".avi", ".mov", ".wmv", ".flv", ".mkv", ".webm"}},
        {"Documents", {".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx"}},
        {"Audio", {".mp3", ".wav", ".flac", ".aac", ".ogg"}},
        {"Archives", {".zip", ".rar", ".7z", ".tar", ".gz"}}
    };
}

// Update selectedExtensions; called when window is first created and when "Select file types" dialog is accepted
void one_step_backup::applySelectedExtensions(const QSet<QString>& extensions)
{
    selectedExtensions.clear();
    for (const QString& rawExtension : extensions) {
        QString normalized = rawExtension.trimmed().toLower();
        if (normalized.isEmpty()) {
            continue;
        }
        if (!normalized.startsWith('.')) {
            normalized.prepend('.');
        }
        selectedExtensions.insert(normalized);
    }
}

// Refreshes the file list display based on current source directory and selected extensions
void one_step_backup::refreshFileList()
{
    fileListWidget->clear();

    if (selectedExtensions.isEmpty()) {
        fileListWidget->addItem("No file types selected.");
        return;
    }

    const QString dir = sourceDirEdit->text();
    if (dir.isEmpty()) {
        fileListWidget->addItem("Select a source directory to view matching files.");
        return;
    }

    const QStringList mediaFiles = findMediaFiles(dir);
    if (mediaFiles.isEmpty()) {
        fileListWidget->addItem("No files matching the selected types were found.");
    } else {
        fileListWidget->addItem(QString("Found %1 matching files:").arg(mediaFiles.size()));
        fileListWidget->addItems(mediaFiles);
    }
}
