// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramGeneratedSite.h"

FVoronoiDiagramGeneratedSite::FVoronoiDiagramGeneratedSite(int32 InIndex, FVector2D InCoordinate, FVector2D InCentroid, FColor InColor, bool InIsCorner, bool InIsEdge)
: Index(InIndex)
, Color(InColor)
, Coordinate(InCoordinate)
, Centroid(InCentroid)
, bIsCorner(InIsCorner)
, bIsEdge(InIsEdge)
{}