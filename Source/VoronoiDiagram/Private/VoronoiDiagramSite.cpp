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

void FVoronoiDiagramSite::GenerateCentroid(FIntRect Bounds)
{

    TArray<FVector2D> SortedVertices;
    TArray<FVoronoiDiagramGeneratedEdge> RemainingEdges;

    for(auto Itr(Edges.CreateConstIterator()); Itr; ++Itr)
    {
        TSharedPtr<FVoronoiDiagramEdge> CurrentEdge = (*Itr);

        // Don't add edge that is (0,0) -> (0,0).  Increment Index if no edge is removed, otherwise the remove should do this shifting for us
        if(
            FMath::RoundToInt(CurrentEdge->LeftClippedEndPoint.X) == 0 && FMath::RoundToInt(CurrentEdge->LeftClippedEndPoint.Y) == 0 &&
            FMath::RoundToInt(CurrentEdge->RightClippedEndPoint.X) == 0 && FMath::RoundToInt(CurrentEdge->RightClippedEndPoint.Y) == 0
        )
        {
            continue;
        }
        
        // For the centroid calculation, if there is an edge that is so small that it is almost a point, ignore it.  I believe this will make very little different when determining the centroid and will avoid rounding errors.
        if(
            FMath::RoundToInt(CurrentEdge->LeftClippedEndPoint.X) == FMath::RoundToInt(CurrentEdge->RightClippedEndPoint.X) &&
            FMath::RoundToInt(CurrentEdge->LeftClippedEndPoint.Y) == FMath::RoundToInt(CurrentEdge->RightClippedEndPoint.Y)
        )
        {
            continue;
        }

        RemainingEdges.Add(FVoronoiDiagramGeneratedEdge(CurrentEdge->Index, CurrentEdge->LeftClippedEndPoint, CurrentEdge->RightClippedEndPoint));
    }
    
    for(auto Itr(RemainingEdges.CreateConstIterator()); Itr; ++Itr)
    {
        FVoronoiDiagramGeneratedEdge CurrentEdge = *Itr;
        
        UE_LOG(LogVoronoiDiagram, Log, TEXT("Edge (%f, %f) -> (%f, %f)"), CurrentEdge.LeftEndPoint.X, CurrentEdge.LeftEndPoint.Y, CurrentEdge.RightEndPoint.X, CurrentEdge.RightEndPoint.Y);
    }
    
    FVector2D SearchVertex = RemainingEdges[0].RightEndPoint;
    RemainingEdges.Add(FVoronoiDiagramGeneratedEdge(RemainingEdges[0].Index, RemainingEdges[0].LeftEndPoint, FVector2D(-1.0f, -1.0f)));
    RemainingEdges.RemoveAt(0);

    // If one is equal to (X.X, Min/Max) or (Min/Max, Y.Y) find the other point that has Min/Max on the same axis and create the edge.  If Min == Min or Max == Max, then create edge, If Min -> Max, create two edges with the corner
    bool bDone = false;
    while(!bDone)
    {
        int32 RemoveIndex = -1;
        for(int32 Index = 0; Index < RemainingEdges.Num(); ++Index)
        {
            FVector2D CurrentLeftVertex = RemainingEdges[Index].LeftEndPoint;
            FVector2D CurrentRightVertex = RemainingEdges[Index].RightEndPoint;
            
            // (x, Min) -> (Min, y)
            // Min, Min Corner
            if(
                (FMath::RoundToInt(SearchVertex.Y) == 0.0f && FMath::RoundToInt(CurrentLeftVertex.X) == 0.0f) ||
                (FMath::RoundToInt(CurrentLeftVertex.Y) == 0.0f && FMath::RoundToInt(SearchVertex.X) == 0.0f)
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(0.0f, 0.0f));
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }
            
            if(
                (FMath::RoundToInt(SearchVertex.Y) == 0.0f && FMath::RoundToInt(CurrentRightVertex.X) == 0.0f) ||
                (FMath::RoundToInt(CurrentRightVertex.Y) == 0.0f && FMath::RoundToInt(SearchVertex.X) == 0.0f)
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(0.0f, 0.0f));
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // x, Min -> Max, y
            // Min, Max Corner
            if(
                (FMath::RoundToInt(SearchVertex.Y) == 0.0f && FMath::RoundToInt(CurrentLeftVertex.X) == Bounds.Width())||
                (FMath::RoundToInt(CurrentLeftVertex.Y) == 0.0f && FMath::RoundToInt(SearchVertex.X) == Bounds.Width())
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(Bounds.Width(), 0.0f));
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }
            
            if(
                (FMath::RoundToInt(SearchVertex.Y) == 0.0f && FMath::RoundToInt(CurrentRightVertex.X) == Bounds.Width()) ||
                (FMath::RoundToInt(CurrentRightVertex.Y) == 0.0f && FMath::RoundToInt(SearchVertex.X) == Bounds.Width())
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(Bounds.Width(), 0.0f));
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // x, Max -> Min, y
            // Max, Min Corner
            if(
                (FMath::RoundToInt(SearchVertex.Y) == Bounds.Height() && FMath::RoundToInt(CurrentLeftVertex.X) == 0.0f) ||
                (FMath::RoundToInt(CurrentLeftVertex.Y) == Bounds.Height() && FMath::RoundToInt(SearchVertex.X) == 0.0f)
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(0.0f, Bounds.Height()));
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }
            
            if(
                (FMath::RoundToInt(SearchVertex.Y) == Bounds.Height() && FMath::RoundToInt(CurrentRightVertex.X) == 0.0f) ||
                (FMath::RoundToInt(CurrentRightVertex.Y) == Bounds.Height() && FMath::RoundToInt(SearchVertex.X) == 0.0f)
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(0.0f, Bounds.Height()));
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // x, Max -> Max, y
            // Max, Max Corner
            if(
                (FMath::RoundToInt(SearchVertex.Y) == Bounds.Height() && FMath::RoundToInt(CurrentLeftVertex.X) == Bounds.Width()) ||
                (FMath::RoundToInt(CurrentLeftVertex.Y) == Bounds.Height() && FMath::RoundToInt(SearchVertex.X) == Bounds.Width())
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(Bounds.Width(), Bounds.Height()));
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }
            
            if(
                (FMath::RoundToInt(SearchVertex.Y) == Bounds.Height() && FMath::RoundToInt(CurrentRightVertex.X) == Bounds.Width()) ||
                (FMath::RoundToInt(CurrentRightVertex.Y) == Bounds.Height() && FMath::RoundToInt(SearchVertex.X) == Bounds.Width())
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(Bounds.Width(), Bounds.Height()));
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // Edges
            if(
                (FMath::RoundToInt(SearchVertex.X) == 0 && FMath::RoundToInt(CurrentLeftVertex.X) == 0) ||
                (FMath::RoundToInt(SearchVertex.X) == Bounds.Width() && FMath::RoundToInt(CurrentLeftVertex.X) == Bounds.Width()) ||
                (FMath::RoundToInt(SearchVertex.Y) == 0 && FMath::RoundToInt(CurrentLeftVertex.Y) == 0) ||
                (FMath::RoundToInt(SearchVertex.Y) == Bounds.Height() && FMath::RoundToInt(CurrentLeftVertex.Y) == Bounds.Height())
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }

            if(
                (FMath::RoundToInt(SearchVertex.X) == 0 && FMath::RoundToInt(CurrentRightVertex.X) == 0) ||
                (FMath::RoundToInt(SearchVertex.X) == Bounds.Width() && FMath::RoundToInt(CurrentRightVertex.X) == Bounds.Width()) ||
                (FMath::RoundToInt(SearchVertex.Y) == 0 && FMath::RoundToInt(CurrentRightVertex.Y) == 0) ||
                (FMath::RoundToInt(SearchVertex.Y) == Bounds.Height() && FMath::RoundToInt(CurrentRightVertex.Y) == Bounds.Height())
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // Not an edge case
            if(FMath::RoundToInt(CurrentLeftVertex.X) == FMath::RoundToInt(SearchVertex.X) && FMath::RoundToInt(CurrentLeftVertex.Y) == FMath::RoundToInt(SearchVertex.Y))
            {
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }

            if(FMath::RoundToInt(CurrentRightVertex.X) == FMath::RoundToInt(SearchVertex.X) && FMath::RoundToInt(CurrentRightVertex.Y) == FMath::RoundToInt(SearchVertex.Y))
            {
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
        }
        
        if(RemoveIndex != -1)
        {
            RemainingEdges.RemoveAt(RemoveIndex);
        }
        
        if(RemainingEdges.Num() == 0 || RemoveIndex == -1 || SearchVertex == FVector2D(-1.0f, -1.0f))
        {
            bDone = true;
        }
    }
    
    for(auto Itr(SortedVertices.CreateConstIterator()); Itr; ++Itr)
    {
        UE_LOG(LogVoronoiDiagram, Log, TEXT("Vertex (%f, %f)"), (*Itr).X, (*Itr).Y);
    }

    Centroid = FVector2D::ZeroVector;

    if(SortedVertices.Num() == 0)
    {
        // Something went wrong
        return;
    }

    FVector2D CurrentVertex;
    FVector2D NextVertex;
    float SignedArea = 0.0f;
    float PartialArea;
    
    // Use all vertices except the last one
    for(int32 Index = 0; Index < SortedVertices.Num() - 1; ++Index)
    {
        CurrentVertex = FVector2D(SortedVertices[Index]);
        NextVertex = FVector2D(SortedVertices[Index + 1]);

        PartialArea = CurrentVertex.X * NextVertex.Y - NextVertex.X * CurrentVertex.Y;
        SignedArea += PartialArea;

        Centroid = FVector2D(Centroid.X + (CurrentVertex.X + NextVertex.X) * PartialArea, Centroid.Y + (CurrentVertex.Y + NextVertex.Y) * PartialArea);
    }

    // Process last vertex
    CurrentVertex = SortedVertices[SortedVertices.Num() - 1];
    NextVertex = SortedVertices[0];
    PartialArea = (CurrentVertex.X * NextVertex.Y - NextVertex.X * CurrentVertex.Y);
    SignedArea += PartialArea;
    Centroid = FVector2D(Centroid.X + (CurrentVertex.X + NextVertex.X) * PartialArea, Centroid.Y + (CurrentVertex.Y + NextVertex.Y) * PartialArea);
    
    SignedArea *= 0.5f;
    Centroid = FVector2D( Centroid.X / (6.0f * SignedArea), Centroid.Y / (6.0f * SignedArea) );
    
    UE_LOG(LogVoronoiDiagram, Log, TEXT("Centroid: (%f, %f)"), Centroid.X, Centroid.Y);
}


FVector2D FVoronoiDiagramSite::GetCoordinate() const
{
    return Coordinate;
}
