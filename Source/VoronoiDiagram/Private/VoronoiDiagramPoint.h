// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

namespace EVoronoiDiagramPoint
{
    enum Type
    {
        None,
        Site,
        Vertex
    };
}

class FVoronoiDiagramPoint
{
public:
    static TSharedPtr<FVoronoiDiagramPoint> CreatePtr(int32 Index, EVoronoiDiagramPoint::Type Type, FVector2D Coordinate);
    static TSharedPtr<FVoronoiDiagramPoint> Intersect(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeA, TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeB);
    
    FVoronoiDiagramPoint();
    FVoronoiDiagramPoint(int32 InIndex, EVoronoiDiagramPoint::Type InType, FVector2D InCoordinate);

    float GetDistanceFrom(TSharedPtr<FVoronoiDiagramPoint> Point);

    int32 Index;
    EVoronoiDiagramPoint::Type Type;
    FVector2D Coordinate;
};
