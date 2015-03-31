// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

#include "IVoronoiDiagramPoint.h"

/*
 *  Represents a vertex in the Voronoi Diagram.  Must be instantiated as a pointer.
 */
class FVoronoiDiagramVertex : public IVoronoiDiagramPoint
{
public:
    /*
     *  Creates a pointer to a new vertex
     *
     *  @param  Index       Index of the new vertex
     *  @param  Coordinate  Coordinate of the new vertex
     *  @return             Pointer to the new vertex
     */
	static TSharedPtr<FVoronoiDiagramVertex, ESPMode::ThreadSafe> CreatePtr(int32 Index, FVector2D Coordinate);
    
    // Needed for TSharedPtr
    virtual ~FVoronoiDiagramVertex(){}
    
    int32 Index;
    FVector2D Coordinate;
    
    /*
     *  Creates a vertex at the intersection of two edges.
     *
     *  @param  HalfEdgeA   The first edge
     *  @param  HalfEdgeB   The second edge
     *  @return             Pointer to the new vertex. nullptr the two edges do not intersect 
     */
	static TSharedPtr<FVoronoiDiagramVertex, ESPMode::ThreadSafe> Intersect(TSharedPtr<class FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdgeA, TSharedPtr<class FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdgeB);

    // Begin IVoronoiDiagramPoint
    virtual FVector2D GetCoordinate() const;
    // End of IVoronoiDiagramPoint


private:
    FVoronoiDiagramVertex(int32 InIndex, FVector2D InCoordinate);
};