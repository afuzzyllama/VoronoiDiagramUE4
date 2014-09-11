// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramHalfEdge.h"


TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdge::CreatePtr(TSharedPtr<FVoronoiDiagramEdge> Edge, EVoronoiDiagramEdge::Type EdgeType)
{
    return TSharedPtr<FVoronoiDiagramHalfEdge>(new FVoronoiDiagramHalfEdge(Edge, EdgeType));
}

bool FVoronoiDiagramHalfEdge::IsLeftOf(TSharedPtr<FVoronoiDiagramPoint> Point)
{
    TSharedPtr<FVoronoiDiagramPoint> TopSite;
    bool bAbove, bFast, bRightOfSite;
    float dxp, dyp, dxs;
    
    TopSite = Edge->RightRegion;
    bRightOfSite = Point->Coordinate.X > TopSite->Coordinate.X;

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
        dyp = Point->Coordinate.Y - TopSite->Coordinate.Y;
        dxp = Point->Coordinate.X - TopSite->Coordinate.X;

        bFast = false;
        if((!bRightOfSite && Edge->b < 0.0f) ||
           ( bRightOfSite && Edge->b >= 0.0f))
        {
            bAbove = dyp >= Edge->b * dxp;
            bFast = bAbove;
        }
        else 
        {
            bAbove = Point->Coordinate.X + Point->Coordinate.Y * Edge->b > Edge->c;
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
            dxs = TopSite->Coordinate.X - Edge->LeftRegion->Coordinate.X;
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
        yl = Edge->c - Edge->a * Point->Coordinate.X;
        t1 = Point->Coordinate.Y - yl;
        t2 = Point->Coordinate.X - TopSite->Coordinate.X;
        t3 = yl - TopSite->Coordinate.Y;
        bAbove = t1 * t1 > t2 * t2 + t3 * t3;
    }
    return EdgeType == EVoronoiDiagramEdge::Left ? bAbove : !bAbove;
}

bool FVoronoiDiagramHalfEdge::IsRightOf(TSharedPtr<FVoronoiDiagramPoint> Point)
{
    return !IsLeftOf(Point);
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
