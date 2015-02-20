// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

/*
 *  Stores final information about a site that is returned in GenerateEdges
 */
class FVoronoiDiagramGeneratedSite
{
public:
    FVoronoiDiagramGeneratedSite(int32 InIndex, FVector2D InCoordinate, FVector2D InCentroid, FColor InColor, bool InIsCorner, bool InIsEdge);
    
    int32 Index;
    FColor Color;
    FVector2D Coordinate;
    FVector2D Centroid;
    TArray<class FVoronoiDiagramGeneratedEdge> Edges;
    TArray<FVector2D> Vertices;
    TArray<int32> NeighborSites;
    
    bool bIsCorner;
    bool bIsEdge;
};