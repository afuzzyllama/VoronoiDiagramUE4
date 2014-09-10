// Copyright 2014 afuzzyllama. All Rights Reserved.
#pragma once

class FVoronoiDiagramSite
{
public:
    static TSharedPtr<class FVoronoiDiagramSite> CreateSitePtr(int32 Index, FVector2D InCoordinate);

    void AddEdge(TSharedPtr<class FVoronoiDiagramEdge> NewEdge);

    TArray<TSharedPtr<class FVoronoiDiagramEdge>> GetEdges() const;

    float DistanceFrom(FVector2D Point);

    int32 GetIndex() const;
    FVector2D GetCoordinate() const;
    
    FString ToString();
private:
    int32 Index;
    FVector2D Coordinate;
    TArray<TSharedPtr<class FVoronoiDiagramEdge>> Edges;
    
    FVoronoiDiagramSite(int32 InIndex, FVector2D InCoordinate);
};