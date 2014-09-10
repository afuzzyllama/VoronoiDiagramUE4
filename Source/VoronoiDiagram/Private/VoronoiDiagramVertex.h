// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagramVertex
{
public:
    static TSharedPtr<FVoronoiDiagramVertex> Intersect(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeA, TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdgeB);
    
    int32 GetIndex() const;
    FVector2D GetCoordinate() const;

    FString ToString();
private:
    int32 Index;
    FVector2D Coordinate;

    FVoronoiDiagramVertex(FVector2D InPoint);

};