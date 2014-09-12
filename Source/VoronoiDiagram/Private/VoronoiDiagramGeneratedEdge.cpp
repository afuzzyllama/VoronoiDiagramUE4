// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramGeneratedEdge.h"

FVoronoiDiagramGeneratedEdge::FVoronoiDiagramGeneratedEdge(int32 InIndex, FVector2D InLeftEndPoint, FVector2D InRightEndPoint, FVoronoiDiagramGeneratedSite InLeftSite, FVoronoiDiagramGeneratedSite InRightSite)
: Index(InIndex)
, LeftEndPoint(InLeftEndPoint)
, RightEndPoint(InRightEndPoint)
, LeftSite(InLeftSite)
, RightSite(InRightSite)
{}