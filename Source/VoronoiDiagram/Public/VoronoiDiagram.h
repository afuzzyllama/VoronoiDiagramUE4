// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagram
{
public:
    static const int32 LeftEdge;
    static const int32 RightEdge;
    
    FVoronoiDiagram(FIntRect InBounds);

    /*
     *  Adds points to the diagram.  
     *
     *  @oaram  Points  Points to add to the diagram
     *  @return         True if points added successfully, false otherwise.  If false, no points were added.
     */
    bool AddPoints(TArray<FIntPoint>& Points);

    /*
     *  Generates the diagram and places the edges into the passed array if successful
     *
     *  @param  OutEdges    Array to place the generated edges into.  Empty is not successful
     *  @return             True if successful, false otherwise
     */
    bool GetEdges(TArray<TArray<FIntPoint>>& OutEdges);

    FIntRect GetBounds();
    TArray<TSharedPtr<class FVoronoiDiagramSite>> GetSites();
    
    TSharedPtr<class FVoronoiDiagramSite> GetNextSiteAndIncrementCount();
    

    
private:
    /*
     *  Sites that will generate the diagram
     */
    TArray<TSharedPtr<class FVoronoiDiagramSite>> Sites;
    
    /*
     *  Edges that are genreated from the sites
     */
    TArray<TSharedPtr<class FVoronoiDiagramEdge>> GeneratedEdges;
    
    int32 CurrentSiteIndex;
    
    /*
     *  Minimum values of points
     */
    FVector2D MinValues;
    
    /*
     *  Maximum values of points
     */
    FVector2D MaxValues;
    
    /*
     *  Delta values of points
     */
    FVector2D DeltaValues;
    
    /*
     *  Half edge priority queue
     */
    TArray<TSharedPtr<class FVoronoiDiagramHalfEdge>> PriorityQueue;
    
    
    TSharedPtr<FVoronoiDiagramSite> BottomMostSite;
    
    /*
     *  Bounds of the diagram
     */
    FIntRect Bounds;

    TSharedPtr<FVoronoiDiagramSite> GetLeftRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge);
    TSharedPtr<FVoronoiDiagramSite> GetRightRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge);
    
};

class FVoronoiDiagramHelper
{
public:
    static void GenerateTexture(FVoronoiDiagram VoronoiDiagram, class UTexture2D*& GeneratedTexture);
};