// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

#include "IVoronoiDiagramPoint.h"

/*
 *  Represents a Voronoi Diagram Site.  Must be instantiated as a pointer.
 */
class FVoronoiDiagramSite : public IVoronoiDiagramPoint
{
public:
    /*
     *  Creates a pointer to a site
     *
     *  @paran  Index       Index of the new site
     *  @param  Coordinate  Coordinate of the new site
     *  @return             Pointer to a the new site
     */
    static TSharedPtr<FVoronoiDiagramSite> CreatePtr(int32 Index, FVector2D Coordinate);

    int32 Index;
    FVector2D Coordinate;

    FVector2D Centroid;
    TArray<FVector2D> Vertices;
    
    TArray<TSharedPtr<class FVoronoiDiagramEdge>> Edges;

    /*
     *  Gets the distance between the site and the passed in Voronoi Diagram point
     *
     *  @param  Point   Point to calculation distance from
     *  @return         Distance between this site and the pointer
     */
    float GetDistanceFrom(TSharedPtr<IVoronoiDiagramPoint> Point);
    
    void GenerateCentroid(FIntRect Bounds);
    
    // Begin IVoronoiDiagramPoint
    virtual FVector2D GetCoordinate() const override;
    // end of IVoronoiDiagramPoint

private:
    FVoronoiDiagramSite(int32 InIndex, FVector2D InCoordinate);
    
    /*
     *  Cross as defined in Monotone Chain
     */
    float Cross(const FVector2D& O, const FVector2D& A, const FVector2D& B);
};
