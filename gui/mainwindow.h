#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QKeySequence>
#include <QTextStream>
#include <QDir>
#include <QOverload>
#include <QSplitter>
#include "customtextwidget.h"
#include "fileexplorer.h"
#include "editortabs.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void openFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void openWorkspace();
    bool saveFile();
    bool saveAsFile();
    void about();
    void toggleSidebar();
    void onFileSelected(const QString &filePath);
    void onActiveFileChanged(const QString &filePath);
    void onEditorFocusChanged(CustomTextWidget *editor);
    void updateStatusBar();

private:
    void setupUI();
    void createMenus();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    QString strippedName(const QString &fullFileName);

    // UI Components
    QSplitter *mainSplitter;
    FileExplorer *fileExplorer;
    EditorTabs *editorTabs;
    
    // Status bar
    QLabel *statusLabel;
    QLabel *positionLabel;
    
    // Current state
    QString currentWorkspace;
    
    // Actions
    QAction *newAct;
    QAction *openAct;
    QAction *openWorkspaceAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *toggleSidebarAct;
};

#endif // MAINWINDOW_H 