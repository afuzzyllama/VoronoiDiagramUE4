// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagram
{
public:
    FVoronoiDiagram(FIntRect InBounds);
    bool AddPoints(TArray<FIntPoint>& Points);
    void GenerateEdges();
    
private:
    TArray<TSharedPtr<class FVoronoiDiagramPoint>> Sites;
    int32 CurrentSiteIndex;
    TSharedPtr<FVector2D> MinValues;
    TSharedPtr<FVector2D> MaxValues;
    TSharedPtr<FVector2D> DeltaValues;
    FIntRect Bounds;
    
    TSharedPtr<class FVoronoiDiagramPoint> GetNextSite();

};

// class FVoronoiDiagramHelper
// {
// public:
//    static void GenerateTexture(FVoronoiDiagram VoronoiDiagram, class UTexture2D*& GeneratedTexture);
// };

