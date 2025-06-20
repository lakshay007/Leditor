#include "fileexplorer.h"
#include <QHeaderView>
#include <QApplication>
#include <QFileInfo>

FileExplorer::FileExplorer(QWidget *parent)
    : QWidget(parent)
    , fileSystemModel(nullptr)
{
    setupUI();
    setupFileSystemModel();

    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            border: none;
        }
        QLabel {
            color: #ffffff;
            font-weight: bold;
            padding: 8px;
            font-size: 12px;
        }
        QPushButton {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #555555;
            padding: 6px 12px;
            border-radius: 4px;
            font-size: 11px;
        }
        QPushButton:hover {
            background-color: #505050;
            border-color: #666666;
        }
        QPushButton:pressed {
            background-color: #353535;
        }
        QTreeView {
            background-color: #2b2b2b;
            color: #ffffff;
            border: none;
            outline: none;
            font-size: 12px;
        }
        QTreeView::item {
            padding: 4px;
            border: none;
        }
        QTreeView::item:hover {
            background-color: #404040;
        }
        QTreeView::item:selected {
            background-color: #0078d4;
        }
        QTreeView::branch:has-children:!has-siblings:closed,
        QTreeView::branch:closed:has-children:has-siblings {
            border-image: none;
            background: none;
        }
        QTreeView::branch:open:has-children:!has-siblings,
        QTreeView::branch:open:has-children:has-siblings {
            border-image: none;
            background: none;
        }
        QTreeView::branch:has-children:closed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0.3 transparent, stop:0.5 #666666, stop:0.7 transparent);
        }
        QTreeView::branch:has-children:open {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0.3 transparent, stop:0.5 #666666, stop:0.7 transparent);
        }
    )");
}

void FileExplorer::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(8, 8, 8, 8);

    workspaceLabel = new QLabel("No Workspace");
    workspaceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    openWorkspaceBtn = new QPushButton("Open");
    openWorkspaceBtn->setFixedSize(60, 28);

    headerLayout->addWidget(workspaceLabel);
    headerLayout->addWidget(openWorkspaceBtn);

    fileTreeView = new QTreeView();
    fileTreeView->setHeaderHidden(true);
    fileTreeView->setRootIsDecorated(true);
    fileTreeView->setAlternatingRowColors(false);
    fileTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    fileTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileTreeView->setExpandsOnDoubleClick(true);
    fileTreeView->setItemsExpandable(true);
    fileTreeView->setAutoExpandDelay(500);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(fileTreeView);

    connect(openWorkspaceBtn, &QPushButton::clicked, this, &FileExplorer::onOpenWorkspace);
    connect(fileTreeView, &QTreeView::doubleClicked, this, &FileExplorer::onItemDoubleClicked);
    connect(fileTreeView, &QTreeView::clicked, this, &FileExplorer::onItemClicked);
}

void FileExplorer::setupFileSystemModel()
{
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath("");

    fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    fileTreeView->setModel(fileSystemModel);

    fileTreeView->hideColumn(1);
    fileTreeView->hideColumn(2);
    fileTreeView->hideColumn(3);
}

void FileExplorer::setWorkspaceRoot(const QString &path)
{
    workspacePath = path;

    if (!path.isEmpty()) {
        QDir dir(path);
        workspaceLabel->setText(dir.dirName());

        QModelIndex rootIndex = fileSystemModel->setRootPath(path);
        fileTreeView->setRootIndex(rootIndex);
        fileTreeView->expandToDepth(0); 

        emit workspaceChanged(path);
    } else {
        workspaceLabel->setText("No Workspace");
        fileTreeView->setRootIndex(QModelIndex());
    }
}

void FileExplorer::onOpenWorkspace()
{
    QString selectedPath = QFileDialog::getExistingDirectory(
        this,
        "Select Workspace Folder",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!selectedPath.isEmpty()) {
        setWorkspaceRoot(selectedPath);
    }
}

void FileExplorer::onItemClicked(const QModelIndex &index)
{
    if (fileSystemModel->isDir(index)) {

        if (fileTreeView->isExpanded(index)) {
            fileTreeView->collapse(index);
        } else {
            fileTreeView->expand(index);
        }
    }
}

void FileExplorer::onItemDoubleClicked(const QModelIndex &index)
{
    if (!fileSystemModel->isDir(index)) {

        QString filePath = fileSystemModel->filePath(index);
        emit fileSelected(filePath);
    }

}