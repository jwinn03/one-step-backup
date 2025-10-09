// file_type_selection.cpp
// Licensed under Apache 2.0

#include "file_type_selection.h"

#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

FileTypeSelectionDialog::FileTypeSelectionDialog(const QMap<QString, QStringList>& categories,
                                                 const QSet<QString>& selectedExtensions,
                                                 QWidget* parent)
    : QDialog(parent),
      treeWidget(new QTreeWidget(this)),
      categoryMap(categories),
      currentSelection(selectedExtensions),
      updatingTree(false)
{
    setWindowTitle("Select file types");
    setModal(true);

    treeWidget->setColumnCount(1);
    treeWidget->setHeaderHidden(true);
    treeWidget->setUniformRowHeights(true);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(treeWidget);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FileTypeSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FileTypeSelectionDialog::reject);
    connect(treeWidget, &QTreeWidget::itemChanged, this, &FileTypeSelectionDialog::onItemChanged);

    populateTree();
}

QSet<QString> FileTypeSelectionDialog::selectedExtensions() const
{
    return currentSelection;
}

void FileTypeSelectionDialog::accept()
{
    syncSelectionFromTree();
    QDialog::accept();
}

// Handles changes to checkbox states, updating children and parent items whenever change is made to tree widget
// item: changed checkbox
void FileTypeSelectionDialog::onItemChanged(QTreeWidgetItem* item, int /*column*/)
{
    if (updatingTree) {
        return;
    }

    updatingTree = true;

    if (!item->parent()) {
        updateChildrenState(item);
    } else {
        updateParentState(item);
    }

    updatingTree = false;
}

// Populates the tree widget with categories and their associated file extensions
void FileTypeSelectionDialog::populateTree()
{
    updatingTree = true;
    treeWidget->clear();

    for (auto it = categoryMap.cbegin(); it != categoryMap.cend(); ++it) {
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(treeWidget);
        categoryItem->setText(0, it.key());
        // Configure each category's checkbox to be tristate so it can reflect the state of its children (i.e. use "indeterminate" state if only some children are checked)
        categoryItem->setFlags(categoryItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        // This default state may be overriden later (e.g. in one_step_backup constructor)
        categoryItem->setCheckState(0, Qt::Unchecked);

        int checkedCount = 0;
        const QStringList extensions = it.value();
    
        for (const QString& rawExtension : extensions) {
            QString normalizedExtension = rawExtension.trimmed().toLower();
            if (!normalizedExtension.startsWith('.')) {
                normalizedExtension.prepend('.');
            }

            QTreeWidgetItem* extensionItem = new QTreeWidgetItem(categoryItem);
            extensionItem->setText(0, normalizedExtension);
            extensionItem->setFlags(extensionItem->flags() | Qt::ItemIsUserCheckable);
            const bool isChecked = currentSelection.contains(normalizedExtension);
            extensionItem->setCheckState(0, isChecked ? Qt::Checked : Qt::Unchecked);

            if (isChecked) {
                ++checkedCount;
            }
        }

        if (extensions.isEmpty()) {
            categoryItem->setCheckState(0, Qt::Unchecked);
        } else if (checkedCount == 0) {
            categoryItem->setCheckState(0, Qt::Unchecked);
        } else if (checkedCount == extensions.size()) {
            categoryItem->setCheckState(0, Qt::Checked);
        } else {
            categoryItem->setCheckState(0, Qt::PartiallyChecked);
        }
    }

    treeWidget->expandAll();
    updatingTree = false;
}

// Updates all children of the given parent item to match the parent's state
void FileTypeSelectionDialog::updateChildrenState(QTreeWidgetItem* parentItem)
{
    Qt::CheckState state = parentItem->checkState(0);

    // Do not change children if parent category becomes PartiallyChecked (i.e. only some children are checked), so that children do not also become PartiallyChecked
    if (state == Qt::PartiallyChecked) {
        return;
    }

    // Set all children to match the parent's state
    const int childCount = parentItem->childCount();
    for (int i = 0; i < childCount; ++i) {
        QTreeWidgetItem* child = parentItem->child(i);
        child->setCheckState(0, state);
        // Recursively update grandchildren if any exist; not currently used
        /*
        if (child->childCount() > 0) {
            updateChildrenState(child);
        }
        */
    }
}

// Updates the parent item's state based on the states of its children
void FileTypeSelectionDialog::updateParentState(QTreeWidgetItem* childItem)
{
    QTreeWidgetItem* parentItem = childItem->parent();
    if (!parentItem) {
        return;
    }

    int checkedCount = 0;
    const int childCount = parentItem->childCount();

    for (int i = 0; i < childCount; ++i) {
        QTreeWidgetItem* child = parentItem->child(i);
        const Qt::CheckState state = child->checkState(0);
        if (state == Qt::Checked) {
            ++checkedCount;
        } else if (state == Qt::Unchecked) {
            continue;
        } else {
            parentItem->setCheckState(0, Qt::PartiallyChecked);
            updateParentState(parentItem);
            return;
        }
    }

    if (checkedCount == childCount) {
        parentItem->setCheckState(0, Qt::Checked);
    } else if (checkedCount == 0) {
        parentItem->setCheckState(0, Qt::Unchecked);
    } else {
        parentItem->setCheckState(0, Qt::PartiallyChecked);
    }

    // Recursively update parent's parent if any exist; not currently used
    // updateParentState(parentItem);
}

void FileTypeSelectionDialog::syncSelectionFromTree()
{
    currentSelection = collectSelections(nullptr);
}

// Recursively collects all checked extensions starting from the given parent item; if parentItem is nullptr, starts from the root of the tree
QSet<QString> FileTypeSelectionDialog::collectSelections(QTreeWidgetItem* parentItem) const
{
    QSet<QString> selection;
    if (parentItem) {
        const int childCount = parentItem->childCount();
        for (int i = 0; i < childCount; ++i) {
            QTreeWidgetItem* child = parentItem->child(i);
            if (child->childCount() == 0) {
                if (child->checkState(0) == Qt::Checked) {
                    selection.insert(child->text(0).trimmed().toLower());
                }
            } else {
                selection.unite(collectSelections(child));
            }
        }
    } else {
        const int topLevelCount = treeWidget->topLevelItemCount();
        for (int i = 0; i < topLevelCount; ++i) {
            selection.unite(collectSelections(treeWidget->topLevelItem(i)));
        }
    }
    return selection;
}
