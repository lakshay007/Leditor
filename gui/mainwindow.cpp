#include "mainwindow.h"
#include <QCloseEvent>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    createMenus();
    createStatusBar();
    readSettings();

    connect(fileExplorer, &FileExplorer::fileSelected,
            this, &MainWindow::onFileSelected);
    connect(editorTabs, &EditorTabs::activeFileChanged,
            this, &MainWindow::onActiveFileChanged);
    connect(editorTabs, &EditorTabs::editorFocusChanged,
            this, &MainWindow::onEditorFocusChanged);

    setUnifiedTitleAndToolBarOnMac(true);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
            color: #ffffff;
        }
        QMenuBar {
            background-color: #2d2d30;
            color: #ffffff;
            border: none;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 6px 12px;
        }
        QMenuBar::item:selected {
            background-color: #3e3e42;
        }
        QMenu {
            background-color: #2d2d30;
            color: #ffffff;
            border: 1px solid #555555;
        }
        QMenu::item {
            padding: 6px 20px;
        }
        QMenu::item:selected {
            background-color: #0078d4;
        }
        QSplitter::handle {
            background-color: #2d2d30;
        }
        QSplitter::handle:horizontal {
            width: 1px;
            background-color: #2d2d30;
            border: none;
            margin: 0px;
        }
        QSplitter::handle:horizontal:hover {
            background-color: #007acc;
        }
    )");

    updateStatusBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{

    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);

    fileExplorer = new FileExplorer(this);
    fileExplorer->setMinimumWidth(120);  
    fileExplorer->setMaximumWidth(800);  

    editorTabs = new EditorTabs(this);

    mainSplitter->addWidget(fileExplorer);
    mainSplitter->addWidget(editorTabs);

    mainSplitter->setSizes({250, 800});
    mainSplitter->setCollapsible(0, true);  
    mainSplitter->setCollapsible(1, false); 

    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    mainSplitter->setChildrenCollapsible(true);
    mainSplitter->setHandleWidth(1);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    editorTabs->newFile();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        currentWorkspace.isEmpty() ? QDir::homePath() : currentWorkspace,
        tr("Text Files (*.txt);;All Files (*)")
    );
    if (!fileName.isEmpty()) {
        editorTabs->openFile(fileName);
    }
}

void MainWindow::openFile(const QString &fileName)
{
    editorTabs->openFile(fileName);
}

void MainWindow::openWorkspace()
{
    QString selectedPath = QFileDialog::getExistingDirectory(
        this,
        "Select Workspace Folder",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!selectedPath.isEmpty()) {
        currentWorkspace = selectedPath;
        fileExplorer->setWorkspaceRoot(selectedPath);
    }
}

bool MainWindow::saveFile()
{
    return editorTabs->saveCurrentFile();
}

bool MainWindow::saveAsFile()
{
    return editorTabs->saveFileAs(editorTabs->currentIndex());
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Leditor"),
                       tr("Leditor is a simple text editor built with Qt.\n\n"
                          "Features:\n"
                          "• Basic text editing\n"
                          "• File operations (New, Open, Save)\n"
                          "• Normal cursor and text selection\n"
                          "• Status bar with cursor position"));
}

void MainWindow::toggleSidebar()
{
    if (fileExplorer->isVisible()) {
        fileExplorer->hide();
    } else {
        fileExplorer->show();
    }
}

void MainWindow::onFileSelected(const QString &filePath)
{
    editorTabs->openFile(filePath);
}

void MainWindow::onActiveFileChanged(const QString &filePath)
{
    QString displayName = filePath.isEmpty() ? "Untitled" : strippedName(filePath);
    setWindowTitle(QString("Leditor - %1").arg(displayName));
    updateStatusBar();
}

void MainWindow::onEditorFocusChanged(CustomTextWidget *editor)
{
    Q_UNUSED(editor)
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    CustomTextWidget *currentEditor = editorTabs->getCurrentEditor();
    if (!currentEditor) {
        statusLabel->setText("Ready");
        positionLabel->setText("");
        return;
    }

    QString filePath = editorTabs->getCurrentFilePath();
    QString status = filePath.isEmpty() ? "Untitled" : strippedName(filePath);

    if (currentEditor->isModified()) {
        status += " [Modified]";
    }

    statusLabel->setText(status);

    int line = currentEditor->getCurrentLine();
    int col = currentEditor->getCurrentColumn();
    positionLabel->setText(tr("Ln %1, Col %2").arg(line).arg(col));
}

void MainWindow::createMenus()
{

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);

    openAct = new QAction(tr("&Open File..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, QOverload<>::of(&MainWindow::openFile));
    fileMenu->addAction(openAct);

    openWorkspaceAct = new QAction(tr("Open &Workspace..."), this);
    openWorkspaceAct->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    openWorkspaceAct->setStatusTip(tr("Open a folder as workspace"));
    connect(openWorkspaceAct, &QAction::triggered, this, &MainWindow::openWorkspace);
    fileMenu->addAction(openWorkspaceAct);

    fileMenu->addSeparator();

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(saveAct);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFile);
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAct);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    toggleSidebarAct = new QAction(tr("Toggle &Sidebar"), this);
    toggleSidebarAct->setShortcut(QKeySequence(tr("Ctrl+B")));
    toggleSidebarAct->setStatusTip(tr("Show or hide the file explorer sidebar"));
    connect(toggleSidebarAct, &QAction::triggered, this, &MainWindow::toggleSidebar);
    viewMenu->addAction(toggleSidebarAct);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAct);
}

void MainWindow::createStatusBar()
{
    statusLabel = new QLabel(tr("Ready"));
    statusLabel->setMinimumWidth(200);
    statusBar()->addWidget(statusLabel, 1);

    QLabel *separator = new QLabel(" | ");
    statusBar()->addPermanentWidget(separator);

    positionLabel = new QLabel(tr("Ln 1, Col 1"));
    positionLabel->setMinimumWidth(100);
    statusBar()->addPermanentWidget(positionLabel);

    statusBar()->setStyleSheet(R"(
        QStatusBar {
            background-color: #2d2d30;
            color: #ffffff;
            border-top: 1px solid #555555;
        }
        QLabel {
            color: #ffffff;
            padding: 2px 8px;
        }
    )");
}

void MainWindow::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(800, 600)).toSize();
    move(pos);
    resize(size);

    QByteArray splitterState = settings.value("splitterState").toByteArray();
    if (!splitterState.isEmpty()) {
        mainSplitter->restoreState(splitterState);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    settings.setValue("splitterState", mainSplitter->saveState());
}

bool MainWindow::maybeSave()
{
    if (!editorTabs->hasUnsavedChanges())
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Leditor"),
                              tr("Some files have unsaved changes.\n"
                                 "Do you want to save them before closing?"),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:

        for (int i = 0; i < editorTabs->count(); ++i) {
            CustomTextWidget *editor = editorTabs->getEditorAt(i);
            if (editor && editor->isModified()) {
                editorTabs->setCurrentIndex(i);
                if (!saveFile()) return false;
            }
        }
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}