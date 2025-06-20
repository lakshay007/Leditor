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
    , textEdit(new CustomTextWidget)
    , currentFile("")
{
    setCentralWidget(textEdit);
    
    createMenus();
    createStatusBar();
    readSettings();
    
    // Connect text editor signals
    connect(textEdit, &CustomTextWidget::textChanged,
            this, &MainWindow::documentWasModified);
    connect(textEdit, &CustomTextWidget::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
    
    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
    
    // Initial status update
    updateStatusBar();
}

MainWindow::~MainWindow()
{
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
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open File"),
            "",
            tr("Text Files (*.txt);;All Files (*)")
        );
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

void MainWindow::openFile(const QString &fileName)
{
    if (maybeSave()) {
        loadFile(fileName);
    }
}

bool MainWindow::saveFile()
{
    if (currentFile.isEmpty()) {
        return saveAsFile();
    } else {
        return saveFileAs(currentFile);
    }
}

bool MainWindow::saveAsFile()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Text Files (*.txt);;All Files (*)"));
    dialog.setDefaultSuffix("txt");
    
    if (dialog.exec() != QDialog::Accepted)
        return false;
    
    return saveFileAs(dialog.selectedFiles().first());
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

void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->isModified());
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    QString status;
    
    if (currentFile.isEmpty()) {
        status = tr("Untitled");
    } else {
        status = strippedName(currentFile);
    }
    
    if (textEdit->isModified()) {
        status += tr(" [Modified]");
    }
    
    // Ensure minimum width and clean text
    statusLabel->setText(status);
    
    // Update cursor position with better formatting
    int line = textEdit->getCurrentLine();
    int col = textEdit->getCurrentColumn();
    positionLabel->setText(tr("Ln %1, Col %2").arg(line).arg(col));
}

void MainWindow::createMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);
    
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, QOverload<>::of(&MainWindow::openFile));
    fileMenu->addAction(openAct);
    
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
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAct);
}

void MainWindow::createStatusBar()
{
    statusLabel = new QLabel(tr("Ready"));
    statusLabel->setMinimumWidth(200); // Give it some minimum width
    statusBar()->addWidget(statusLabel, 1); // Give it stretch factor
    
    // Add a separator (optional)
    QLabel *separator = new QLabel(" | ");
    statusBar()->addPermanentWidget(separator);
    
    positionLabel = new QLabel(tr("Line 1, Col 1"));
    positionLabel->setMinimumWidth(100); // Fixed width for position
    statusBar()->addPermanentWidget(positionLabel);
}

void MainWindow::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(800, 600)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

bool MainWindow::maybeSave()
{
    if (!textEdit->isModified())
        return true;
    
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Leditor"),
                              tr("The document has been modified.\n"
                                 "Do you want to save your changes?"),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Leditor"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return;
    }
    
    QTextStream in(&file);
    
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->loadText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    
    setCurrentFile(fileName);
}

bool MainWindow::saveFileAs(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Leditor"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }
    
    QTextStream out(&file);
    
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << textEdit->getText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    
    setCurrentFile(fileName);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    textEdit->setModified(false);
    setWindowModified(false);
    
    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
    updateStatusBar();
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
} 