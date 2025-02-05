/***************************************************************************
    qgsreportsectionmodel.h
    ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREPORTSECTIONMODEL_H
#define QGSREPORTSECTIONMODEL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsreport.h"
#include <QAbstractItemModel>

/**
 * \ingroup gui
 * \class QgsReportSectionModel
 * \brief A model for managing the sections in a QgsReport.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsReportSectionModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsReportSectionModel, for the specified \a report.
     */
    QgsReportSectionModel( QgsReport *report, QObject *parent );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    bool hasChildren( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    void addSection( const QModelIndex &parent, std::unique_ptr< QgsAbstractReportSection > section ) SIP_SKIP;

    /**
     * Returns the report section for the given \a index.
     */
    QgsAbstractReportSection *sectionForIndex( const QModelIndex &index ) const;

    QModelIndex indexForSection( QgsAbstractReportSection *section ) const;

    void setEditedSection( QgsAbstractReportSection *section );

  private:
    QgsReport *mReport = nullptr;
    QgsAbstractReportSection *mEditedSection = nullptr;
};

#endif // QGSREPORTSECTIONMODEL_H
