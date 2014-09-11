// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramEdge.h"

TSharedPtr<FVoronoiDiagramEdge> FVoronoiDiagramEdge::Bisect(TSharedPtr<FVoronoiDiagramPoint> SiteA, TSharedPtr<FVoronoiDiagramPoint> SiteB)
{
    float dx, dy;
    TSharedPtr<FVoronoiDiagramEdge> NewEdge(new FVoronoiDiagramEdge());
    
    NewEdge->LeftRegion = SiteA;
    NewEdge->RightRegion = SiteB;
    
    NewEdge->LeftEndPoint = nullptr;
    NewEdge->RightEndPoint = nullptr;
    
    dx = SiteB->Coordinate.X - SiteA->Coordinate.X;
    dy = SiteB->Coordinate.Y - SiteA->Coordinate.Y;
    
    NewEdge->c = SiteA->Coordinate.X * dx + SiteA->Coordinate.Y * dy + (dx * dx + dy * dy) * 0.5f;
    if(FMath::Abs(dx) > FMath::Abs(dy))
    {
        NewEdge->a = 1.0f;
        NewEdge->b = dy/dx;
        NewEdge->c /= dx;
    }
    else
    {
        NewEdge->b = 1.0f;
        NewEdge->a = dx/dy;
        NewEdge->c /= dy;
    }
    
    return NewEdge;
}

FVoronoiDiagramEdge::FVoronoiDiagramEdge()
: bMarkedForDeletion(false)
{}

bool FVoronoiDiagramEdge::IsDeleted()
{
    return bMarkedForDeletion;
}

void FVoronoiDiagramEdge::MarkForDeletion()
{
    bMarkedForDeletion = true;
}

void FVoronoiDiagramEdge::SetEndpoint(TSharedPtr<class FVoronoiDiagramPoint> Vertex, EVoronoiDiagramEdge::Type EdgeType)
{
    if(EdgeType == EVoronoiDiagramEdge::Left)
    {
        LeftEndPoint = Vertex;
    }
    else
    {
        RightEndPoint = Vertex;
    }
    
    if(
        (EdgeType == EVoronoiDiagramEdge::Left && RightEndPoint.IsValid() == false) ||
        (EdgeType == EVoronoiDiagramEdge::Right && LeftEndPoint.IsValid() == false)
    )
    {
        return;
    }

    UE_LOG(LogVoronoiDiagram, Log, TEXT("Edge %i: %i@(%f, %f) -> %i@(%f, %f)"), EdgeIndex, LeftEndPoint->Index, LeftEndPoint->Coordinate.X, LeftEndPoint->Coordinate.Y, RightEndPoint->Coordinate.X, RightEndPoint->Coordinate.Y);
}