/***************************************************************************
                             qgsreportorganizerwidget.h
                             ----------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREPORTORGANIZERWIDGET_H
#define QGSREPORTORGANIZERWIDGET_H

#include "qgis_gui.h"
#include "ui_qgsreportorganizerwidgetbase.h"
#include "qgspanelwidget.h"
#include <QStyledItemDelegate>

class QgsReportSectionModel;
class QgsReport;
class QgsMessageBar;
class QgsLayoutDesignerInterface;
class QgsAbstractReportSection;

class GUI_EXPORT QgsReportOrganizerWidget: public QgsPanelWidget, private Ui::QgsReportOrganizerBase
{
    Q_OBJECT
  public:
    QgsReportOrganizerWidget( QWidget *parent, QgsLayoutDesignerInterface *designer, QgsReport *report );

    void setMessageBar( QgsMessageBar *bar );
    void setEditedSection( QgsAbstractReportSection *section );

  private slots:

    void addLayoutSection();
    void addFieldGroupSection();
    void removeSection();
    void selectionChanged( const QModelIndex &current, const QModelIndex &previous );

  private:

    QgsReport *mReport = nullptr;
    QgsReportSectionModel *mSectionModel = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QgsLayoutDesignerInterface *mDesigner = nullptr;
    QWidget *mConfigWidget = nullptr;

};



#endif // QGSREPORTORGANIZERWIDGET_H
