// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

namespace EVoronoiDiagramEdge
{
    enum Type
    {
        None,
        Left,
        Right
    };
}

class FVoronoiDiagramEdge
{
public:
    static TSharedPtr<FVoronoiDiagramEdge> Bisect(TSharedPtr<class FVoronoiDiagramPoint> SiteA, TSharedPtr<class FVoronoiDiagramPoint> SiteB);

    FVoronoiDiagramEdge();
    
    bool IsDeleted();
    void MarkForDeletion();
    
    void SetEndpoint(TSharedPtr<class FVoronoiDiagramPoint> Vertex, EVoronoiDiagramEdge::Type EdgeType);

    int32 EdgeIndex;

    // The equation of the edge: ax + by = c
    float a, b, c;
    
    TSharedPtr<class FVoronoiDiagramPoint> LeftEndPoint;
    TSharedPtr<class FVoronoiDiagramPoint> RightEndPoint;
    
    TSharedPtr<class FVoronoiDiagramPoint> LeftRegion;
    TSharedPtr<class FVoronoiDiagramPoint> RightRegion;
    
    bool bMarkedForDeletion;
    

};
