/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                   netterfield@astro.utoronto.ca                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOGPAGE_H
#define DIALOGPAGE_H

#include <QTabWidget>
#include <QPointer>

#include "kst_export.h"

namespace Kst {

class Dialog;
class DialogTab;

class DialogPage : public QWidget
{
  Q_OBJECT
  public:
    DialogPage(Dialog *parent);
    virtual ~DialogPage();

    Dialog *dialog() const;

    QString pageTitle() const { return _pageTitle; }
    void setPageTitle(const QString &pageTitle) { _pageTitle = pageTitle; }

    QPixmap pageIcon() const { return _pageIcon; }
    void setPageIcon(const QPixmap &pageIcon) { _pageIcon = pageIcon; }

    void addDialogTab(DialogTab *tab);

    virtual QWidget* currentWidget();

  Q_SIGNALS:
    void ok();
    void apply();
    void cancel();
    void modified();

  private:
    QString _pageTitle;
    QPixmap _pageIcon;
    QPointer<Dialog> _dialog;
    QWidget* _widget;
};

class DialogPageTab : public DialogPage
{
  Q_OBJECT
  public:
    DialogPageTab(Dialog *parent);
    virtual ~DialogPageTab();

    void addDialogTab(DialogTab *tab);
    void setTabText(int i, const QString &title);

    QWidget* currentWidget();

  private:
    QTabWidget* _tabWidget;
};

}

#endif

// vim: ts=2 sw=2 et
