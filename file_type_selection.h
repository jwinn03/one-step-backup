// file_type_selection.h
// Licensed under Apache 2.0

#pragma once

#include <QDialog>
#include <QMap>
#include <QSet>
#include <QStringList>

class QTreeWidget;
class QTreeWidgetItem;

class FileTypeSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    FileTypeSelectionDialog(const QMap<QString, QStringList>& categories,
                            const QSet<QString>& selectedExtensions,
                            QWidget* parent = nullptr);

    QSet<QString> selectedExtensions() const;
    void accept() override;

private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);

private:
    void populateTree();
    void updateChildrenState(QTreeWidgetItem* parentItem);
    void updateParentState(QTreeWidgetItem* childItem);
    void syncSelectionFromTree();
    QSet<QString> collectSelections(QTreeWidgetItem* parentItem) const;

    QTreeWidget* treeWidget;
    QMap<QString, QStringList> categoryMap;
    QSet<QString> currentSelection;
    bool updatingTree;
};
