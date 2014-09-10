// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramHalfEdge.h"

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdge::CreateEmptyHalfEdgePtr()
{
    return TSharedPtr<FVoronoiDiagramHalfEdge>(new FVoronoiDiagramHalfEdge());
}

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdge::CreateHalfEdgePtr(TSharedPtr<FVoronoiDiagramEdge> Edge, int32 LeftRight)
{
    return TSharedPtr<FVoronoiDiagramHalfEdge>(new FVoronoiDiagramHalfEdge(Edge, LeftRight));
}

void FVoronoiDiagramHalfEdge::AttemptToReset(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    if(HalfEdge->GetLeftNeighbor().IsValid() || HalfEdge->GetRightNeighbor().IsValid() || HalfEdge->GetNextInQueue().IsValid())
    {
        return;
    }
    
    HalfEdge.Reset();
    HalfEdge = nullptr;
}

float FVoronoiDiagramHalfEdge::GetYStar() const
{
    return YStar;
}

void FVoronoiDiagramHalfEdge::SetYStar(float NewYStar)
{
    YStar = NewYStar;
}

TSharedPtr<FVoronoiDiagramVertex> FVoronoiDiagramHalfEdge::GetVertex() const
{
    return Vertex;
}

void FVoronoiDiagramHalfEdge::SetVertex(TSharedPtr<class FVoronoiDiagramVertex> NewVertex)
{
    Vertex = NewVertex;
}

bool FVoronoiDiagramHalfEdge::IsLeftOf(FVector2D Point) const
{
    TSharedPtr<FVoronoiDiagramSite> TopSite;
    bool bRightOfSite, bAbove, bFast;
    float dxp, dyp, dxs, t1, t2, t3, yl;
    
    TopSite = Edge->GetRightSite();
    bRightOfSite = Point.X > TopSite->GetCoordinate().X;
    
    if(bRightOfSite && LeftRight == FVoronoiDiagram::LeftEdge)
    {
        return true;
    }
    
    if(!bRightOfSite && LeftRight == FVoronoiDiagram::RightEdge)
    {
        return false;
    }
    
    if(Edge->a == 1.0f)
    {
        dyp = Point.Y - TopSite->GetCoordinate().Y;
        dxp = Point.X - TopSite->GetCoordinate().X;
        bFast = false;
        if(
            (!bRightOfSite && Edge->b <  0.0f) ||
            ( bRightOfSite && Edge->b >= 0.0f)
        )
        {
            bAbove = dyp >= Edge->b * dxp;
            bFast = bAbove;
        }
        else
        {
            bAbove = (Point.X + Point.Y * Edge->b) > Edge->c;

            if(Edge->b < 0.0f)
            {
                bAbove = !bAbove;
            }
            
            if(!bAbove)
            {
                bFast = true;
            }
        }
        
        if(!bFast)
        {
            dxs = TopSite->GetCoordinate().X - Edge->GetLeftSite()->GetCoordinate().X;
            bAbove = (Edge->b * (dxp * dxp - dyp * dyp)) < (dxs * dyp * (1.0f + 2.0f * dxp / dxs + Edge->b * Edge->b));
            if(Edge->b < 0.0f)
            {
                bAbove = !bAbove;
            }
        }
    }
    else
    {
        // Edge->b == 1.0f
        yl = Edge->c - Edge->a * Point.X;
        t1 = Point.Y - yl;
        t2 = Point.X - TopSite->GetCoordinate().X;
        t3 = yl - TopSite->GetCoordinate().Y;
        bAbove = (t1 * t1) > (t2 * t2 + t3 * t3);
    }
    
    return LeftRight == FVoronoiDiagram::LeftEdge ? bAbove : !bAbove;
}

TSharedPtr<FVoronoiDiagramEdge> FVoronoiDiagramHalfEdge::GetEdge()
{
    return Edge;
}

void FVoronoiDiagramHalfEdge::SetEdge(TSharedPtr<FVoronoiDiagramEdge> NewEdge)
{
    Edge = NewEdge;
}

int32 FVoronoiDiagramHalfEdge::GetLeftRight()
{
    return LeftRight;
}

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdge::GetLeftNeighbor() const
{
    return LeftNeighbor;
}

void FVoronoiDiagramHalfEdge::SetLeftNeighbor(TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge)
{
    LeftNeighbor = NewHalfEdge;
}

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdge::GetRightNeighbor() const
{
    return RightNeighbor;
}

void FVoronoiDiagramHalfEdge::SetRightNeighbor(TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge)
{
    RightNeighbor = NewHalfEdge;
}

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdge::GetNextInQueue() const
{
    return NextInQueue;
}

void FVoronoiDiagramHalfEdge::SetNextInQueue(TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge)
{
    NextInQueue = NewHalfEdge;
}

FString FVoronoiDiagramHalfEdge::ToString()
{
    return FString::Printf(TEXT("Half Edge - %s; Vertex: %s"), GetLeftRight() == FVoronoiDiagram::LeftEdge ? *(FString("Left")) : (GetLeftRight() == FVoronoiDiagram::RightEdge ? *(FString("Right")) : *(FString("None"))), GetVertex().IsValid() ? *(GetVertex()->ToString()) : *(FString("null")));
}

FVoronoiDiagramHalfEdge::FVoronoiDiagramHalfEdge()
{
    FVoronoiDiagramHalfEdge(nullptr, -1);
}

FVoronoiDiagramHalfEdge::FVoronoiDiagramHalfEdge(TSharedPtr<FVoronoiDiagramEdge> InEdge, int32 InLeftRight)
: Edge(InEdge)
, LeftRight(InLeftRight)
, Vertex(nullptr)
, NextInQueue(nullptr)
{}
