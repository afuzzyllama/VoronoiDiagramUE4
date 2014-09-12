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
    static TSharedPtr<FVoronoiDiagramEdge> DELETED;
    static TSharedPtr<FVoronoiDiagramEdge> Bisect(TSharedPtr<class FVoronoiDiagramSite> SiteA, TSharedPtr<class FVoronoiDiagramSite> SiteB);

    int32 Index;
    
    // The equation of the edge: ax + by = c
    float a, b, c;
    
    TSharedPtr<class FVoronoiDiagramVertex> LeftEndPoint;
    TSharedPtr<class FVoronoiDiagramVertex> RightEndPoint;
    
    TSharedPtr<class FVoronoiDiagramSite> LeftSite;
    TSharedPtr<class FVoronoiDiagramSite> RightSite;

    void SetEndpoint(TSharedPtr<class FVoronoiDiagramVertex> Vertex, EVoronoiDiagramEdge::Type EdgeType);

private:
    FVoronoiDiagramEdge();
};
