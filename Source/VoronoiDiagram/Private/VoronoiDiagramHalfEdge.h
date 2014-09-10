// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagramHalfEdge
{
public:
    static TSharedPtr<FVoronoiDiagramHalfEdge> CreateEmptyHalfEdgePtr();
    static TSharedPtr<FVoronoiDiagramHalfEdge> CreateHalfEdgePtr(TSharedPtr<class FVoronoiDiagramEdge> Edge, int32 LeftRight);
    static void AttemptToReset(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge);

    bool IsLeftOf(FVector2D Point) const;

    float GetYStar() const;
    void SetYStar(float NewYStar);
    
    TSharedPtr<class FVoronoiDiagramVertex> GetVertex() const;
    void SetVertex(TSharedPtr<class FVoronoiDiagramVertex> NewVertex);

    TSharedPtr<FVoronoiDiagramEdge> GetEdge();
    void SetEdge(TSharedPtr<FVoronoiDiagramEdge> NewEdge);
    
    int32 GetLeftRight();

    TSharedPtr<FVoronoiDiagramHalfEdge> GetLeftNeighbor() const;
    void SetLeftNeighbor(TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge);

    TSharedPtr<FVoronoiDiagramHalfEdge> GetRightNeighbor() const;
    void SetRightNeighbor(TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge);
    
    TSharedPtr<FVoronoiDiagramHalfEdge> GetNextInQueue() const;
    void SetNextInQueue(TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge);

    FString ToString();

private:
    float YStar;
    TSharedPtr<class FVoronoiDiagramVertex> Vertex;
    TSharedPtr<class FVoronoiDiagramEdge> Edge;
    TArray<TSharedPtr<class FVoronoiDiagramSite>> Sites;
    int32 LeftRight;

    TSharedPtr<FVoronoiDiagramHalfEdge> LeftNeighbor;
    TSharedPtr<FVoronoiDiagramHalfEdge> RightNeighbor;
    TSharedPtr<FVoronoiDiagramHalfEdge> NextInQueue;

    FVoronoiDiagramHalfEdge();
    FVoronoiDiagramHalfEdge(TSharedPtr<class FVoronoiDiagramEdge> InEdge, int32 InLeftRight);
};
