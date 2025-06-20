#include "editortabs.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>

EditorTabs::EditorTabs(QWidget *parent)
    : QTabWidget(parent)
    , untitledCounter(1)
{
    setupUI();

    connect(this, &QTabWidget::tabCloseRequested, this, &EditorTabs::onTabCloseRequested);
    connect(this, &QTabWidget::currentChanged, this, &EditorTabs::onCurrentChanged);
}

void EditorTabs::setupUI()
{
    setTabsClosable(false);  
    setMovable(true);
    setDocumentMode(true); 

    setStyleSheet(R"(
        QTabWidget::pane {
            border: none;
            background-color: #1e1e1e;
        }
        QTabBar::tab {
            background-color: #2d2d30;
            color: #ffffff;
            padding: 8px 16px;
            margin-right: 2px;
            border-top: 2px solid transparent;
            min-width: 100px;
        }
        QTabBar::tab:selected {
            background-color: #1e1e1e;
            border-top: 2px solid #0078d4;
        }
        QTabBar::tab:hover {
            background-color: #3e3e42;
        }

    )");

    newFile();
}

void EditorTabs::openFile(const QString &filePath)
{

    for (auto it = tabFilePaths.begin(); it != tabFilePaths.end(); ++it) {
        if (it.value() == filePath) {
            setCurrentIndex(it.key());
            return;
        }
    }

    CustomTextWidget *editor = new CustomTextWidget(this);

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        editor->loadText(content);
        file.close();
    }

    int tabIndex = addTab(editor, getDisplayName(filePath));
    tabFilePaths[tabIndex] = filePath;
    addCustomCloseButton(tabIndex);  
    setCurrentIndex(tabIndex);

    connect(editor, &CustomTextWidget::textChanged, this, &EditorTabs::onTextChanged);
    connect(editor, &CustomTextWidget::cursorPositionChanged, [this, editor]() {
        emit editorFocusChanged(editor);
    });

    emit fileOpened(filePath);
}

void EditorTabs::newFile()
{
    CustomTextWidget *editor = new CustomTextWidget(this);

    QString fileName = QString("Untitled %1").arg(untitledCounter++);
    int tabIndex = addTab(editor, fileName);
    tabFilePaths[tabIndex] = ""; 
    addCustomCloseButton(tabIndex);  
    setCurrentIndex(tabIndex);

    connect(editor, &CustomTextWidget::textChanged, this, &EditorTabs::onTextChanged);
    connect(editor, &CustomTextWidget::cursorPositionChanged, [this, editor]() {
        emit editorFocusChanged(editor);
    });
}

bool EditorTabs::closeFile(int index)
{
    if (index < 0 || index >= count()) return false;

    CustomTextWidget *editor = getEditorAt(index);
    if (editor && editor->isModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Save Changes",
            QString("Do you want to save changes to '%1'?").arg(tabText(index)),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            if (!saveCurrentFile()) return false;
        } else if (reply == QMessageBox::Cancel) {
            return false;
        }
    }

    QString filePath = tabFilePaths.value(index);
    tabFilePaths.remove(index);

    QMap<int, QString> newMap;
    for (auto it = tabFilePaths.begin(); it != tabFilePaths.end(); ++it) {
        int newIndex = it.key() > index ? it.key() - 1 : it.key();
        newMap[newIndex] = it.value();
    }
    tabFilePaths = newMap;

    removeTab(index);

    if (!filePath.isEmpty()) {
        emit fileClosed(filePath);
    }

    if (count() == 0) {
        newFile();
    }

    return true;
}

bool EditorTabs::closeCurrentFile()
{
    return closeFile(currentIndex());
}

bool EditorTabs::saveCurrentFile()
{
    int index = currentIndex();
    if (index < 0) return false;

    QString filePath = tabFilePaths.value(index);
    if (filePath.isEmpty()) {
        return saveFileAs(index);
    }

    CustomTextWidget *editor = getEditorAt(index);
    if (!editor) return false;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << editor->getText();
        file.close();

        editor->setModified(false);
        updateTabTitle(index);
        return true;
    }

    return false;
}

bool EditorTabs::saveFileAs(int index)
{
    if (index < 0 || index >= count()) return false;

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save File As",
        "",
        "Text Files (*.txt);;All Files (*)"
    );

    if (filePath.isEmpty()) return false;

    CustomTextWidget *editor = getEditorAt(index);
    if (!editor) return false;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << editor->getText();
        file.close();

        tabFilePaths[index] = filePath;
        setTabText(index, getDisplayName(filePath));
        editor->setModified(false);
        updateTabTitle(index);
        return true;
    }

    return false;
}

CustomTextWidget* EditorTabs::getCurrentEditor()
{
    return getEditorAt(currentIndex());
}

CustomTextWidget* EditorTabs::getEditorAt(int index)
{
    if (index < 0 || index >= count()) return nullptr;
    return qobject_cast<CustomTextWidget*>(widget(index));
}

QString EditorTabs::getFilePathAt(int index)
{
    return tabFilePaths.value(index);
}

QString EditorTabs::getCurrentFilePath()
{
    return getFilePathAt(currentIndex());
}

bool EditorTabs::hasUnsavedChanges()
{
    for (int i = 0; i < count(); ++i) {
        CustomTextWidget *editor = getEditorAt(i);
        if (editor && editor->isModified()) {
            return true;
        }
    }
    return false;
}

void EditorTabs::onTabCloseRequested(int index)
{
    closeFile(index);
}

void EditorTabs::onCurrentChanged(int index)
{
    if (index >= 0) {
        QString filePath = tabFilePaths.value(index);
        emit activeFileChanged(filePath);

        CustomTextWidget *editor = getEditorAt(index);
        if (editor) {
            emit editorFocusChanged(editor);
        }
    }
}

void EditorTabs::onTextChanged()
{
    CustomTextWidget *editor = qobject_cast<CustomTextWidget*>(sender());
    if (editor) {

        for (int i = 0; i < count(); ++i) {
            if (getEditorAt(i) == editor) {
                updateTabTitle(i);
                break;
            }
        }
    }
}

void EditorTabs::updateTabTitle(int index)
{
    CustomTextWidget *editor = getEditorAt(index);
    if (!editor) return;

    QString baseName = tabText(index);
    if (baseName.endsWith(" *")) {
        baseName = baseName.left(baseName.length() - 2);
    }

    if (editor->isModified()) {
        setTabText(index, baseName + " *");
    } else {
        setTabText(index, baseName);
    }
}

QString EditorTabs::getDisplayName(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return QString("Untitled %1").arg(untitledCounter);
    }
    return QFileInfo(filePath).fileName();
}

void EditorTabs::addCustomCloseButton(int tabIndex)
{
    QPushButton *closeButton = new QPushButton("×", this);
    closeButton->setFixedSize(20, 20);
    closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 3px;
            color: #969696;
            font: bold 14px;
            padding: 0px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.1);
            color: #ffffff;
        }
        QPushButton:pressed {
            background-color: rgba(232, 17, 35, 0.9);
            color: white;
        }
    )");

    connect(closeButton, &QPushButton::clicked, [this, tabIndex]() {
        closeFile(tabIndex);
    });

    tabBar()->setTabButton(tabIndex, QTabBar::RightSide, closeButton);
}