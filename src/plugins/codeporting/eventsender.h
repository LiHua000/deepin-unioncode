/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef EVENTSENDER_H
#define EVENTSENDER_H

#include <QObject>

class EventSender : public QObject
{
    Q_OBJECT
public:
     static void jumpTo(const QString &filePath, int lineNum);

     static void setAnnotation(const QString &filePath, int lineNum, const QString &text);
     static void cleanAnnotation(const QString &filePath);
     static void setLineBackground(const QString &filePath, QColor &color, int lineNum);
     static void cleanLineBackground(const QString &filePath, int lineNum);
     static void cleanLineBackground(const QString &filePath);


private:
     explicit EventSender(QObject *parent = nullptr);
};

#endif // EVENTSENDER_H