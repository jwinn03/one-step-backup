#include "one_step_backup.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QFile>
#include <QDebug>

one_step_backup::one_step_backup(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowTitle("Media File Backup Tool");

    // Create central widget and layout
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
    connect(startBackupBtn, &QPushButton::clicked, this, &one_step_backup::startBackup);
}

one_step_backup::~one_step_backup()
{
}

void one_step_backup::browseSourceDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Source Directory",
        QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        sourceDirEdit->setText(dir);

        fileListWidget->clear();
        QStringList mediaFiles = findMediaFiles(dir);

        if (mediaFiles.isEmpty()) {
            fileListWidget->addItem("No media files found in the selected directory.");
        } else {
            fileListWidget->addItem(QString("Found %1 media files:").arg(mediaFiles.size()));
            fileListWidget->addItems(mediaFiles);
        }
    }
}

void one_step_backup::browseDestinationDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Destination Directory",
        QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        destDirEdit->setText(dir);
    }
}

bool one_step_backup::isMediaFile(const QString& filePath)
{
    QStringList mediaExtensions = {
        // Image formats
        ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".webp",
        // Video formats
        ".mp4", ".avi", ".mov", ".wmv", ".flv", ".mkv", ".webm"
    };

    QString extension = QFileInfo(filePath).suffix().toLower();
    return mediaExtensions.contains("." + extension);
}

QStringList one_step_backup::findMediaFiles(const QString& directory)
{
    QStringList mediaFiles;
    QDirIterator it(directory, QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        if (isMediaFile(filePath)) {
            mediaFiles.append(filePath);
        }
    }
    
    return mediaFiles;
}

bool one_step_backup::copyFiles(const QStringList& files, const QString& destination)
{
    QDir destDir(destination);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    int totalFiles = files.size();
    int currentFile = 0;

    for (const QString& filePath : files) {
        QFileInfo fileInfo(filePath);
        QString destPath = destDir.filePath(fileInfo.fileName());
        
        // Handle duplicate filenames
        int counter = 1;
        while (QFile::exists(destPath)) {
            QString baseName = fileInfo.baseName();
            QString extension = fileInfo.completeSuffix();
            destPath = destDir.filePath(QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension));
            counter++;
        }

        if (!QFile::copy(filePath, destPath)) {
            QMessageBox::warning(this, "Error", 
                QString("Failed to copy file: %1").arg(filePath));
            return false;
        }

        currentFile++;
        updateProgress((currentFile * 100) / totalFiles, 
            QString("Copying: %1").arg(fileInfo.fileName()));
    }

    return true;
}

void one_step_backup::updateProgress(int value, const QString& message)
{
    progressBar->setValue(value);
    fileListWidget->addItem(message);
    fileListWidget->scrollToBottom();
    QApplication::processEvents();
}

void one_step_backup::startBackup()
{
    QString sourceDir = sourceDirEdit->text();
    QString destDir = destDirEdit->text();

    if (sourceDir.isEmpty() || destDir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select both source and destination directories.");
        return;
    }

    fileListWidget->clear();
    progressBar->setValue(0);
    updateProgress(0, "Searching for media files...");

    QStringList mediaFiles = findMediaFiles(sourceDir);
    if (mediaFiles.isEmpty()) {
        QMessageBox::information(this, "Information", "No media files found in the source directory.");
        return;
    }

    updateProgress(0, QString("Found %1 media files. Starting backup...").arg(mediaFiles.size()));
    
    if (copyFiles(mediaFiles, destDir)) {
        QMessageBox::information(this, "Success", "Backup completed successfully!");
    }
}
