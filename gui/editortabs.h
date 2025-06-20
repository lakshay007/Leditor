#ifndef EDITORTABS_H
#define EDITORTABS_H

#include <QTabWidget>
#include <QTabBar>
#include <QMap>
#include <QString>
#include "customtextwidget.h"

class EditorTabs : public QTabWidget
{
    Q_OBJECT

public:
    explicit EditorTabs(QWidget *parent = nullptr);

    void openFile(const QString &filePath);
    void newFile();
    bool closeFile(int index);
    bool closeCurrentFile();
    bool saveCurrentFile();
    bool saveFileAs(int index);

    CustomTextWidget* getCurrentEditor();
    CustomTextWidget* getEditorAt(int index);
    QString getFilePathAt(int index);
    QString getCurrentFilePath();
    bool hasUnsavedChanges();

signals:
    void fileOpened(const QString &filePath);
    void fileClosed(const QString &filePath);
    void activeFileChanged(const QString &filePath);
    void editorFocusChanged(CustomTextWidget *editor);

private slots:
    void onTabCloseRequested(int index);
    void onCurrentChanged(int index);
    void onTextChanged();

private:
    void setupUI();
    void updateTabTitle(int index);
    QString getDisplayName(const QString &filePath);
    void addCustomCloseButton(int tabIndex);

    QMap<int, QString> tabFilePaths;
    int untitledCounter;
};

#endif 