// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

/*
 *  Stores final information about an edge that is returned in GenerateEdges
 */
struct FVoronoiDiagramGeneratedEdge
{
public:
    FVoronoiDiagramGeneratedEdge(int32 InIndex, FVector2D InLeftEndPoint, FVector2D InRightEndPoint)
    : Index(InIndex)
    , LeftEndPoint(InLeftEndPoint)
    , RightEndPoint(InRightEndPoint)
    {}

    int32 Index;
    FVector2D LeftEndPoint;
    FVector2D RightEndPoint;
};

/*
 *  Stores final information about a site that is returned in GenerateEdges
 */
struct FVoronoiDiagramGeneratedSite
{
public:
    FVoronoiDiagramGeneratedSite(int32 InIndex, FVector2D InCoordinate, FVector2D InCentroid, FColor InColor)
    : Index(InIndex)
    , Color(InColor)
    , Coordinate(InCoordinate)
    , Centroid(InCentroid)
    {}
    
    int32 Index;
    FColor Color;
    FVector2D Coordinate;
    FVector2D Centroid;
    TArray<FVoronoiDiagramGeneratedEdge> Edges;
    TArray<FVector2D> Vertices;
};


/*
 *  Generates Voronoi Diagram from a set of provided points
 */
class FVoronoiDiagram
{
public:
    /*
     *  Constructor
     *
     *  @param  InBounds    Rectangle that represents a 2D region for placing points
     */
    FVoronoiDiagram(FIntRect InBounds);
    
    /*
     *  Adds points to the diagram to be used at generation time
     *
     *  @param  Points  Array of integer points to be added.
     *  @return         True if added successful, false otherwise.  If false, no points are added.
     */
    bool AddPoints(const TArray<FIntPoint>& Points);
    
    /*
     *  Runs Fortune's Algorithm to generate sites with edges for the diagram
     *
     *  @param  OutSites    Array to fill with sites
     */
    void GenerateSites(TArray<class FVoronoiDiagramGeneratedSite>& OutSites);
    
private:
    /*
     *  Stored added points as Sites.  Ordered lexigraphically by y and then x
     */
    TArray<TSharedPtr<class FVoronoiDiagramSite>> Sites;
    
    /*
     *  Stores the bottom most site when running GenerateEdges
     */
    TSharedPtr<class FVoronoiDiagramSite> BottomMostSite;
    
    /*
     *  Stores the current site index when running GenerateEdges
     */
    int32 CurrentSiteIndex;
    
    /*
     *  Stores the minimum values of the points in site array.  Declared as pointer because these values need to be shared between some data structures.
     */
    TSharedPtr<FVector2D> MinValues;
    
    /*
     *  Stores the maximum values of the points in the site array.  Declared as pointer because these values need to be shared between some data structures.
     */
    TSharedPtr<FVector2D> MaxValues;
    
    /*
     *  Stores the delta values of the minimum and maximum values. Declared as pointer because these values need to be shared between some data structures.
     */
    TSharedPtr<FVector2D> DeltaValues;
    
    /*
     *  Bounds of the Voronoi Diagram
     */
    FIntRect Bounds;

    /*
     *  Returns the next site and increments CurrentSiteIndex
     */
    TSharedPtr<class FVoronoiDiagramSite> GetNextSite();
    
    /*
     *  Returns the left region in relation to a half edge
     */
    TSharedPtr<class FVoronoiDiagramSite> GetLeftRegion(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdge);
    
    /*
     *  Returns the right region in relation to a half edge
     */
    TSharedPtr<class FVoronoiDiagramSite> GetRightRegion(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdge);
    
    friend class FVoronoiDiagramHelper;
};

class FVoronoiDiagramHelper
{
public:
    /*
     *  Creates a texture images of the Voronoi Diagram in the passed texture.  Assumes that points have already been added to the Voronoi Diagram
     */
    static void GenerateTexture(FVoronoiDiagram VoronoiDiagram, int32 RelaxationCycles, class UTexture2D*& GeneratedTexture);
private:
    struct FLineSegment
    {
        FLineSegment(float InY, float InXl, float InXr, float InDy)
        : y(InY)
        , xl(InXl)
        , xr(InXr)
        , dy(InDy)
        {}
        
        float y, xl, xr, dy;
    };

    /*
     *  Calculates the index and, if valid, colors the pixel of the texture.  Assumes that MipData is valid and locked for writing.
     */
    static void DrawOnMipData(class FColor* MipData, FColor Color, int32 X, int32 Y, FIntRect Bounds);
    
    /*
     *  Does the passed in point lie inside of the vertices passed in.  The verices are assumed to be sorted.
     */
    static bool PointInVertices(FIntPoint Point, TArray<FVector2D> Vertices);
};

