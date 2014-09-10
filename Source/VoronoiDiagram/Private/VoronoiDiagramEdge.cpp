// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramEdge.h"

TSharedPtr<FVoronoiDiagramEdge> FVoronoiDiagramEdge::CreateBisectingEdge(TSharedPtr<FVoronoiDiagramSite> SiteA, TSharedPtr<FVoronoiDiagramSite> SiteB)
{
    float dx, dy, a, b, c;
    
    dx = SiteB->GetCoordinate().X - SiteA->GetCoordinate().X;
    dy = SiteB->GetCoordinate().Y - SiteA->GetCoordinate().Y;
    
    c = SiteA->GetCoordinate().X * dx + SiteA->GetCoordinate().Y + (dx * dx + dy * dy) * 0.5f;

    if(FMath::Abs(dx) > FMath::Abs(dy))
    {
        a = 1.0f;
        b = dy / dx;
        c /= dx;
    }
    else
    {
        b = 1.0f;
        a = dx / dy;
        c /= dy;
    }
    
    TSharedPtr<FVoronoiDiagramEdge> Edge(new FVoronoiDiagramEdge());
    
    Edge->SetLeftSite(SiteA);
    Edge->SetRightSite(SiteB);
    SiteA->AddEdge(Edge);
    SiteB->AddEdge(Edge);

    Edge->a = a;
    Edge->b = b;
    Edge->c = c;
    
//    UE_LOG(LogVoronoiDiagram, Log, TEXT("Bisecting Edge: a: %f, b: %f, c: %f"), a, b, c);
    
    return Edge;
}

void FVoronoiDiagramEdge::MarkReadyForDeletion()
{
    bReadyForDeletion = true;
}

bool FVoronoiDiagramEdge::IsReadyForDeletion() const
{
    return bReadyForDeletion;
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagramEdge::GetLeftSite() const
{
    return LeftSite;
}

void FVoronoiDiagramEdge::SetLeftSite(TSharedPtr<FVoronoiDiagramSite> NewSite)
{
    LeftSite = NewSite;
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagramEdge::GetRightSite() const
{
    return RightSite;
}

void FVoronoiDiagramEdge::SetRightSite(TSharedPtr<FVoronoiDiagramSite> NewSite)
{
    RightSite = NewSite;
}

TSharedPtr<class FVoronoiDiagramVertex> FVoronoiDiagramEdge::GetLeftVertex() const
{
    return LeftVertex;
}

void FVoronoiDiagramEdge::SetLeftVertex(TSharedPtr<class FVoronoiDiagramVertex> NewVertex)
{
    LeftVertex = NewVertex;
}

TSharedPtr<class FVoronoiDiagramVertex> FVoronoiDiagramEdge::GetRightVertex() const
{
    return RightVertex;
}

void FVoronoiDiagramEdge::SetRightVertex(TSharedPtr<class FVoronoiDiagramVertex> NewVertex)
{
    RightVertex = NewVertex;
}

void FVoronoiDiagramEdge::ClipVertices(FIntRect Bounds)
{
    int32 MinX = Bounds.Width();
    int32 MinY = Bounds.Height();
    int32 MaxX = 0;
    int32 MaxY = 0;
    FIntPoint PointA, PointB;
    
    TSharedPtr<FVoronoiDiagramVertex> VertexA, VertexB;
    
    if(a == 1.0f && b >= 0.0f)
    {
        VertexA = GetRightVertex();
        VertexB = GetLeftVertex();
    }
    else
    {
        VertexA = GetLeftVertex();
        VertexB = GetRightVertex();
    }
    
    if (a == 1.0f)
    {
        PointA.Y = MinY;
        if (VertexA.IsValid() && VertexA->GetCoordinate().Y > MinY)
        {
            PointA.Y = VertexA->GetCoordinate().Y;
        }
        
        if (PointA.Y > MaxY)
        {
            return;
        }
        PointA.X = c - b * PointA.Y;

        PointB.Y = MaxY;
        if (VertexB.IsValid() && VertexB->GetCoordinate().Y < MaxY)
        {
            PointB.Y = VertexB->GetCoordinate().Y;
        }
        
        if (PointB.Y < MinY)
        {
            return;
        }
        PointB.X = c - b * PointB.Y;

        if ((PointA.X > MaxX && PointB.X > MaxX) || (PointA.X < MinX && PointB.X < MinX))
        {
            return;
        }

        if (PointA.X > MaxX)
        {
            PointA.X = MaxX;
            PointA.Y = (c - PointA.X)/b;
        }
        else if (PointA.X < MinX)
        {
            PointA.X = MinX;
            PointA.Y = (c - PointA.X)/b;
        }

        if (PointB.X > MaxX)
        {
            PointB.X = MaxX;
            PointB.Y = (c - PointB.X)/b;
        }
        else if (PointB.X < MinX)
        {
            PointB.X = MinX;
            PointB.Y = (c - PointB.X)/b;
        }
    }
    else
    {
        PointA.X = MinX;
        if (VertexA.IsValid() && VertexA->GetCoordinate().X > MinX)
        {
            PointA.X = VertexA->GetCoordinate().X;
        }
        
        if (PointA.X > MaxX)
        {
            return;
        }
        PointA.Y = c - a * PointA.X;

        PointB.X = MaxX;
        if (VertexB.IsValid() && VertexB->GetCoordinate().X < MaxX)
        {
            PointB.X = VertexB->GetCoordinate().X;
        }
        
        if (PointB.X < MinX)
        {
            return;
        }
        PointB.Y = c - a * PointB.X;

        if((PointA.Y > MaxY && PointB.Y > MaxY) || (PointA.Y < MinY && PointB.Y < MinY))
        {
            return;
        }

        if (PointA.Y > MaxY)
        {
            PointA.Y = MaxY;
            PointA.X = (c - PointA.Y)/a;
        }
        else if (PointA.Y < MinY)
        {
            PointA.Y = MinY;
            PointA.X = (c - PointA.Y)/a;
        }

        if (PointB.Y > MaxY)
        {
            PointB.Y = MaxY;
            PointB.X = (c - PointB.Y)/a;
        }
        else if (PointB.Y < MinY)
        {
            PointB.Y = MinY;
            PointB.X = (c - PointB.Y)/a;
        }
    }

    ClippedVertices.Init(2);
    if (VertexA == GetLeftVertex())
    {
        ClippedVertices[FVoronoiDiagram::LeftEdge] = FIntPoint(PointA.X, PointA.Y);
        ClippedVertices[FVoronoiDiagram::RightEdge] = FIntPoint(PointB.X, PointB.Y);
    }
    else
    {
        ClippedVertices[FVoronoiDiagram::RightEdge] = FIntPoint(PointA.X, PointA.Y);
        ClippedVertices[FVoronoiDiagram::LeftEdge] = FIntPoint(PointB.X, PointB.Y);
    }
}

TArray<FIntPoint> FVoronoiDiagramEdge::GetClippedVertices() const
{
    return ClippedVertices;
}

FString FVoronoiDiagramEdge::ToString()
{
    return FString::Printf(
        TEXT("Edge: Sites - %s, %s; End Vertices - %s, %s"),
        *(GetLeftSite()->ToString()), *(GetRightSite()->ToString()),
        GetLeftVertex().IsValid() ? "Present" : "null", GetRightVertex().IsValid() ? "Present" : "null"
    );
}

FVoronoiDiagramEdge::FVoronoiDiagramEdge()
: bReadyForDeletion(false)
, LeftVertex(nullptr)
, RightVertex(nullptr)
{}
