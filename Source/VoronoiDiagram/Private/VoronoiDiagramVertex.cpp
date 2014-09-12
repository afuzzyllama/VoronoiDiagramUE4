// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramVertex.h"

TSharedPtr<FVoronoiDiagramVertex> FVoronoiDiagramVertex::CreatePtr(int32 Index, FVector2D Coordinate)
{
    return TSharedPtr<FVoronoiDiagramVertex>(new FVoronoiDiagramVertex(Index, Coordinate));
}

TSharedPtr<FVoronoiDiagramVertex> FVoronoiDiagramVertex::Intersect(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdgeA, TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdgeB)
{
    TSharedPtr<FVoronoiDiagramEdge> EdgeA, EdgeB, Edge;
    TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
    float Determinant, IntersectionX, IntersectionY;
    bool bRightOfSite;
    
    EdgeA = HalfEdgeA->Edge;
    EdgeB = HalfEdgeB->Edge;
    
    if(!EdgeA.IsValid() || !EdgeB.IsValid())
    {
        return nullptr;
    }
    
    if(EdgeA->RightSite.Get() == EdgeB->RightSite.Get())
    {
        return nullptr;
    }
    
    Determinant = (EdgeA->a * EdgeB->b) - (EdgeA->b * EdgeB->a);
    
    float AlmostZero = 0.0000000001f;
    if (-AlmostZero < Determinant && Determinant < AlmostZero)
    {
        // The edges are parallel
        return nullptr;
    }
    
    IntersectionX = (EdgeA->c * EdgeB->b - EdgeB->c * EdgeA->b) / Determinant;
    IntersectionY = (EdgeB->c * EdgeA->a - EdgeA->c * EdgeB->a) / Determinant;

    if(
        EdgeA->RightSite->GetCoordinate().Y < EdgeB->RightSite->GetCoordinate().Y ||
        (
            EdgeA->RightSite->GetCoordinate().Y == EdgeB->RightSite->GetCoordinate().Y &&
            EdgeA->RightSite->GetCoordinate().X < EdgeB->RightSite->GetCoordinate().X
        )
    )
    {
        HalfEdge = HalfEdgeA;
        Edge = EdgeA;
    }
    else
    {
        HalfEdge = HalfEdgeB;
        Edge = EdgeB;
    }
    
    bRightOfSite = IntersectionX >= Edge->RightSite->GetCoordinate().X;
    if(
        ( bRightOfSite && HalfEdge->EdgeType == EVoronoiDiagramEdge::Left) ||
        (!bRightOfSite && HalfEdge->EdgeType == EVoronoiDiagramEdge::Right)
    )
    {
        return nullptr;
    }
    
    return FVoronoiDiagramVertex::CreatePtr(-1, FVector2D(IntersectionX, IntersectionY));
}

FVoronoiDiagramVertex::FVoronoiDiagramVertex(int32 InIndex, FVector2D InCoordinate)
: Index(InIndex)
, Coordinate(InCoordinate)
{
    if(Coordinate.X == NAN || Coordinate.Y == NAN)
    {
        // This probably should not happen, but it will alert in the logs if it does
        UE_LOG(LogVoronoiDiagram, Error, TEXT("Contains NaN"));
    }
}

FVector2D FVoronoiDiagramVertex::GetCoordinate() const
{
    return Coordinate;
}
