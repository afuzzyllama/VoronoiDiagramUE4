// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

#include "IVoronoiDiagramPoint.h"

class FVoronoiDiagramSite : public IVoronoiDiagramPoint
{
public:
    static TSharedPtr<FVoronoiDiagramSite> CreatePtr(int32 Index, FVector2D Coordinate);
    static TSharedPtr<FVoronoiDiagramSite> Intersect(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeA, TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeB);

    int32 Index;
    
    float GetDistanceFrom(TSharedPtr<IVoronoiDiagramPoint> Point);
    virtual FVector2D GetCoordinate() const override;

private:
    FVector2D Coordinate;

    FVoronoiDiagramSite(int32 InIndex, FVector2D InCoordinate);
};
