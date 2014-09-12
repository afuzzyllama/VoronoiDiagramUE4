// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramHalfEdge.h"


TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdge::CreatePtr(TSharedPtr<FVoronoiDiagramEdge> Edge, EVoronoiDiagramEdge::Type EdgeType)
{
    return TSharedPtr<FVoronoiDiagramHalfEdge>(new FVoronoiDiagramHalfEdge(Edge, EdgeType));
}

bool FVoronoiDiagramHalfEdge::IsLeftOf(FVector2D Point)
{
    TSharedPtr<FVoronoiDiagramSite> TopSite;
    bool bAbove, bFast, bRightOfSite;
    float dxp, dyp, dxs;
    
    TopSite = Edge->RightSite;
    bRightOfSite = Point.X > TopSite->GetCoordinate().X;

    if( bRightOfSite && EdgeType == EVoronoiDiagramEdge::Left)
    {
        return true;
    }
    
    if(!bRightOfSite && EdgeType == EVoronoiDiagramEdge::Right)
    {
        return false;
    }
    
    if (Edge->a == 1.0f)
    {
        dyp = Point.Y - TopSite->GetCoordinate().Y;
        dxp = Point.X - TopSite->GetCoordinate().X;

        bFast = false;
        if((!bRightOfSite && Edge->b < 0.0f) ||
           ( bRightOfSite && Edge->b >= 0.0f))
        {
            bAbove = dyp >= Edge->b * dxp;
            bFast = bAbove;
        }
        else 
        {
            bAbove = Point.X + Point.Y * Edge->b > Edge->c;
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
            dxs = TopSite->GetCoordinate().X - Edge->LeftSite->GetCoordinate().X;
            bAbove = Edge->b * (dxp * dxp - dyp * dyp) < dxs * dyp * (1.0f + 2.0f * dxp / dxs + Edge->b * Edge->b);
            if (Edge->b < 0.0f)
            {
                bAbove = !bAbove;
            }
        }
    }
    else // edge.b == 1.0
    {
        float t1, t2, t3, yl;
        yl = Edge->c - Edge->a * Point.X;
        t1 = Point.Y - yl;
        t2 = Point.X - TopSite->GetCoordinate().X;
        t3 = yl - TopSite->GetCoordinate().Y;
        bAbove = t1 * t1 > t2 * t2 + t3 * t3;
    }
    return EdgeType == EVoronoiDiagramEdge::Left ? bAbove : !bAbove;
}

bool FVoronoiDiagramHalfEdge::IsRightOf(FVector2D Point)
{
    return !IsLeftOf(Point);
}

bool FVoronoiDiagramHalfEdge::HasReferences()
{
    return EdgeListLeft.IsValid() || EdgeListRight.IsValid() || NextInPriorityQueue.IsValid();
}

// Never want to create a half edge directly since we are dealing with pointers
FVoronoiDiagramHalfEdge::FVoronoiDiagramHalfEdge(TSharedPtr<FVoronoiDiagramEdge> InEdge, EVoronoiDiagramEdge::Type InEdgeType)
: Edge(InEdge)
, EdgeType(InEdgeType)
, Vertex(nullptr)
, YStar(0.0f)
, EdgeListLeft(nullptr)
, EdgeListRight(nullptr)
, NextInPriorityQueue(nullptr)
{}
