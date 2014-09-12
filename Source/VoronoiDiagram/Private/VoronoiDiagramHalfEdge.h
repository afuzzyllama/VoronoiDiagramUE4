// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagramHalfEdge
{
public:
    static TSharedPtr<FVoronoiDiagramHalfEdge> CreatePtr(TSharedPtr<class FVoronoiDiagramEdge> Edge, EVoronoiDiagramEdge::Type EdgeType);

    TSharedPtr<class FVoronoiDiagramEdge> Edge;
    EVoronoiDiagramEdge::Type EdgeType;

    TSharedPtr<FVoronoiDiagramVertex> Vertex;
    float YStar;

    TSharedPtr<FVoronoiDiagramHalfEdge> EdgeListLeft;
    TSharedPtr<FVoronoiDiagramHalfEdge> EdgeListRight;
    TSharedPtr<FVoronoiDiagramHalfEdge> NextInPriorityQueue;

    bool IsLeftOf(FVector2D Point);
    bool IsRightOf(FVector2D Point);

    bool HasReferences();

private:
    // Never want to create a half edge directly since we are dealing with pointers
    FVoronoiDiagramHalfEdge(TSharedPtr<class FVoronoiDiagramEdge> InEdge, EVoronoiDiagramEdge::Type InEdgeType);
};
