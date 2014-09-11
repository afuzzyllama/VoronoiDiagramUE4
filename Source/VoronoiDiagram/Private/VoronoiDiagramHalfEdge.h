// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagramHalfEdge
{
public:
    static TSharedPtr<FVoronoiDiagramHalfEdge> CreatePtr(TSharedPtr<class FVoronoiDiagramEdge> Edge, EVoronoiDiagramEdge::Type EdgeType);

    bool IsLeftOf(TSharedPtr<class FVoronoiDiagramPoint> Point);
    bool IsRightOf(TSharedPtr<class FVoronoiDiagramPoint> Point);
    
    TSharedPtr<class FVoronoiDiagramEdge> Edge;
    EVoronoiDiagramEdge::Type EdgeType;

    TSharedPtr<FVoronoiDiagramPoint> Vertex;
    float YStar;

    TSharedPtr<FVoronoiDiagramHalfEdge> EdgeListLeft;
    TSharedPtr<FVoronoiDiagramHalfEdge> EdgeListRight;
    TSharedPtr<FVoronoiDiagramHalfEdge> NextInPriorityQueue;

private:
    // Never want to create a half edge directly since we are dealing with pointers
    FVoronoiDiagramHalfEdge(TSharedPtr<class FVoronoiDiagramEdge> InEdge, EVoronoiDiagramEdge::Type InEdgeType);
};
