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

#include "viewvectordialog.h"

#include "document.h"
#include "vectormodel.h"
#include "editmultiplewidget.h"
#include "updateserver.h"

#include <datacollection.h>
#include <objectstore.h>
#include <QHeaderView>
#include <QMenu>

#ifdef QT5
#define setResizeMode setSectionResizeMode
#endif

namespace Kst {

ViewVectorDialog::ViewVectorDialog(QWidget *parent, Document *doc)
  : QDialog(parent), _doc(doc) {
  _model = 0;

  Q_ASSERT(_doc && _doc->objectStore());
  setupUi(this);

  int size = style()->pixelMetric(QStyle::PM_SmallIconSize);
  _showVectorList->setFixedSize(size + 8, size + 8);
  _hideVectorList->setFixedSize(size + 8, size + 8);

  _vectors->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  // Allow reorganizing the columns per drag&drop
  _vectors->horizontalHeader()->setMovable(true);

  // Custom context menu for the remove action and display format
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(contextMenu(const QPoint&)));

  connect(_resetButton, SIGNAL(clicked()), this, SLOT(reset()));

  // Add vector list, reusing the editmultiplewidget class + some tweaking
  _showMultipleWidget = new EditMultipleWidget();
  if (_showMultipleWidget) {
    // Set header
    _showMultipleWidget->setHeader(i18n("Select Vectors to View"));
    // Populate the list
    update();
    // Finish setting up the layout
    _vectorListLayout->addWidget(_showMultipleWidget);
  }
  // Add/remove buttons
  _addButton = new QPushButton();
  _addButton->setIcon(QPixmap(":kst_rightarrow.png"));
  _addButton->setShortcut(i18n("Alt+S"));
  _addButton->setToolTip(i18n("View selected vector(s)"));
  _addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _removeButton = new QPushButton();
  _removeButton->setIcon(QPixmap(":kst_leftarrow.png"));
  _removeButton->setShortcut(i18n("Alt+R"));
  _removeButton->setToolTip(i18n("Remove selected vector(s) from view"));
  _removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _addRemoveLayout->addStretch();
  _addRemoveLayout->addWidget(_addButton);
  _addRemoveLayout->addWidget(_removeButton);
  _addRemoveLayout->addStretch();

  _splitter->setStretchFactor(0,0);
  _splitter->setStretchFactor(1,1);
  _splitter->setCollapsible(1, false);
  _splitterSizes = _splitter->sizes();
  connect(_addButton, SIGNAL(clicked()), this, SLOT(addSelected()));
  connect(_removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
  connect(_showMultipleWidget, SIGNAL(itemDoubleClicked()), this, SLOT(addSelected()));
  connect(_showVectorList, SIGNAL(clicked()), this, SLOT(showVectorList()));
  connect(_hideVectorList, SIGNAL(clicked()), this, SLOT(hideVectorList()));

  connect(UpdateServer::self(), SIGNAL(objectListsChanged()), this, SLOT(update()));

}


ViewVectorDialog::~ViewVectorDialog() {
  delete _model;
  delete _showMultipleWidget;
  delete _addButton;
  delete _removeButton;
}

void ViewVectorDialog::show() {
  // vectorSelected();
  QDialog::show();
}

void ViewVectorDialog::contextMenu(const QPoint& position) {
  QMenu menu;
  QPoint cursor = QCursor::pos();
  QAction* removeAction = menu.addAction(i18n("Remove"));
  // Add submenu to select nb of digits
  QMenu* submenu = new QMenu(i18n("Significant digits"));
  QAction* digitNb0Action = submenu->addAction(i18n("Show as int"));
  QAction* digitNb3Action = submenu->addAction("3");
  QAction* digitNb6Action = submenu->addAction(i18n("6 (default)"));
  QAction* digitNb12Action = submenu->addAction("12");
  QAction* digitNb17Action = submenu->addAction("17");
  menu.addMenu(submenu);
  QAction* selectedItem = menu.exec(cursor);
  int digits = 6;
  if (selectedItem == removeAction) {
    removeSelected();
    return;
  } else if (selectedItem == digitNb0Action) {
      digits = 0;
  } else if (selectedItem == digitNb3Action) {
      digits = 3;
  } else if (selectedItem == digitNb6Action) {
      digits = 6;
  } else if (selectedItem == digitNb12Action) {
      digits = 12;
  } else if (selectedItem == digitNb17Action) {
      digits = 17;
  } else {
      return;
  }
  foreach (int column, selectedColumns()) {
    _model->setDigitNumber(column, digits);
  }
}

void ViewVectorDialog::update()
{
  VectorList objects = _doc->objectStore()->getObjects<Vector>();
  _showMultipleWidget->clearObjects();
  foreach(VectorPtr object, objects) {
    _showMultipleWidget->addObject(object->Name(), object->descriptionTip());
  }
  if (_model) {
    _model->resetIfChanged();
    _vectors->viewport()->update();
  }
}

void ViewVectorDialog::addSelected() {
  if (_model == 0) {
      _model = new VectorModel();
      _vectors->setModel(_model);
  }
  // Retrieve list of selected objects by name
  QStringList objects = _showMultipleWidget->selectedObjects();
  // Get to the pointers and add them to the model
  foreach (const QString &objectName, objects) {
    VectorPtr vector = kst_cast<Vector>(_doc->objectStore()->retrieveObject(objectName));
    if (vector) {
      _model->addVector(vector);
    }
  }
}

void ViewVectorDialog::removeSelected() {
  if (!_model) {
    return;
  }
  // Remove columns starting from the highest index to avoid shifting them
  int column;
  foreach (column, selectedColumns()) {
    _model->removeVector(column);
  }
}

void ViewVectorDialog::reset() {
  delete _model;
  _model = 0;
}

void ViewVectorDialog::showVectorList() {
  _splitterSizes[0] = qMax(_splitterSizes[0],150);
  _splitter->setSizes(_splitterSizes);
}

void ViewVectorDialog::hideVectorList() {
  _splitterSizes = _splitter->sizes();
  QList<int> sizes;
  sizes << 0 << width();
  _splitter->setSizes(sizes);
}

QList<int> ViewVectorDialog::selectedColumns() {
  // Get current selection
  QModelIndexList sel = _vectors->selectionModel()->selectedIndexes();
  // Now go through the list to see how may columns it spans
  QList<int> columns;
  QModelIndex index;
  foreach (index, sel) {
    if (!columns.contains(index.column())) {
      columns << index.column();
    }
  }
  // Sort the columns in descending order
  qSort(columns.begin(), columns.end(), qGreater<int>());
  return columns;
}


}


// vim: ts=2 sw=2 et
