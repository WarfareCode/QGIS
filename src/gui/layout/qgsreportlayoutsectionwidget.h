/***************************************************************************
                             qgsreportlayoutsectionwidget.h
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

#ifndef QGSREPORTLAYOUTSECTIONWIDGET_H
#define QGSREPORTLAYOUTSECTIONWIDGET_H

#include "qgis_gui.h"
#include "ui_qgsreportwidgetlayoutsectionbase.h"

class QgsLayoutDesignerInterface;
class QgsReportSectionLayout;
class QgsReportOrganizerWidget;

class GUI_EXPORT QgsReportLayoutSectionWidget: public QWidget, private Ui::QgsReportWidgetLayoutSectionBase
{
    Q_OBJECT
  public:
    QgsReportLayoutSectionWidget(QgsReportOrganizerWidget *parent, QgsLayoutDesignerInterface *designer, QgsReportSectionLayout *section );

  private slots:

    void toggleBody( bool enabled );
    void editBody();

  private:

    QgsReportOrganizerWidget *mOrganizer = nullptr;
    QgsReportSectionLayout *mSection = nullptr;
    QgsLayoutDesignerInterface *mDesigner = nullptr;

};

#endif // QGSREPORTLAYOUTSECTIONWIDGET_H
