#include "clickablewidget.h"
#include <QApplication>
#include <QWidget>

ClickableWidget::ClickableWidget(QWidget *parent)
    : QWidget(parent)
{

}

void ClickableWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget *focusWidget = QApplication::focusWidget();
    if (focusWidget) {
        focusWidget->clearFocus();
    }
    QWidget::mousePressEvent(event);
}
