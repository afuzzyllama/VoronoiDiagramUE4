// Copyright 2015 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramSite.h"

TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> FVoronoiDiagramSite::CreatePtr(int32 Index, FVector2D Coordinate)
{
	return TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe>(new FVoronoiDiagramSite(Index, Coordinate));
}

FVoronoiDiagramSite::FVoronoiDiagramSite(int32 InIndex, FVector2D InCoordinate)
: Index(InIndex)
, Coordinate(InCoordinate)
, bIsCorner(false)
, bIsEdge(false)
{}

float FVoronoiDiagramSite::GetDistanceFrom(TSharedPtr<IVoronoiDiagramPoint, ESPMode::ThreadSafe> Point)
{
    float dx, dy;

    dx = GetCoordinate().X - Point->GetCoordinate().X;
    dy = GetCoordinate().Y - Point->GetCoordinate().Y;

    return FMath::Sqrt(dx * dx + dy * dy);
}

void FVoronoiDiagramSite::GenerateCentroid(FIntRect Bounds)
{
    TArray<FVector2D> SortedVertices;

    // Gather all vertices from the edges
    // Solve for corners
    bool bHas_X_Min = false;
    bool bHas_X_Max = false;
    bool bHas_Min_Y = false;
    bool bHas_Max_Y = false;
	for(auto Itr(Edges.CreateConstIterator()); Itr; ++Itr)
    {
		TSharedPtr<FVoronoiDiagramEdge, ESPMode::ThreadSafe> CurrentEdge = (*Itr);

        // Don't add edge that is (0,0) -> (0,0).  Increment Index if no edge is removed, otherwise the remove should do this shifting for us
        if(
            FMath::IsNearlyEqual(CurrentEdge->LeftClippedEndPoint.X, FLT_MIN) && FMath::IsNearlyEqual(CurrentEdge->LeftClippedEndPoint.Y, FLT_MIN)  &&
            FMath::IsNearlyEqual(CurrentEdge->RightClippedEndPoint.X, FLT_MIN) && FMath::IsNearlyEqual(CurrentEdge->RightClippedEndPoint.Y, FLT_MIN)
        )
        {
            continue;
        }
        
        FVector2D LeftEndPoint = CurrentEdge->LeftClippedEndPoint;
        FVector2D RightEndPoint = CurrentEdge->RightClippedEndPoint;

        // (x, Min)
        if(FMath::IsNearlyZero(LeftEndPoint.Y, NOT_REALLY_KINDA_SMALL_NUMBER) || FMath::IsNearlyZero(RightEndPoint.Y, NOT_REALLY_KINDA_SMALL_NUMBER))
        {
            bHas_X_Min = true;
        }

        // (x, Max)
        if(FMath::IsNearlyEqual(LeftEndPoint.Y, Bounds.Height(), NOT_REALLY_KINDA_SMALL_NUMBER) || FMath::IsNearlyEqual(RightEndPoint.Y, Bounds.Height(), NOT_REALLY_KINDA_SMALL_NUMBER))
        {
            bHas_X_Max = true;
        }
        
        // (Min, y)
        if(FMath::IsNearlyZero(LeftEndPoint.X, NOT_REALLY_KINDA_SMALL_NUMBER) || FMath::IsNearlyZero(RightEndPoint.X, NOT_REALLY_KINDA_SMALL_NUMBER))
        {
            bHas_Min_Y = true;
        }

        // (Max, y)
        if(FMath::IsNearlyEqual(LeftEndPoint.X, Bounds.Width(), NOT_REALLY_KINDA_SMALL_NUMBER) || FMath::IsNearlyEqual(RightEndPoint.X, Bounds.Width(), NOT_REALLY_KINDA_SMALL_NUMBER))
        {
            bHas_Max_Y = true;
        }
        
        SortedVertices.Add(LeftEndPoint);
        SortedVertices.Add(RightEndPoint);
    }
    
    // Add corners if applicable
    // (x, Min) -> (Min, y)
    // Min, Min Corner
    if(bHas_X_Min && bHas_Min_Y)
    {
        SortedVertices.Add(FVector2D(0.0f, 0.0f));
        bIsCorner = true;
    }
    
    // x, Min -> Max, y
    // Min, Max Corner
    if(bHas_X_Min && bHas_Max_Y)
    {
        SortedVertices.Add(FVector2D(Bounds.Width(), 0.0f));
        bIsCorner = true;
    }
    
    // x, Max -> Min, y
    // Max, Min Corner
    if(bHas_X_Max && bHas_Min_Y)
    {
        SortedVertices.Add(FVector2D(0.0f, static_cast<float>(Bounds.Height())));
        bIsCorner = true;
    }
    
    // x, Max -> Max, y
    // Max, Max Corner
    if(bHas_X_Max && bHas_Max_Y)
    {
        SortedVertices.Add(FVector2D(static_cast<float>(Bounds.Width()), static_cast<float>(Bounds.Height())));
        bIsCorner = true;
    }
    
    if(bHas_X_Min || bHas_X_Max || bHas_Min_Y || bHas_Max_Y)
    {
        bIsEdge = true;
    }

    // Monotone Chain
    // Sort the vertices lexigraphically by X and then Y
    struct FSortVertex
    {
        bool operator()(const FVector2D& A, const FVector2D& B) const
        {
            if(A.X < B.X)
            {
                return true;
            }
            
            if(A.X > B.X)
            {
                return false;
            }
            
            if(A.Y < B.Y)
            {
                return true;
            }
            
            if(A.Y > B.Y)
            {
                return false;
            }
            return false;
        }
    };
    SortedVertices.Sort(FSortVertex());
    
    TArray<FVector2D> LowerHull;
    for(int32 i = 0; i < SortedVertices.Num(); ++i)
    {
        while(LowerHull.Num() >= 2 && (Cross( LowerHull[ LowerHull.Num() - 2 ], LowerHull[ LowerHull.Num() - 1 ], SortedVertices[i]) < 0.0f || FMath::IsNearlyZero(Cross( LowerHull[ LowerHull.Num() - 2 ], LowerHull[ LowerHull.Num() - 1 ], SortedVertices[i]))))
        {
            LowerHull.RemoveAt(LowerHull.Num() - 1);
        }
        LowerHull.Add(SortedVertices[i]);
    }
    
    TArray<FVector2D> UpperHull;
    for(int32 i = SortedVertices.Num() - 1; i >= 0; --i)
    {
        while(UpperHull.Num() >= 2 && (Cross( UpperHull[ UpperHull.Num() - 2 ], UpperHull[ UpperHull.Num() - 1 ], SortedVertices[i]) < 0.0f || FMath::IsNearlyZero(Cross( UpperHull[ UpperHull.Num() - 2 ], UpperHull[ UpperHull.Num() - 1 ], SortedVertices[i]))))
        {
            UpperHull.RemoveAt(UpperHull.Num() - 1);
        }
        UpperHull.Add(SortedVertices[i]);
    }
    
    // Remove last point because they are represented in the other list
    UpperHull.RemoveAt(UpperHull.Num() - 1);
    LowerHull.RemoveAt(LowerHull.Num() - 1);
    
    SortedVertices.Empty();
    SortedVertices.Append(LowerHull);
    SortedVertices.Append(UpperHull);



    // Calculate Centroid
    Centroid = FVector2D::ZeroVector;
    Vertices.Empty();
    Vertices.Append(SortedVertices);

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
  
//    UE_LOG(LogVoronoiDiagram, Log, TEXT("Centroid (%f, %f)"), Centroid.X, Centroid.Y);
//    if(Centroid.X == NAN || Centroid.Y == NAN)
//    {
//        UE_LOG(LogVoronoiDiagram, Log, TEXT("Centroid (%f, %f)"), Centroid.X, Centroid.Y);
//        for(auto Itr(SortedVertices.CreateConstIterator()); Itr; ++Itr)
//        {
//            UE_LOG(LogVoronoiDiagram, Log, TEXT("Vertex (%f, %f)"), (*Itr).X, (*Itr).Y);
//        }
//    }
}


FVector2D FVoronoiDiagramSite::GetCoordinate() const
{
    return Coordinate;
}

float FVoronoiDiagramSite::Cross(const FVector2D& O, const FVector2D& A, const FVector2D& B)
{
    return (A.X - O.X) * (B.Y - O.Y) - (A.Y - O.Y) * (B.X - O.X);
}
