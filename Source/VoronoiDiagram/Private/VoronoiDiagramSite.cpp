// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramSite.h"

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagramSite::CreateSitePtr(int32 Index, FVector2D InCoordinate)
{
    return TSharedPtr<FVoronoiDiagramSite>(new FVoronoiDiagramSite(Index, InCoordinate));
}

void FVoronoiDiagramSite::AddEdge(TSharedPtr<FVoronoiDiagramEdge> NewEdge)
{
    Edges.Add(NewEdge);
}

TArray<TSharedPtr<class FVoronoiDiagramEdge>> FVoronoiDiagramSite::GetEdges() const
{
    return Edges;
}

float FVoronoiDiagramSite::DistanceFrom(FVector2D Point)
{
    float dx = Point.X - Coordinate.X;
    float dy = Point.Y - Coordinate.Y;

    return FMath::Sqrt(dx * dx + dy * dy);
}

int32 FVoronoiDiagramSite::GetIndex() const
{
    return Index;
}

FVector2D FVoronoiDiagramSite::GetCoordinate() const
{
    return Coordinate;
}

FString FVoronoiDiagramSite::ToString()
{
    return FString::Printf(TEXT("Site - Index: %i; Coordinate: %i, %i"), GetIndex(), GetCoordinate().X, GetCoordinate().Y);
}

FVoronoiDiagramSite::FVoronoiDiagramSite(int32 InIndex, FVector2D InCoordinate)
: Index(InIndex)
, Coordinate(InCoordinate)
{}