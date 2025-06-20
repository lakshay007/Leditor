#ifndef CUSTOMTEXTWIDGET_H
#define CUSTOMTEXTWIDGET_H

#include <QWidget>
#include <QString>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <vector>
#include <string>

class CustomTextWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(float scrollOffsetY READ getScrollOffsetY WRITE setScrollOffsetY)

public:
    explicit CustomTextWidget(QWidget *parent = nullptr);

    void loadText(const QString &text);
    QString getText() const;
    void clear();

    int getCurrentLine() const { return cursorY + 1; }
    int getCurrentColumn() const { return cursorX + 1; }
    bool isModified() const { return isDirty; }
    void setModified(bool modified) { isDirty = modified; }

    float getScrollOffsetY() const { return scrollOffsetY; }
    void setScrollOffsetY(float offset);
    void smoothScrollTo(float targetY);

signals:
    void textChanged();
    void cursorPositionChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void blinkCursor();

private:

    std::vector<std::string> buffer;

    int cursorX, cursorY;

    bool isDirty;

    QFont textFont;
    QFontMetrics *fontMetrics;
    int lineHeight;
    int charWidth;

    QTimer *cursorTimer;
    bool cursorVisible;

    int scrollOffsetX;
    float scrollOffsetY;
    float targetScrollY;
    QPropertyAnimation *scrollAnimation;

    void insertChar(char c);
    void deleteChar();
    void insertNewline();
    void moveCursor(int dx, int dy);

    void ensureCursorVisible();
    void updateFontMetrics();
    void emitSignals();
};

#endif 