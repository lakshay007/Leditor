#ifndef CUSTOMTEXTWIDGET_H
#define CUSTOMTEXTWIDGET_H

#include <QWidget>
#include <QString>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <vector>
#include <string>

class CustomTextWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTextWidget(QWidget *parent = nullptr);
    
    // File operations
    void loadText(const QString &text);
    QString getText() const;
    void clear();
    
    // Status information
    int getCurrentLine() const { return cursorY + 1; }
    int getCurrentColumn() const { return cursorX + 1; }
    bool isModified() const { return isDirty; }
    void setModified(bool modified) { isDirty = modified; }

signals:
    void textChanged();
    void cursorPositionChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void blinkCursor();

private:
    
    std::vector<std::string> buffer;
    
    
    int cursorX, cursorY;
    
    // Modified flag
    bool isDirty;
    
    // Display properties
    QFont textFont;
    QFontMetrics *fontMetrics;
    int lineHeight;
    int charWidth;
    
    // Cursor blinking
    QTimer *cursorTimer;
    bool cursorVisible;
    
    // Scrolling
    int scrollOffsetX, scrollOffsetY;
    
    //editing functions
    void insertChar(char c);
    void deleteChar();
    void insertNewline();
    void moveCursor(int dx, int dy);
    
    // Helper functions
    void ensureCursorVisible();
    void updateFontMetrics();
    void emitSignals();
};

#endif // CUSTOMTEXTWIDGET_H 