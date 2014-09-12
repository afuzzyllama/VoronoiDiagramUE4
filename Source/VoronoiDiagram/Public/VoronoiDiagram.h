// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagram
{
public:
    FVoronoiDiagram(FIntRect InBounds);
    bool AddPoints(TArray<FIntPoint>& Points);
    void GenerateEdges(TArray<class FVoronoiDiagramGeneratedEdge>& OutEdges);
    
    TArray<TSharedPtr<class FVoronoiDiagramSite>> Sites;
    TSharedPtr<class FVoronoiDiagramSite> BottomMostSite;
    int32 CurrentSiteIndex;
    TSharedPtr<FVector2D> MinValues;
    TSharedPtr<FVector2D> MaxValues;
    TSharedPtr<FVector2D> DeltaValues;
    FIntRect Bounds;

private:
    TSharedPtr<class FVoronoiDiagramSite> GetNextSite();
    TSharedPtr<class FVoronoiDiagramSite> GetLeftRegion(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdge);
    TSharedPtr<class FVoronoiDiagramSite> GetRightRegion(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdge);
};

class FVoronoiDiagramHelper
{
public:
    static void GenerateTexture(FVoronoiDiagram VoronoiDiagram, class UTexture2D*& GeneratedTexture);
private:
    static void DrawOnMipData(class FColor* MipData, FColor Color, int32 X, int32 Y, FIntRect Bounds);
};

