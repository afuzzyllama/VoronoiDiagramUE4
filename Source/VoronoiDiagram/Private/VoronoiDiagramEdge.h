// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagramEdge
{
public:
    static TSharedPtr<FVoronoiDiagramEdge> CreateBisectingEdge(TSharedPtr<FVoronoiDiagramSite> SiteA, TSharedPtr<FVoronoiDiagramSite> SiteB);
    static TSharedPtr<FVoronoiDiagramEdge> CreateEdgePtr(TSharedPtr<FVoronoiDiagramEdge> Edge, int32 LeftRight);
    
    void MarkReadyForDeletion();
    bool IsReadyForDeletion() const;

    TSharedPtr<class FVoronoiDiagramSite> GetLeftSite() const;
    void SetLeftSite(TSharedPtr<class FVoronoiDiagramSite> NewSite);

    TSharedPtr<class FVoronoiDiagramSite> GetRightSite() const;
    void SetRightSite(TSharedPtr<class FVoronoiDiagramSite> NewSite);

    TSharedPtr<class FVoronoiDiagramVertex> GetLeftVertex() const;
    void SetLeftVertex(TSharedPtr<class FVoronoiDiagramVertex> NewVertex);
    
    TSharedPtr<class FVoronoiDiagramVertex> GetRightVertex() const;
    void SetRightVertex(TSharedPtr<class FVoronoiDiagramVertex> NewVertex);

    void ClipVertices(FIntRect Bounds);

    // the equation of the edge: ax + by = c
	float a, b, c;

    TArray<FIntPoint> GetClippedVertices() const;
    
    FString ToString();

private:
    TArray<FIntPoint> ClippedVertices;

    bool bReadyForDeletion;
    TSharedPtr<class FVoronoiDiagramSite> LeftSite;
    TSharedPtr<class FVoronoiDiagramSite> RightSite;
    
    TSharedPtr<class FVoronoiDiagramVertex> LeftVertex;
    TSharedPtr<class FVoronoiDiagramVertex> RightVertex;
    


    FVoronoiDiagramEdge();
};