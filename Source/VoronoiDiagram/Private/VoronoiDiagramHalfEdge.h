// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

/*
 *  Half edge representation (left or right) for a Voronoi Diagram.  Must be instantiated as a pointer.
 */
class FVoronoiDiagramHalfEdge
{
public:
    /*
     *  Creates a new half edge pointer
     *
     *  @param  Edge        The edge of the new half edge
     (  @param  EdgeType    The edge type that this half edge represents (left or right).  The only half edges that should be "None" are the buckets in the priority queue.
     */
	static TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> CreatePtr(TSharedPtr<class FVoronoiDiagramEdge, ESPMode::ThreadSafe> Edge, EVoronoiDiagramEdge::Type EdgeType);

	TSharedPtr<class FVoronoiDiagramEdge, ESPMode::ThreadSafe> Edge;
    EVoronoiDiagramEdge::Type EdgeType;

	TSharedPtr<FVoronoiDiagramVertex, ESPMode::ThreadSafe> Vertex;
    float YStar;

	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> EdgeListLeft;
	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> EdgeListRight;
	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> NextInPriorityQueue;

    /*
     *  Is the edge left of the passed in point?
     *
     *  @param  Point   Point to check against
     *  @return         True if left, false if right
     */
    bool IsLeftOf(FVector2D Point);
    
    /*
     *  Is the edge right of the passed in point?
     *
     *  @param  Point   Point to check against
     *  @return         True if right, false if left
     */
    bool IsRightOf(FVector2D Point);

    /*
     *  Are there any references to this half edge?  This is useful to know if a half edge is still being used in the edge list or priority queue.  It might not be necessary due to the functionality already supplied by
     *
     *  @return     True if yes, false otherwise.
     */
    bool HasReferences();

private:
	FVoronoiDiagramHalfEdge(TSharedPtr<class FVoronoiDiagramEdge, ESPMode::ThreadSafe> InEdge, EVoronoiDiagramEdge::Type InEdgeType);
};
