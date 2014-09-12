// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

#include "VoronoiDiagramGeneratedSite.h"

class FVoronoiDiagramGeneratedEdge
{
public:
    FVoronoiDiagramGeneratedEdge(int32 InIndex, FVector2D InLeftEndPoint, FVector2D InRightEndPoint, FVoronoiDiagramGeneratedSite InLeftSite, FVoronoiDiagramGeneratedSite InRightSite);
    
    const int32 Index;
    const FVector2D LeftEndPoint;
    const FVector2D RightEndPoint;
    const FVoronoiDiagramGeneratedSite LeftSite;
    const FVoronoiDiagramGeneratedSite RightSite;
};
