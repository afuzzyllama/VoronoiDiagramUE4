// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramEdge.h"

TSharedPtr<FVoronoiDiagramEdge> FVoronoiDiagramEdge::DELETED(new FVoronoiDiagramEdge());

TSharedPtr<FVoronoiDiagramEdge> FVoronoiDiagramEdge::Bisect(TSharedPtr<FVoronoiDiagramSite> SiteA, TSharedPtr<FVoronoiDiagramSite> SiteB)
{
    float dx, dy;
    TSharedPtr<FVoronoiDiagramEdge> NewEdge(new FVoronoiDiagramEdge());

    NewEdge->LeftSite = SiteA;
    NewEdge->RightSite = SiteB;
    NewEdge->LeftEndPoint = nullptr;
    NewEdge->RightEndPoint = nullptr;
    
    dx = SiteB->GetCoordinate().X - SiteA->GetCoordinate().X;
    dy = SiteB->GetCoordinate().Y - SiteA->GetCoordinate().Y;
    
    NewEdge->c = SiteA->GetCoordinate().X * dx + SiteA->GetCoordinate().Y * dy + (dx * dx + dy * dy) * 0.5f;
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

void FVoronoiDiagramEdge::SetEndpoint(TSharedPtr<FVoronoiDiagramVertex> Vertex, EVoronoiDiagramEdge::Type EdgeType)
{
    if(EdgeType == EVoronoiDiagramEdge::Left)
    {
        LeftEndPoint = Vertex;
    }
    else
    {
        RightEndPoint = Vertex;
    }
}

FVoronoiDiagramEdge::FVoronoiDiagramEdge()
: Index(-1)
, a(0.0f)
, b(0.0f)
, c(0.0f)
, LeftEndPoint(nullptr)
, RightEndPoint(nullptr)
, LeftSite(nullptr)
, RightSite(nullptr)
{}