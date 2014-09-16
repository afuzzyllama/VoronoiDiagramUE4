// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramSite.h"

#define NOT_REALLY_KINDA_SMALL_NUMBER (1.e-2f)

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
            FMath::IsNearlyZero(CurrentEdge->LeftClippedEndPoint.X) && FMath::IsNearlyZero(CurrentEdge->LeftClippedEndPoint.Y)  &&
            FMath::IsNearlyZero(CurrentEdge->RightClippedEndPoint.X) && FMath::IsNearlyZero(CurrentEdge->RightClippedEndPoint.Y)
        )
        {
            continue;
        }
        
//        // For the centroid calculation, if there is an edge that is so small that it is almost a point, ignore it.  I believe this will make very little different when determining the centroid and will avoid rounding errors.
//        if(
//            FMath::IsNearlyEqual(CurrentEdge->LeftClippedEndPoint.X, CurrentEdge->RightClippedEndPoint.X, NOT_REALLY_KINDA_SMALL_NUMBER) &&
//            FMath::IsNearlyEqual(CurrentEdge->LeftClippedEndPoint.Y, CurrentEdge->RightClippedEndPoint.Y, NOT_REALLY_KINDA_SMALL_NUMBER)
//        )
//        {
//            continue;
//        }

        RemainingEdges.Add(FVoronoiDiagramGeneratedEdge(CurrentEdge->Index, CurrentEdge->LeftClippedEndPoint, CurrentEdge->RightClippedEndPoint));
    }
    
//    for(auto Itr(RemainingEdges.CreateConstIterator()); Itr; ++Itr)
//    {
//        FVoronoiDiagramGeneratedEdge CurrentEdge = *Itr;
//        
//        UE_LOG(LogVoronoiDiagram, Log, TEXT("Edge (%f, %f) -> (%f, %f)"), CurrentEdge.LeftEndPoint.X, CurrentEdge.LeftEndPoint.Y, CurrentEdge.RightEndPoint.X, CurrentEdge.RightEndPoint.Y);
//    }
    
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
                (FMath::IsNearlyZero(SearchVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(CurrentLeftVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyZero(CurrentLeftVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER))
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
                (FMath::IsNearlyZero(SearchVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(CurrentRightVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyZero(CurrentRightVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER))
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
                (FMath::IsNearlyZero(SearchVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentLeftVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyZero(CurrentLeftVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(SearchVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER))
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
                (FMath::IsNearlyZero(SearchVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentRightVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyZero(CurrentRightVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(SearchVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER))
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(static_cast<float>(Bounds.Width()), 0.0f));
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // x, Max -> Min, y
            // Max, Min Corner
            if(
                (FMath::IsNearlyEqual(SearchVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(CurrentLeftVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyEqual(CurrentLeftVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER))
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(0.0f, static_cast<float>(Bounds.Height())));
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }
            
            if(
                (FMath::IsNearlyEqual(SearchVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(CurrentRightVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyEqual(CurrentRightVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER))
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(0.0f, static_cast<float>(Bounds.Height())));
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // x, Max -> Max, y
            // Max, Max Corner
            if(
                (FMath::IsNearlyEqual(SearchVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentLeftVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyEqual(CurrentLeftVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(SearchVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER))
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(static_cast<float>(Bounds.Width()), static_cast<float>(Bounds.Height())));
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }
            
            if(
                (FMath::IsNearlyEqual(SearchVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentRightVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyEqual(CurrentRightVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(SearchVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER))
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(FVector2D(static_cast<float>(Bounds.Width()), static_cast<float>(Bounds.Height())));
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // Edges
            if(
                (FMath::IsNearlyZero(SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(CurrentLeftVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyEqual(SearchVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentLeftVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyZero(SearchVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(CurrentLeftVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyEqual(SearchVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentLeftVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER))
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }

            if(
                (FMath::IsNearlyZero(SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(CurrentRightVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyEqual(SearchVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentRightVertex.X, static_cast<float>(Bounds.Width()), NOT_REALLY_KINDA_SMALL_NUMBER)) ||
                (FMath::IsNearlyZero(SearchVertex.Y) && FMath::IsNearlyZero(CurrentRightVertex.Y)) ||
                (FMath::IsNearlyEqual(SearchVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentRightVertex.Y, static_cast<float>(Bounds.Height()), NOT_REALLY_KINDA_SMALL_NUMBER))
            )
            {
                SortedVertices.Add(SearchVertex);
                SortedVertices.Add(CurrentRightVertex);
                SearchVertex = CurrentLeftVertex;
                RemoveIndex = Index;
                break;
            }
            
            // Not an edge case
            if(FMath::IsNearlyEqual(CurrentLeftVertex.X, SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentLeftVertex.Y, SearchVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER))
            {
                SortedVertices.Add(CurrentLeftVertex);
                SearchVertex = CurrentRightVertex;
                RemoveIndex = Index;
                break;
            }

            if(FMath::IsNearlyEqual(CurrentRightVertex.X, SearchVertex.X, NOT_REALLY_KINDA_SMALL_NUMBER) && FMath::IsNearlyEqual(CurrentRightVertex.Y, SearchVertex.Y, NOT_REALLY_KINDA_SMALL_NUMBER))
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
    
//    for(auto Itr(SortedVertices.CreateConstIterator()); Itr; ++Itr)
//    {
//        UE_LOG(LogVoronoiDiagram, Log, TEXT("Vertex (%f, %f)"), (*Itr).X, (*Itr).Y);
//    }

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
    
//    UE_LOG(LogVoronoiDiagram, Log, TEXT("Centroid: (%f, %f)"), Centroid.X, Centroid.Y);
}


FVector2D FVoronoiDiagramSite::GetCoordinate() const
{
    return Coordinate;
}

#undef NOT_REALLY_KINDA_SMALL_NUMBER
