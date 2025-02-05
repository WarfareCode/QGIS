/***************************************************************************
                             qgslayoutapputils.cpp
                             ---------------------
    Date                 : October 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutapputils.h"
#include "qgsgui.h"
#include "qgslayout.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutviewrubberband.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutmapwidget.h"
#include "qgslayoutshapewidget.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutpolygonwidget.h"
#include "qgslayoutpolylinewidget.h"
#include "qgslayoutpicturewidget.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutlabelwidget.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutlegendwidget.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemhtml.h"
#include "qgslayouthtmlwidget.h"
#include "qgslayoutscalebarwidget.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutattributetablewidget.h"
#include "qgsmapcanvas.h"

/**
 * Attempts to find the best guess at a map item to link \a referenceItem to,
 * by:
 * 1. Prioritizing a selected map
 * 2. If no selection, prioritizing the topmost map the item was drawn over
 * 3. If still none, use the layout's reference map (or biggest map)
 */
QgsLayoutItemMap *findSensibleDefaultLinkedMapItem( QgsLayoutItem *referenceItem )
{
  // start by trying to find a selected map
  QList<QgsLayoutItemMap *> mapItems;
  referenceItem->layout()->layoutItems( mapItems );

  QgsLayoutItemMap *targetMap = nullptr;
  for ( QgsLayoutItemMap *map : qgis::as_const( mapItems ) )
  {
    if ( map->isSelected() )
    {
      return map;
    }
  }

  // nope, no selection... hm, was the item drawn over a map? If so, use the topmost intersecting one
  double largestZValue = std::numeric_limits< double >::lowest();
  for ( QgsLayoutItemMap *map : qgis::as_const( mapItems ) )
  {
    if ( map->collidesWithItem( referenceItem ) && map->zValue() > largestZValue )
    {
      targetMap = map;
      largestZValue = map->zValue();
    }
  }
  if ( targetMap )
    return targetMap;

  // ah frick it, just use the reference (or biggest!) map
  return referenceItem->layout()->referenceMap();
}

void QgsLayoutAppUtils::registerGuiForKnownItemTypes( QgsMapCanvas *mapCanvas )
{
  QgsLayoutItemGuiRegistry *registry = QgsGui::layoutItemGuiRegistry();

  registry->addItemGroup( QgsLayoutItemGuiGroup( QStringLiteral( "shapes" ), QObject::tr( "Shape" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicShape.svg" ) ) ) );
  registry->addItemGroup( QgsLayoutItemGuiGroup( QStringLiteral( "nodes" ), QObject::tr( "Node Item" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddNodesItem.svg" ) ) ) );

  auto createRubberBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewRectangularRubberBand( view );
  } );
  auto createEllipseBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewEllipticalRubberBand( view );
  } );
  auto createTriangleBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewTriangleRubberBand( view );
  } );

#if 0
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 1002, QStringLiteral( "test" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLabel.svg" ) ), nullptr, createRubberBand ) );
#endif

  // map item

  auto mapItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutMap, QObject::tr( "Map" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMap.svg" ) ),
                         [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutMapWidget( qobject_cast< QgsLayoutItemMap * >( item ), mapCanvas );
  }, createRubberBand );
  mapItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( item );
    Q_ASSERT( map );

    //get the color for map canvas background and set map background color accordingly
    map->setBackgroundColor( QgsProject::instance()->backgroundColor() );

    if ( mapCanvas )
    {
      map->zoomToExtent( mapCanvas->mapSettings().visibleExtent() );
    }
  } );
  registry->addLayoutItemGuiMetadata( mapItemMetadata.release() );

  // picture item

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutPicture, QObject::tr( "Picture" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddImage.svg" ) ),
                                      [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPictureWidget( qobject_cast< QgsLayoutItemPicture * >( item ) );
  }, createRubberBand ) );


  // label item

  auto labelItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutLabel, QObject::tr( "Label" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabel.svg" ) ),
                           [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutLabelWidget( qobject_cast< QgsLayoutItemLabel * >( item ) );
  }, createRubberBand );
  labelItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemLabel *label = qobject_cast< QgsLayoutItemLabel * >( item );
    Q_ASSERT( label );

    label->setText( QObject::tr( "Lorem ipsum" ) );
    if ( QApplication::isRightToLeft() )
    {
      label->setHAlign( Qt::AlignRight );
    }
    QSizeF minSize = label->sizeForText();
    QSizeF currentSize = label->rect().size();

    //make sure label size is sufficient to fit text
    double labelWidth = std::max( minSize.width(), currentSize.width() );
    double labelHeight = std::max( minSize.height(), currentSize.height() );
    label->attemptSetSceneRect( QRectF( label->pos().x(), label->pos().y(), labelWidth, labelHeight ) );
  } );

  registry->addLayoutItemGuiMetadata( labelItemMetadata.release() );


  // legend item

  auto legendItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutLegend, QObject::tr( "Legend" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLegend.svg" ) ),
                            [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutLegendWidget( qobject_cast< QgsLayoutItemLegend * >( item ), mapCanvas );
  }, createRubberBand );
  legendItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemLegend *legend = qobject_cast< QgsLayoutItemLegend * >( item );
    Q_ASSERT( legend );

    // try to find a good map to link the legend with by default
    legend->setLinkedMap( findSensibleDefaultLinkedMapItem( legend ) );

    if ( QApplication::isRightToLeft() )
    {
      // for right-to-left locales, use an appropriate default layout
      legend->setSymbolAlignment( Qt::AlignRight );
      legend->rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignRight );
      legend->rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignRight );
      legend->rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignRight );
      legend->setTitleAlignment( Qt::AlignRight );
    }

    //set default legend font from settings
    QgsSettings settings;
    const QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
    if ( !defaultFontString.isEmpty() )
    {
      legend->rstyle( QgsLegendStyle::Title ).rfont().setFamily( defaultFontString );
      legend->rstyle( QgsLegendStyle::Group ).rfont().setFamily( defaultFontString );
      legend->rstyle( QgsLegendStyle::Subgroup ).rfont().setFamily( defaultFontString );
      legend->rstyle( QgsLegendStyle::SymbolLabel ).rfont().setFamily( defaultFontString );
    }

    legend->updateLegend();
  } );

  registry->addLayoutItemGuiMetadata( legendItemMetadata.release() );

  // scalebar item

  auto scalebarItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutScaleBar, QObject::tr( "Scale Bar" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionScaleBar.svg" ) ),
                              [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutScaleBarWidget( qobject_cast< QgsLayoutItemScaleBar * >( item ) );
  }, createRubberBand );
  scalebarItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemScaleBar *scalebar = qobject_cast< QgsLayoutItemScaleBar * >( item );
    Q_ASSERT( scalebar );

    // try to find a good map to link the scalebar with by default
    if ( QgsLayoutItemMap *targetMap = findSensibleDefaultLinkedMapItem( scalebar ) )
    {
      scalebar->setLinkedMap( targetMap );
      scalebar->applyDefaultSize( scalebar->guessUnits() );
    }
  } );

  registry->addLayoutItemGuiMetadata( scalebarItemMetadata.release() );


  // north arrow
  std::unique_ptr< QgsLayoutItemGuiMetadata > northArrowMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata>(
        QgsLayoutItemRegistry::LayoutPicture, QObject::tr( "North Arrow" ), QgsApplication::getThemeIcon( QStringLiteral( "/north_arrow.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPictureWidget( qobject_cast< QgsLayoutItemPicture * >( item ) );
  }, createRubberBand );
  northArrowMetadata->setItemCreationFunction( []( QgsLayout * layout )->QgsLayoutItem *
  {

    // count how many existing north arrows are already in layout
    QList< QgsLayoutItemPicture * > pictureItems;
    layout->layoutItems( pictureItems );
    int northArrowCount = 0;

    QgsSettings settings;
    const QString defaultPath = settings.value( QStringLiteral( "LayoutDesigner/defaultNorthArrow" ), QStringLiteral( ":/images/north_arrows/layout_default_north_arrow.svg" ), QgsSettings::Gui ).toString();

    for ( QgsLayoutItemPicture *p : qgis::as_const( pictureItems ) )
    {
      // look for pictures which use the default north arrow svg
      if ( p->picturePath() == defaultPath )
        northArrowCount++;
    }

    std::unique_ptr< QgsLayoutItemPicture > picture = qgis::make_unique< QgsLayoutItemPicture >( layout );
    picture->setNorthMode( QgsLayoutItemPicture::GridNorth );
    picture->setPicturePath( defaultPath );
    // set an id by default, so that north arrows are discernible in layout item lists
    picture->setId( northArrowCount > 0 ? QObject::tr( "North Arrow %1" ).arg( northArrowCount + 1 ) : QObject::tr( "North Arrow" ) );
    return picture.release();
  } );
  northArrowMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemPicture *picture = qobject_cast< QgsLayoutItemPicture * >( item );
    Q_ASSERT( picture );

    QList<QgsLayoutItemMap *> mapItems;
    picture->layout()->layoutItems( mapItems );

    // try to find a good map to link the north arrow with by default
    picture->setLinkedMap( findSensibleDefaultLinkedMapItem( picture ) );
  } );
  registry->addLayoutItemGuiMetadata( northArrowMetadata.release() );

  // shape items

  auto createShapeWidget =
    []( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutShapeWidget( qobject_cast< QgsLayoutItemShape * >( item ) );
  };

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Rectangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ), createShapeWidget, createRubberBand, QStringLiteral( "shapes" ), false, nullptr, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Rectangle );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Ellipse" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicCircle.svg" ) ), createShapeWidget, createEllipseBand, QStringLiteral( "shapes" ), false, nullptr, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Ellipse );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Triangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicTriangle.svg" ) ), createShapeWidget, createTriangleBand, QStringLiteral( "shapes" ), false, nullptr, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Triangle );
    return shape.release();
  } ) );

  // arrow
  std::unique_ptr< QgsLayoutItemGuiMetadata > arrowMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata>(
        QgsLayoutItemRegistry::LayoutPolyline, QObject::tr( "Arrow" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddArrow.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPolylineWidget( qobject_cast< QgsLayoutItemPolyline * >( item ) );
  }, createRubberBand, QString(), true );
  arrowMetadata->setItemCreationFunction( []( QgsLayout * layout )->QgsLayoutItem *
  {
    std::unique_ptr< QgsLayoutItemPolyline > arrow = qgis::make_unique< QgsLayoutItemPolyline >( layout );
    arrow->setEndMarker( QgsLayoutItemPolyline::ArrowHead );
    return arrow.release();
  } );
  arrowMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * )->QGraphicsPathItem*
  {
    std::unique_ptr< QGraphicsPathItem > band = qgis::make_unique< QGraphicsPathItem >();
    band->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( arrowMetadata.release() );

  // node items

  std::unique_ptr< QgsLayoutItemGuiMetadata > polygonMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >(
        QgsLayoutItemRegistry::LayoutPolygon, QObject::tr( "Polygon" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolygon.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPolygonWidget( qobject_cast< QgsLayoutItemPolygon * >( item ) );
  }, createRubberBand, QStringLiteral( "nodes" ), true );
  polygonMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * )->QGraphicsPolygonItem*
  {
    std::unique_ptr< QGraphicsPolygonItem > band = qgis::make_unique< QGraphicsPolygonItem >();
    band->setBrush( Qt::NoBrush );
    band->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( polygonMetadata.release() );

  std::unique_ptr< QgsLayoutItemGuiMetadata > polylineMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata>(
        QgsLayoutItemRegistry::LayoutPolyline, QObject::tr( "Polyline" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolyline.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPolylineWidget( qobject_cast< QgsLayoutItemPolyline * >( item ) );
  }, createRubberBand, QStringLiteral( "nodes" ), true );
  polylineMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * )->QGraphicsPathItem*
  {
    std::unique_ptr< QGraphicsPathItem > band = qgis::make_unique< QGraphicsPathItem >();
    band->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( polylineMetadata.release() );


  // html item

  auto htmlItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutHtml, QObject::tr( "HTML" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddHtml.svg" ) ),
                          [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutHtmlWidget( qobject_cast< QgsLayoutFrame * >( item ) );
  }, createRubberBand );
  htmlItemMetadata->setItemCreationFunction( [ = ]( QgsLayout * layout )->QgsLayoutItem *
  {
    std::unique_ptr< QgsLayoutItemHtml > htmlMultiFrame = qgis::make_unique< QgsLayoutItemHtml >( layout );
    QgsLayoutItemHtml *html = htmlMultiFrame.get();
    layout->addMultiFrame( htmlMultiFrame.release() );
    std::unique_ptr< QgsLayoutFrame > frame = qgis::make_unique< QgsLayoutFrame >( layout, html );
    QgsLayoutFrame *f = frame.get();
    html->addFrame( frame.release() );
    return f;
  } );
  registry->addLayoutItemGuiMetadata( htmlItemMetadata.release() );

  // attribute table item

  auto attributeTableItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutAttributeTable, QObject::tr( "Attribute Table" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddTable.svg" ) ),
                                    [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutAttributeTableWidget( qobject_cast< QgsLayoutFrame * >( item ) );
  }, createRubberBand );
  attributeTableItemMetadata->setItemCreationFunction( [ = ]( QgsLayout * layout )->QgsLayoutItem *
  {
    std::unique_ptr< QgsLayoutItemAttributeTable > tableMultiFrame = qgis::make_unique< QgsLayoutItemAttributeTable >( layout );
    QgsLayoutItemAttributeTable *table = tableMultiFrame.get();

    //set first vector layer from layer registry as table source
    QMap<QString, QgsMapLayer *> layerMap = layout->project()->mapLayers();
    for ( auto it = layerMap.constBegin() ; it != layerMap.constEnd(); ++it )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() ) )
      {
        table->setVectorLayer( vl );
        break;
      }
    }

    layout->addMultiFrame( tableMultiFrame.release() );
    std::unique_ptr< QgsLayoutFrame > frame = qgis::make_unique< QgsLayoutFrame >( layout, table );
    QgsLayoutFrame *f = frame.get();
    table->addFrame( frame.release() );
    return f;
  } );
  registry->addLayoutItemGuiMetadata( attributeTableItemMetadata .release() );
}
