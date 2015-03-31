// Copyright 2015 afuzzyllama. All Rights Reserved.
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

/*
 *  Represents a Voronoi Diagram edge.  Must be instantiated as a pointer.
 */
class FVoronoiDiagramEdge
{
public:
    /*
     *  Represents a deleted edge.  Any edge that needs to present a deleted edge will point to this.
     */
	static TSharedPtr<FVoronoiDiagramEdge, ESPMode::ThreadSafe> DELETED;
    
    /*
     *  Return an edge that is the bisection between two sites
     *
     *  @param  SiteA   The first site
     *  @param  SiteB   The second site
     *  @return         A pointer to the new edge
     */
	static TSharedPtr<FVoronoiDiagramEdge, ESPMode::ThreadSafe> Bisect(TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> SiteA, TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> SiteB);

    int32 Index;
    
    // The equation of the edge: ax + by = c
    float a, b, c;
    
	TSharedPtr<class FVoronoiDiagramVertex, ESPMode::ThreadSafe> LeftEndPoint;
	TSharedPtr<class FVoronoiDiagramVertex, ESPMode::ThreadSafe> RightEndPoint;

    FVector2D LeftClippedEndPoint;
    FVector2D RightClippedEndPoint;
    
	TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> LeftSite;
	TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> RightSite;

    /*
     *  Sets and end point for the edge
     *
     *  @param  Vertex      The vertex that represents the end point
     *  @param  EdgeType    The edge type of this vertex is (left or right) 
     */
	void SetEndpoint(TSharedPtr<class FVoronoiDiagramVertex, ESPMode::ThreadSafe> Vertex, EVoronoiDiagramEdge::Type EdgeType);
    
    /*
     *  After a diagram is completely generated.  This will clean up any end points that are outside of the passed in bounds or that are still nullptr.
     *
     *  @param  Bounds  The bounds of the Voronoi Diagram
     */
    void GenerateClippedEndPoints(FIntRect Bounds);

private:
    FVoronoiDiagramEdge();
};
