#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef OWNCALENDAR_H
#define OWNCALENDAR_H

#include <QCalendarWidget>

class ownCalendarWidget final : public QCalendarWidget
{
    Q_OBJECT

public:
    ownCalendarWidget(QWidget *parent = nullptr);
    ~ownCalendarWidget();

    void ourCall(QDate);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);

    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
};

#endif // OWNCALENDAR_H
