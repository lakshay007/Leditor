#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>

class FileExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit FileExplorer(QWidget *parent = nullptr);
    
    void setWorkspaceRoot(const QString &path);
    QString getCurrentWorkspace() const { return workspacePath; }

signals:
    void fileSelected(const QString &filePath);
    void workspaceChanged(const QString &workspacePath);

private slots:
    void onOpenWorkspace();
    void onItemClicked(const QModelIndex &index);
    void onItemDoubleClicked(const QModelIndex &index);

private:
    void setupUI();
    void setupFileSystemModel();
    
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QLabel *workspaceLabel;
    QPushButton *openWorkspaceBtn;
    QTreeView *fileTreeView;
    QFileSystemModel *fileSystemModel;
    
    QString workspacePath;
};

#endif // FILEEXPLORER_H 