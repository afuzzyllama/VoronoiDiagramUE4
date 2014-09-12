// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

#include "IVoronoiDiagramPoint.h"

class FVoronoiDiagramVertex : public IVoronoiDiagramPoint
{
public:
    static TSharedPtr<FVoronoiDiagramVertex> CreatePtr(int32 Index, FVector2D Coordinate);
    static TSharedPtr<FVoronoiDiagramVertex> Intersect(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeA, TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeB);
    
    int32 Index;
    
    virtual FVector2D GetCoordinate() const override;

    
private:
    FVector2D Coordinate;

    FVoronoiDiagramVertex(int32 InIndex, FVector2D InCoordinate);
};