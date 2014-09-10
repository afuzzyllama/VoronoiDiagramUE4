// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramVertex.h"

TSharedPtr<FVoronoiDiagramVertex> FVoronoiDiagramVertex::FVoronoiDiagramVertex::Intersect(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdgeA, TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdgeB)
{
    TSharedPtr<FVoronoiDiagramEdge> EdgeA, EdgeB, Edge;
    TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
    float Determinant, IntersectionX, IntersectionY;
    bool bRightOfSite;
    
    EdgeA = HalfEdgeA->GetEdge();
    EdgeB = HalfEdgeB->GetEdge();
    
    if(!EdgeA.IsValid() || !EdgeB.IsValid())
    {
        return nullptr;
    }
    
    if(EdgeA->GetRightSite() == EdgeB->GetRightSite())
    {
        return nullptr;
    }
    
    Determinant = (EdgeA->a * EdgeB->b) - (EdgeA->b * EdgeB->a);
    
    const float AlmostZero = 0.0000000001f;
    if(-AlmostZero < Determinant && Determinant < AlmostZero )
    {
        // The edges are parallel
        return nullptr;
    }
    
    IntersectionX = (EdgeA->c * EdgeB->b - EdgeB->c * EdgeA->b)/Determinant;
    IntersectionY = (EdgeB->c * EdgeA->a - EdgeA->c * EdgeB->a)/Determinant;
    
    if(
        EdgeA->GetRightSite()->GetCoordinate().Y < EdgeB->GetRightSite()->GetCoordinate().Y ||
        (
            EdgeA->GetRightSite()->GetCoordinate().Y == EdgeB->GetRightSite()->GetCoordinate().Y &&
            EdgeA->GetRightSite()->GetCoordinate().X <  EdgeB->GetRightSite()->GetCoordinate().X
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
    
    bRightOfSite = IntersectionX >= Edge->GetRightSite()->GetCoordinate().X;
    
    if(
        ( bRightOfSite && HalfEdge->GetLeftRight() == FVoronoiDiagram::LeftEdge) ||
        (!bRightOfSite && HalfEdge->GetLeftRight() == FVoronoiDiagram::RightEdge)
    )
    {
        return nullptr;
    }
    
    return TSharedPtr<FVoronoiDiagramVertex>(new FVoronoiDiagramVertex(FIntPoint(IntersectionX, IntersectionY)));
}


int32 FVoronoiDiagramVertex::GetIndex() const
{
    return Index;
}

FVector2D FVoronoiDiagramVertex::GetCoordinate() const
{
    return Coordinate;
}

FString FVoronoiDiagramVertex::ToString()
{
    return FString::Printf(TEXT("Vertex: %i, %i"), GetCoordinate().X, GetCoordinate().Y);
}

FVoronoiDiagramVertex::FVoronoiDiagramVertex(FVector2D InCoordinate)
: Coordinate(InCoordinate)
{}