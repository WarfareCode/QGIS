# -*- coding: utf-8 -*-

"""
***************************************************************************
    QgisAlgorithmProvider.py
    ---------------------
    Date                 : December 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os

from qgis.core import (QgsApplication,
                       QgsProcessingProvider)

from PyQt5.QtCore import QCoreApplication

from .AddTableField import AddTableField
from .Aggregate import Aggregate
from .Aspect import Aspect
from .BarPlot import BarPlot
from .BasicStatistics import BasicStatisticsForField
from .BoxPlot import BoxPlot
from .CheckValidity import CheckValidity
from .Climb import Climb
from .ConcaveHull import ConcaveHull
from .CreateAttributeIndex import CreateAttributeIndex
from .CreateConstantRaster import CreateConstantRaster
from .Datasources2Vrt import Datasources2Vrt
from .DefineProjection import DefineProjection
from .Delaunay import Delaunay
from .DeleteColumn import DeleteColumn
from .DeleteDuplicateGeometries import DeleteDuplicateGeometries
from .DensifyGeometries import DensifyGeometries
from .EliminateSelection import EliminateSelection
from .ExecuteSQL import ExecuteSQL
from .ExportGeometryInfo import ExportGeometryInfo
from .ExtentFromLayer import ExtentFromLayer
from .ExtractSpecificVertices import ExtractSpecificVertices
from .FieldPyculator import FieldsPyculator
from .FieldsCalculator import FieldsCalculator
from .FieldsMapper import FieldsMapper
from .FindProjection import FindProjection
from .GeometryConvert import GeometryConvert
from .GeometryByExpression import GeometryByExpression
from .Heatmap import Heatmap
from .Hillshade import Hillshade
from .HubDistanceLines import HubDistanceLines
from .HubDistancePoints import HubDistancePoints
from .HypsometricCurves import HypsometricCurves
from .IdwInterpolation import IdwInterpolation
from .ImportIntoPostGIS import ImportIntoPostGIS
from .ImportIntoSpatialite import ImportIntoSpatialite
from .KeepNBiggestParts import KeepNBiggestParts
from .KNearestConcaveHull import KNearestConcaveHull
from .LinesToPolygons import LinesToPolygons
from .MeanAndStdDevPlot import MeanAndStdDevPlot
from .MinimumBoundingGeometry import MinimumBoundingGeometry
from .NearestNeighbourAnalysis import NearestNeighbourAnalysis
from .Orthogonalize import Orthogonalize
from .PointDistance import PointDistance
from .PointsDisplacement import PointsDisplacement
from .PointsFromLines import PointsFromLines
from .PointsFromPolygons import PointsFromPolygons
from .PointsInPolygon import PointsInPolygon
from .PointsLayerFromTable import PointsLayerFromTable
from .PointsToPaths import PointsToPaths
from .PolarPlot import PolarPlot
from .PoleOfInaccessibility import PoleOfInaccessibility
from .Polygonize import Polygonize
from .PostGISExecuteSQL import PostGISExecuteSQL
from .PostGISExecuteAndLoadSQL import PostGISExecuteAndLoadSQL
from .RandomExtract import RandomExtract
from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
from .RandomPointsAlongLines import RandomPointsAlongLines
from .RandomPointsExtent import RandomPointsExtent
from .RandomPointsLayer import RandomPointsLayer
from .RandomPointsPolygons import RandomPointsPolygons
from .RandomSelection import RandomSelection
from .RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from .Rasterize import RasterizeAlgorithm
from .RasterCalculator import RasterCalculator
from .RasterLayerHistogram import RasterLayerHistogram
from .RasterLayerStatistics import RasterLayerStatistics
from .RasterSampling import RasterSampling
from .RectanglesOvalsDiamondsFixed import RectanglesOvalsDiamondsFixed
from .RectanglesOvalsDiamondsVariable import RectanglesOvalsDiamondsVariable
from .RegularPoints import RegularPoints
from .Relief import Relief
from .Ruggedness import Ruggedness
from .SelectByAttribute import SelectByAttribute
from .SelectByExpression import SelectByExpression
from .ServiceAreaFromLayer import ServiceAreaFromLayer
from .ServiceAreaFromPoint import ServiceAreaFromPoint
from .SetMValue import SetMValue
from .SetRasterStyle import SetRasterStyle
from .SetVectorStyle import SetVectorStyle
from .SetZValue import SetZValue
from .SingleSidedBuffer import SingleSidedBuffer
from .Slope import Slope
from .SnapGeometries import SnapGeometriesToLayer
from .SpatialiteExecuteSQL import SpatialiteExecuteSQL
from .SpatialIndex import SpatialIndex
from .SpatialJoin import SpatialJoin
from .SpatialJoinSummary import SpatialJoinSummary
from .StatisticsByCategories import StatisticsByCategories
from .SumLines import SumLines
from .TextToFloat import TextToFloat
from .TilesXYZ import TilesXYZAlgorithmDirectory, TilesXYZAlgorithmMBTiles
from .TinInterpolation import TinInterpolation
from .TopoColors import TopoColor
from .TruncateTable import TruncateTable
from .UniqueValues import UniqueValues
from .VariableDistanceBuffer import VariableDistanceBuffer
from .VectorLayerHistogram import VectorLayerHistogram
from .VectorLayerScatterplot import VectorLayerScatterplot
from .VectorLayerScatterplot3D import VectorLayerScatterplot3D
from .VectorSplit import VectorSplit
from .VoronoiPolygons import VoronoiPolygons
from .ZonalStatistics import ZonalStatistics


class QgisAlgorithmProvider(QgsProcessingProvider):
    fieldMappingParameterName = QCoreApplication.translate('Processing', 'Fields Mapper')

    def __init__(self):
        super().__init__()
        self.algs = []
        self.externalAlgs = []

    def getAlgs(self):
        algs = [AddTableField(),
                Aggregate(),
                Aspect(),
                BarPlot(),
                BasicStatisticsForField(),
                BoxPlot(),
                CheckValidity(),
                Climb(),
                ConcaveHull(),
                CreateAttributeIndex(),
                CreateConstantRaster(),
                Datasources2Vrt(),
                DefineProjection(),
                Delaunay(),
                DeleteColumn(),
                DeleteDuplicateGeometries(),
                DensifyGeometries(),
                EliminateSelection(),
                ExecuteSQL(),
                ExportGeometryInfo(),
                ExtentFromLayer(),
                ExtractSpecificVertices(),
                FieldsCalculator(),
                FieldsMapper(),
                FieldsPyculator(),
                FindProjection(),
                GeometryByExpression(),
                GeometryConvert(),
                Heatmap(),
                Hillshade(),
                HubDistanceLines(),
                HubDistancePoints(),
                HypsometricCurves(),
                IdwInterpolation(),
                ImportIntoPostGIS(),
                ImportIntoSpatialite(),
                KeepNBiggestParts(),
                KNearestConcaveHull(),
                LinesToPolygons(),
                MeanAndStdDevPlot(),
                MinimumBoundingGeometry(),
                NearestNeighbourAnalysis(),
                Orthogonalize(),
                PointDistance(),
                PointsDisplacement(),
                PointsFromLines(),
                PointsFromPolygons(),
                PointsInPolygon(),
                PointsLayerFromTable(),
                PointsToPaths(),
                PolarPlot(),
                PoleOfInaccessibility(),
                Polygonize(),
                PostGISExecuteSQL(),
                PostGISExecuteAndLoadSQL(),
                RandomExtract(),
                RandomExtractWithinSubsets(),
                RandomPointsAlongLines(),
                RandomPointsExtent(),
                RandomPointsLayer(),
                RandomPointsPolygons(),
                RandomSelection(),
                RandomSelectionWithinSubsets(),
                RasterCalculator(),
                RasterizeAlgorithm(),
                RasterLayerHistogram(),
                RasterLayerStatistics(),
                RasterSampling(),
                RectanglesOvalsDiamondsFixed(),
                RectanglesOvalsDiamondsVariable(),
                RegularPoints(),
                Relief(),
                Ruggedness(),
                SelectByAttribute(),
                SelectByExpression(),
                ServiceAreaFromLayer(),
                ServiceAreaFromPoint(),
                SetMValue(),
                SetRasterStyle(),
                SetVectorStyle(),
                SetZValue(),
                SingleSidedBuffer(),
                Slope(),
                SnapGeometriesToLayer(),
                SpatialiteExecuteSQL(),
                SpatialIndex(),
                SpatialJoin(),
                SpatialJoinSummary(),
                StatisticsByCategories(),
                SumLines(),
                TextToFloat(),
                TilesXYZAlgorithmDirectory(),
                TilesXYZAlgorithmMBTiles(),
                TinInterpolation(),
                TopoColor(),
                TruncateTable(),
                UniqueValues(),
                VariableDistanceBuffer(),
                VectorLayerHistogram(),
                VectorLayerScatterplot(),
                VectorLayerScatterplot3D(),
                VectorSplit(),
                VoronoiPolygons(),
                ZonalStatistics()
                ]

        return algs

    def id(self):
        return 'qgis'

    def helpId(self):
        return 'qgis'

    def name(self):
        return 'QGIS'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def loadAlgorithms(self):
        self.algs = self.getAlgs()
        for a in self.algs:
            self.addAlgorithm(a)
        for a in self.externalAlgs:
            self.addAlgorithm(a)

    def load(self):
        success = super().load()

        if success:
            self.parameterTypeFieldsMapping = FieldsMapper.ParameterFieldsMappingType()
            QgsApplication.instance().processingRegistry().addParameterType(self.parameterTypeFieldsMapping)

        return success

    def unload(self):
        super().unload()

        QgsApplication.instance().processingRegistry().removeParameterType(self.parameterTypeFieldsMapping)

    def supportsNonFileBasedOutput(self):
        return True
