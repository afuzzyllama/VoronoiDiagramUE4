// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramSite.h"

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagramSite::CreatePtr(int32 Index, FVector2D Coordinate)
{
    return TSharedPtr<FVoronoiDiagramSite>(new FVoronoiDiagramSite(Index, Coordinate));
}


FVoronoiDiagramSite::FVoronoiDiagramSite(int32 InIndex, FVector2D InCoordinate)
: Index(InIndex)
, Coordinate(InCoordinate)
{}

float FVoronoiDiagramSite::GetDistanceFrom(TSharedPtr<IVoronoiDiagramPoint> Point)
{
    float dx, dy;

    dx = GetCoordinate().X - Point->GetCoordinate().X;
    dy = GetCoordinate().Y - Point->GetCoordinate().Y;

    return FMath::Sqrt(dx * dx + dy * dy);
}

FVector2D FVoronoiDiagramSite::GetCoordinate() const
{
    return Coordinate;
}
