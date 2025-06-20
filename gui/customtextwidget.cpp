#include "customtextwidget.h"
#include <QPaintEvent>
#include <QApplication>
#include <QTextStream>
#include <algorithm>

CustomTextWidget::CustomTextWidget(QWidget *parent)
    : QWidget(parent)
    , cursorX(0)
    , cursorY(0)
    , isDirty(false)
    , textFont("Monaco", 12)
    , fontMetrics(nullptr)
    , scrollOffsetX(0)
    , scrollOffsetY(0)
    , cursorVisible(true)
{

    buffer.push_back("");

    textFont.setFixedPitch(true);
    setFont(textFont);
    updateFontMetrics();

    cursorTimer = new QTimer(this);
    connect(cursorTimer, &QTimer::timeout, this, &CustomTextWidget::blinkCursor);
    cursorTimer->start(500); 

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::black);
    pal.setColor(QPalette::Text, Qt::white);
    setPalette(pal);
}

void CustomTextWidget::updateFontMetrics()
{
    if (fontMetrics) delete fontMetrics;
    fontMetrics = new QFontMetrics(textFont);
    lineHeight = fontMetrics->height();
    charWidth = fontMetrics->horizontalAdvance(' '); 
}

void CustomTextWidget::loadText(const QString &text)
{
    buffer.clear();
    QTextStream stream(const_cast<QString*>(&text));

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        buffer.push_back(line.toStdString());
    }

    if (buffer.empty()) {
        buffer.push_back("");
    }

    cursorX = 0;
    cursorY = 0;
    isDirty = false;

    update();
    emitSignals();
}

QString CustomTextWidget::getText() const
{
    QString result;
    for (size_t i = 0; i < buffer.size(); ++i) {
        result += QString::fromStdString(buffer[i]);
        if (i < buffer.size() - 1) {
            result += '\n';
        }
    }
    return result;
}

void CustomTextWidget::clear()
{
    buffer.clear();
    buffer.push_back("");
    cursorX = 0;
    cursorY = 0;
    isDirty = false;
    update();
    emitSignals();
}

void CustomTextWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setFont(textFont);

    painter.fillRect(rect(), Qt::black);

    int startLine = std::max(0, scrollOffsetY);
    int endLine = std::min((int)buffer.size(), scrollOffsetY + height() / lineHeight + 1);

    for (int i = startLine; i < endLine; ++i) {
        int y = (i - scrollOffsetY) * lineHeight + fontMetrics->ascent() + 5;

        if (y > height()) break;

        QString line = QString::fromStdString(buffer[i]);
        painter.setPen(Qt::white);
        painter.drawText(5 - scrollOffsetX, y, line);
    }

    if (hasFocus() && cursorVisible) {

        int cursorScreenX = 5 - scrollOffsetX;
        if (cursorY < (int)buffer.size() && cursorX > 0) {
            QString lineUpToCursor = QString::fromStdString(buffer[cursorY].substr(0, cursorX));
            cursorScreenX += fontMetrics->horizontalAdvance(lineUpToCursor);
        }
        int cursorScreenY = (cursorY - scrollOffsetY) * lineHeight + 5;

        if (cursorScreenX >= 0 && cursorScreenX < width() && 
            cursorScreenY >= 0 && cursorScreenY < height()) {
            painter.setPen(QPen(Qt::white, 2));
            painter.drawLine(cursorScreenX, cursorScreenY, 
                           cursorScreenX, cursorScreenY + lineHeight - 2);
        }
    }
}

void CustomTextWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Left:
            moveCursor(-1, 0);
            break;

        case Qt::Key_Right:
            moveCursor(1, 0);
            break;

        case Qt::Key_Up:
            moveCursor(0, -1);
            break;

        case Qt::Key_Down:
            moveCursor(0, 1);
            break;

        case Qt::Key_Backspace:
            deleteChar();
            break;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            insertNewline();
            break;

        default:
            if (!event->text().isEmpty()) {
                char c = event->text().at(0).toLatin1();
                if (c >= 32 && c < 127) { 
                    insertChar(c);
                }
            }
            break;
    }

    ensureCursorVisible();
    update();
}

void CustomTextWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {

        int clickX = event->pos().x() + scrollOffsetX - 5;
        int clickY = (event->pos().y() + scrollOffsetY * lineHeight - 5) / lineHeight;

        cursorY = std::max(0, std::min(clickY, (int)buffer.size() - 1));

        if (cursorY < (int)buffer.size()) {
            cursorX = 0;
            QString line = QString::fromStdString(buffer[cursorY]);

            for (int i = 0; i <= line.length(); ++i) {
                QString lineUpToPos = line.left(i);
                int textWidth = fontMetrics->horizontalAdvance(lineUpToPos);

                if (textWidth > clickX) {

                    if (i > 0) {
                        QString lineUpToPrev = line.left(i - 1);
                        int prevWidth = fontMetrics->horizontalAdvance(lineUpToPrev);
                        if (clickX - prevWidth < textWidth - clickX) {
                            cursorX = i - 1;
                        } else {
                            cursorX = i;
                        }
                    } else {
                        cursorX = 0;
                    }
                    break;
                }
                cursorX = i;
            }
        }

        update();
        emitSignals();
    }
    setFocus();
}

void CustomTextWidget::focusInEvent(QFocusEvent *event)
{
    cursorTimer->start(500);
    cursorVisible = true;
    update();
    QWidget::focusInEvent(event);
}

void CustomTextWidget::focusOutEvent(QFocusEvent *event)
{
    cursorTimer->stop();
    cursorVisible = false;
    update();
    QWidget::focusOutEvent(event);
}

void CustomTextWidget::blinkCursor()
{
    cursorVisible = !cursorVisible;
    update();
}

void CustomTextWidget::insertChar(char c)
{
    if (cursorY < (int)buffer.size()) {
        buffer[cursorY].insert(cursorX, 1, c);
        cursorX++;
        isDirty = true;
        emitSignals();
    }
}

void CustomTextWidget::deleteChar()
{
    if (cursorX > 0 && cursorY < (int)buffer.size()) {
        buffer[cursorY].erase(cursorX - 1, 1);
        cursorX--;
        isDirty = true;
        emitSignals();
    } else if (cursorX == 0 && cursorY > 0) {

        cursorX = buffer[cursorY - 1].length();
        buffer[cursorY - 1] += buffer[cursorY];
        buffer.erase(buffer.begin() + cursorY);
        cursorY--;
        isDirty = true;
        emitSignals();
    }
}

void CustomTextWidget::insertNewline()
{
    if (cursorY < (int)buffer.size()) {
        std::string currentLine = buffer[cursorY];
        std::string newLine = currentLine.substr(cursorX);
        buffer[cursorY] = currentLine.substr(0, cursorX);
        buffer.insert(buffer.begin() + cursorY + 1, newLine);
        cursorY++;
        cursorX = 0;
        isDirty = true;
        emitSignals();
    }
}

void CustomTextWidget::moveCursor(int dx, int dy)
{
    if (dx != 0) {
        if (dx < 0) { 
            if (cursorX > 0) {
                cursorX--;
            } else if (cursorY > 0) {
                cursorY--;
                cursorX = buffer[cursorY].length();
            }
        } else { 
            if (cursorY < (int)buffer.size() && cursorX < (int)buffer[cursorY].length()) {
                cursorX++;
            } else if (cursorY < (int)buffer.size() - 1) {
                cursorY++;
                cursorX = 0;
            }
        }
    }

    if (dy != 0) {
        if (dy < 0 && cursorY > 0) { 
            cursorY--;
            cursorX = std::min(cursorX, (int)buffer[cursorY].length());
        } else if (dy > 0 && cursorY < (int)buffer.size() - 1) { 
            cursorY++;
            cursorX = std::min(cursorX, (int)buffer[cursorY].length());
        }
    }

    emitSignals();
}

void CustomTextWidget::ensureCursorVisible()
{

    int cursorScreenX = 0;
    if (cursorY < (int)buffer.size() && cursorX > 0) {
        QString lineUpToCursor = QString::fromStdString(buffer[cursorY].substr(0, cursorX));
        cursorScreenX = fontMetrics->horizontalAdvance(lineUpToCursor);
    }
    int cursorScreenY = cursorY * lineHeight;

    if (cursorScreenX < scrollOffsetX) {
        scrollOffsetX = cursorScreenX;
    } else if (cursorScreenX >= scrollOffsetX + width() - 50) {
        scrollOffsetX = cursorScreenX - width() + 50;
    }

    if (cursorScreenY < scrollOffsetY * lineHeight) {
        scrollOffsetY = cursorScreenY / lineHeight;
    } else if (cursorScreenY >= (scrollOffsetY * lineHeight) + height() - lineHeight) {
        scrollOffsetY = (cursorScreenY - height() + lineHeight) / lineHeight + 1;
    }
}

void CustomTextWidget::emitSignals()
{
    emit textChanged();
    emit cursorPositionChanged();
}