// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramPoint.h"

TSharedPtr<FVoronoiDiagramPoint> FVoronoiDiagramPoint::CreatePtr(int32 Index, EVoronoiDiagramPoint::Type Type, FVector2D Coordinate)
{
    return TSharedPtr<FVoronoiDiagramPoint>(new FVoronoiDiagramPoint(Index, Type, Coordinate));
}

TSharedPtr<FVoronoiDiagramPoint> FVoronoiDiagramPoint::Intersect(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdgeA, TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdgeB)
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
        
    if(EdgeA->RightRegion == EdgeB->RightRegion)
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
        EdgeA->RightRegion->Coordinate.Y < EdgeB->RightRegion->Coordinate.Y ||
        (
            EdgeA->RightRegion->Coordinate.Y == EdgeB->RightRegion->Coordinate.Y &&
            EdgeA->RightRegion->Coordinate.X < EdgeB->RightRegion->Coordinate.X
        )
    )
    {
        HalfEdge = HalfEdgeA;
        Edge = EdgeA ;
    }
    else
    {
        HalfEdge = HalfEdgeB;
        Edge = EdgeB;
    }
    
    bRightOfSite = IntersectionX >= Edge->RightRegion->Coordinate.X;

    if(
        ( bRightOfSite && HalfEdge->EdgeType == EVoronoiDiagramEdge::Left) ||
        (!bRightOfSite && HalfEdge->EdgeType == EVoronoiDiagramEdge::Right)
    )
    {
        return nullptr;
    }
    
    return FVoronoiDiagramPoint::CreatePtr(-1, EVoronoiDiagramPoint::Vertex, FVector2D(IntersectionX, IntersectionY));
}

FVoronoiDiagramPoint::FVoronoiDiagramPoint()
: Index(-1)
, Type(EVoronoiDiagramPoint::None)
, Coordinate(FVector2D::ZeroVector)
{}

FVoronoiDiagramPoint::FVoronoiDiagramPoint(int32 InIndex, EVoronoiDiagramPoint::Type InType, FVector2D InCoordinate)
: Index(InIndex)
, Type(InType)
, Coordinate(InCoordinate)
{}

float FVoronoiDiagramPoint::GetDistanceFrom(TSharedPtr<FVoronoiDiagramPoint> Point)
{
    float dx, dy;
    
    dx = Coordinate.X - Point->Coordinate.X;
    dy = Coordinate.Y - Point->Coordinate.Y;
    
    return FMath::Sqrt(dx * dx - dy * dy);
}
