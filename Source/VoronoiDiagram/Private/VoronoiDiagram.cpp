// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramModule.h"
#include "VoronoiDiagram.h"

////////////////////////////////////////////////////////////////////////////////
// FVoronoiDiagramEdgeList

class FVoronoiDiagramEdgeList
{
public:
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> Hash;
    TSharedPtr<FVoronoiDiagramHalfEdge> LeftEnd;
    TSharedPtr<FVoronoiDiagramHalfEdge> RightEnd;
    TSharedPtr<FVector2D> MinimumValues, DeltaValues;
    
    FVoronoiDiagramEdgeList(int32 NumberOfSites, TSharedPtr<FVector2D> InMinimumValues, TSharedPtr<FVector2D> InDeltaValues)
    : MinimumValues(InMinimumValues)
    , DeltaValues(InDeltaValues)
    {
        for(int32 i = 0; i < 2 * FMath::Sqrt(NumberOfSites); ++i)
        {
            Hash.Add(nullptr);
        }
        
        LeftEnd = FVoronoiDiagramHalfEdge::CreatePtr(nullptr, EVoronoiDiagramEdge::None);
        RightEnd = FVoronoiDiagramHalfEdge::CreatePtr(nullptr, EVoronoiDiagramEdge::None);
        
        LeftEnd->EdgeListLeft = nullptr;
        LeftEnd->EdgeListRight = RightEnd;
        
        RightEnd->EdgeListLeft = LeftEnd;
        RightEnd->EdgeListRight = nullptr;
        
        Hash[0] = LeftEnd;
        Hash[Hash.Num() - 1] = RightEnd;
    }
    
    void Insert(TSharedPtr<FVoronoiDiagramHalfEdge> LeftBound, TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge)
    {
        NewHalfEdge->EdgeListLeft = LeftBound;
        NewHalfEdge->EdgeListRight = LeftBound->EdgeListRight;
        LeftBound->EdgeListRight->EdgeListLeft = NewHalfEdge;
        LeftBound->EdgeListRight = NewHalfEdge;
    }
    
    void Delete(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
    {
        HalfEdge->EdgeListLeft->EdgeListRight = HalfEdge->EdgeListRight;
        HalfEdge->EdgeListRight->EdgeListLeft = HalfEdge->EdgeListLeft;
        HalfEdge->Edge = FVoronoiDiagramEdge::DELETED;
        HalfEdge->EdgeListLeft = nullptr;
        HalfEdge->EdgeListRight = nullptr;
    }

    TSharedPtr<FVoronoiDiagramHalfEdge> GetFromHash(int32 Bucket)
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
        
        if(Bucket < 0 || Bucket >= Hash.Num())
        {
            return nullptr;
        }
        
        HalfEdge = Hash[Bucket];
        if(HalfEdge.IsValid() && HalfEdge->Edge == FVoronoiDiagramEdge::DELETED)
        {
            // Edge ready for deletion, return null instead
            Hash[Bucket] = nullptr;
            
            // Cannot delete half edge yet, so just return nullptr at this point
            return nullptr;
        }

        return HalfEdge;
    }
    
    TSharedPtr<FVoronoiDiagramHalfEdge> GetLeftBoundFrom(FVector2D Point)
    {
        int32 Bucket;
        TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
        
        Bucket = (Point.X - MinimumValues->X) / DeltaValues->X * Hash.Num();
        
        if(Bucket < 0)
        {
            Bucket = 0;
        }
        
        if(Bucket >= Hash.Num())
        {
            Bucket = Hash.Num() - 1;
        }
        
        HalfEdge = GetFromHash(Bucket);
        if(!HalfEdge.IsValid())
        {
            int32 Index = 1;
            while(true)
            {
                HalfEdge = GetFromHash(Bucket - Index);
                if(HalfEdge.IsValid())
                {
                    break;
                }
                
                HalfEdge = GetFromHash(Bucket + Index);
                if(HalfEdge.IsValid())
                {
                    break;
                }
                
                Index++;

                // Infinite loop check
                if((Bucket - Index) < 0 && (Bucket + Index) >= Hash.Num())
                {
                    UE_LOG(LogVoronoiDiagram, Log, TEXT("(Bucket - Index) < 0 && (Bucket + Index) >= Hash.Num())"));
                    UE_LOG(LogVoronoiDiagram, Log, TEXT("(%i) < 0 && (%i) >= %i)"), Bucket - Index, Bucket + Index, Hash.Num());
                    check(false);
                }
            }
        }
        
        // If we are at the left end or if we are not at the right end of the half edge is left of the passed in point
        if(HalfEdge.Get() == LeftEnd.Get() || (HalfEdge.Get() != RightEnd.Get() && HalfEdge->IsLeftOf(Point)))
        {
            do
            {
                HalfEdge = HalfEdge->EdgeListRight;
            }
            while( HalfEdge != RightEnd && HalfEdge->IsLeftOf(Point));
            HalfEdge = HalfEdge->EdgeListLeft;
        }
        else
        {
            // If we are at the right end or if we are not at the left end of the half edge is right of the passed in point
            do
            {
                HalfEdge = HalfEdge->EdgeListLeft;
            }
            while( HalfEdge != LeftEnd && HalfEdge->IsRightOf(Point));
        }
        
        // Update the hash table and reference counts. Excludes left and right end
        if(Bucket > 0 && Bucket < Hash.Num() - 1)
        {
            Hash[Bucket] = HalfEdge;
        }
        return HalfEdge;
    }
};


// End of FVoronoiDiagramEdgeList
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FVoronoiDiagramHalfEdgePriorityQueue

class FVoronoiDiagramPriorityQueue
{
public:
    int32 MinimumBucket, Count;
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> Hash;
    TSharedPtr<FVector2D> MinimumValues, DeltaValues;
    
    FVoronoiDiagramPriorityQueue(int32 NumberOfSites, TSharedPtr<FVector2D> InMinimumValues, TSharedPtr<FVector2D> InDeltaValues)
    : MinimumValues(InMinimumValues)
    , DeltaValues(InDeltaValues)
    , MinimumBucket(0)
    , Count(0)
    {
        // Create an array full of dummies that represent the beginning of a bucket
        for(int32 i = 0; i < 4 * FMath::Sqrt(NumberOfSites); ++i)
        {
            Hash.Add(FVoronoiDiagramHalfEdge::CreatePtr(nullptr, EVoronoiDiagramEdge::None));
            Hash[i]->NextInPriorityQueue = nullptr;
        }
    }

    int32 GetBucket(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
    {
        int32 Bucket;
        
        Bucket = (HalfEdge->YStar - MinimumValues->Y) / DeltaValues->Y * Hash.Num();
        if(Bucket < 0)
        {
            Bucket = 0;
        }
        
        if(Bucket >= Hash.Num())
        {
            Bucket = Hash.Num() - 1;
        }
        
        return Bucket;
    }
    
    void Insert(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> Previous, Next;
        int32 InsertionBucket = GetBucket(HalfEdge);
        
        if(InsertionBucket < MinimumBucket)
        {
            MinimumBucket = InsertionBucket;
        }
     
        // Start at the beginning of the bucket and find where the half edge should go
        Previous = Hash[InsertionBucket];
        Next = Previous->NextInPriorityQueue;
        while(
            Next.IsValid() &&
            (
                HalfEdge->YStar > Next->YStar ||
                (HalfEdge->YStar == Next->YStar && HalfEdge->Vertex->GetCoordinate().X > Next->Vertex->GetCoordinate().X)
            )
        )
        {
            Previous = Next;
            Next = Previous->NextInPriorityQueue;
        }
        
        HalfEdge->NextInPriorityQueue = Previous->NextInPriorityQueue;
        Previous->NextInPriorityQueue = HalfEdge;
        Count++;
    }
    
    void Delete(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> Previous;
        int32 RemovalBucket = GetBucket(HalfEdge);
        
        if(HalfEdge->Vertex.IsValid())
        {
            Previous = Hash[RemovalBucket];
            while(Previous->NextInPriorityQueue != HalfEdge)
            {
                Previous = Previous->NextInPriorityQueue;
            }
            
            Previous->NextInPriorityQueue = HalfEdge->NextInPriorityQueue;
            Count--;
            
            HalfEdge->Vertex = nullptr;
            HalfEdge->NextInPriorityQueue;
            
            if(!HalfEdge->HasReferences())
            {
                HalfEdge.Reset();
            }
        }
    }
    
    bool IsEmpty()
    {
        return Count == 0;
    }
    
    FVector2D GetMinimumBucketFirstPoint()
    {
        while(MinimumBucket < Hash.Num() - 1 && !Hash[MinimumBucket]->NextInPriorityQueue.IsValid())
        {
            MinimumBucket++;
        }

        return FVector2D(
            Hash[MinimumBucket]->NextInPriorityQueue->Vertex->GetCoordinate().X,
            Hash[MinimumBucket]->NextInPriorityQueue->YStar
        );
    }
    
    TSharedPtr<FVoronoiDiagramHalfEdge> RemoveAndReturnMinimum()
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> MinEdge;

        MinEdge = Hash[MinimumBucket]->NextInPriorityQueue;
        Hash[MinimumBucket]->NextInPriorityQueue = MinEdge->NextInPriorityQueue;
        Count--;
        
        MinEdge->NextInPriorityQueue = nullptr;
        
        return MinEdge;
    }
};

// End of FVoronoiDiagramHalfEdgePriorityQueue
////////////////////////////////////////////////////////////////////////////////


FVoronoiDiagram::FVoronoiDiagram(FIntRect InBounds)
: Bounds(InBounds)
{}

bool FVoronoiDiagram::AddPoints(const TArray<FIntPoint>& Points)
{
    for(auto Itr(Points.CreateConstIterator()); Itr; ++Itr)
    {
        FIntPoint CurrentPoint = *Itr;
        if(!Bounds.Contains(CurrentPoint))
        {
            UE_LOG(LogVoronoiDiagram, Error, TEXT("Point (%i, %i) out of diagram bounds (%i, %i)"), CurrentPoint.X, CurrentPoint.Y, Bounds.Width(), Bounds.Height());
            return false;
        }
    }
    
    for(auto Itr(Points.CreateConstIterator()); Itr; ++Itr)
    {
        FIntPoint CurrentPoint = *Itr;
        Sites.Add(FVoronoiDiagramSite::CreatePtr(Sites.Num(), FVector2D(static_cast<float>(CurrentPoint.X), static_cast<float>(CurrentPoint.Y))));
    }
    
    struct FSortSite
    {
        bool operator()(const TSharedPtr<FVoronoiDiagramSite>& A, const TSharedPtr<FVoronoiDiagramSite>& B) const
        {
            if(FMath::RoundToInt(A->GetCoordinate().Y) < FMath::RoundToInt(B->GetCoordinate().Y))
            {
                return true;
            }
            
            if(FMath::RoundToInt(A->GetCoordinate().Y) > FMath::RoundToInt(B->GetCoordinate().Y))
            {
                return false;
            }
            
            if(FMath::RoundToInt(A->GetCoordinate().X) < FMath::RoundToInt(B->GetCoordinate().X))
            {
                return true;
            }
            
            if(FMath::RoundToInt(A->GetCoordinate().X) > FMath::RoundToInt(B->GetCoordinate().X))
            {
                return false;
            }
            
            return false;
        }
    };
    Sites.Sort(FSortSite());
    
    FVector2D CurrentMin(FLT_MAX, FLT_MAX);
    FVector2D CurrentMax(FLT_MIN, FLT_MIN);
    for(auto Itr(Sites.CreateConstIterator()); Itr; ++Itr)
    {
        FVector2D CurrentPoint = (*Itr)->GetCoordinate();
        if(CurrentPoint.X < CurrentMin.X)
        {
            CurrentMin.X = CurrentPoint.X;
        }

        if(CurrentPoint.X > CurrentMax.X)
        {
            CurrentMax.X = CurrentPoint.X;
        }
        
        if(CurrentPoint.Y < CurrentMin.Y)
        {
            CurrentMin.Y = CurrentPoint.Y;
        }

        if(CurrentPoint.Y > CurrentMax.Y)
        {
            CurrentMax.Y = CurrentPoint.Y;
        }
    }
    
    MinValues = MakeShareable(new FVector2D(CurrentMin));
    MaxValues = MakeShareable(new FVector2D(CurrentMax));
    DeltaValues = MakeShareable(new FVector2D(CurrentMax.X - CurrentMin.X, CurrentMax.Y - CurrentMin.Y));

    return true;
}

void FVoronoiDiagram::GenerateSites(TArray<FVoronoiDiagramGeneratedSite>& OutSites)
{
    // Fortune's Algorithm
    int32 NumGeneratedEdges = 0;
    int32 NumGeneratedVertices = 0;
    CurrentSiteIndex = 0;

    FVoronoiDiagramPriorityQueue PriorityQueue(Sites.Num(), MinValues, DeltaValues);
    FVoronoiDiagramEdgeList EdgeList(Sites.Num(), MinValues, DeltaValues);

    FVector2D CurrentIntersectionStar;
    TSharedPtr<FVoronoiDiagramSite> CurrentSite, BottomSite, TopSite, TempSite;
    TSharedPtr<FVoronoiDiagramVertex> v, Vertex;
    TSharedPtr<FVoronoiDiagramHalfEdge> LeftBound, RightBound, LeftLeftBound, RightRightBound, Bisector;
    TSharedPtr<FVoronoiDiagramEdge> Edge;
    EVoronoiDiagramEdge::Type EdgeType;
    
    TArray<TSharedPtr<FVoronoiDiagramEdge>> GeneratedEdges;

    bool bDone = false;
    BottomMostSite = GetNextSite();
    CurrentSite = GetNextSite();
    while(!bDone)
    {
        if(!PriorityQueue.IsEmpty())
        {
            CurrentIntersectionStar = PriorityQueue.GetMinimumBucketFirstPoint();
        }
        
        if(
            CurrentSite.IsValid() &&
            (
                PriorityQueue.IsEmpty() ||
                CurrentSite->GetCoordinate().Y < CurrentIntersectionStar.Y ||
                (
                    CurrentSite->GetCoordinate().Y == CurrentIntersectionStar.Y &&
                    CurrentSite->GetCoordinate().X <  CurrentIntersectionStar.X
                )
            )
        )
        {
            // Current processed site is the smallest
            LeftBound = EdgeList.GetLeftBoundFrom(CurrentSite->GetCoordinate());
            RightBound = LeftBound->EdgeListRight;
            BottomSite = GetRightRegion(LeftBound);

            Edge = FVoronoiDiagramEdge::Bisect(BottomSite, CurrentSite);
            Edge->Index = NumGeneratedEdges;
            NumGeneratedEdges++;
            
            GeneratedEdges.Add(Edge);
            
            Bisector = FVoronoiDiagramHalfEdge::CreatePtr(Edge, EVoronoiDiagramEdge::Left);
            EdgeList.Insert(LeftBound, Bisector);
            
            Vertex = FVoronoiDiagramVertex::Intersect(LeftBound, Bisector);
            if(Vertex.IsValid())
            {
                PriorityQueue.Delete(LeftBound);
                
                LeftBound->Vertex = Vertex;
                LeftBound->YStar = Vertex->GetCoordinate().Y + CurrentSite->GetDistanceFrom(Vertex);

                PriorityQueue.Insert(LeftBound);
            }
            
            LeftBound = Bisector;
            Bisector = FVoronoiDiagramHalfEdge::CreatePtr(Edge, EVoronoiDiagramEdge::Right);
            
            EdgeList.Insert(LeftBound, Bisector);
            
            Vertex = FVoronoiDiagramVertex::Intersect(Bisector, RightBound);
            if(Vertex.IsValid())
            {
                Bisector->Vertex = Vertex;
                Bisector->YStar = Vertex->GetCoordinate().Y + CurrentSite->GetDistanceFrom(Vertex);

                PriorityQueue.Insert(Bisector);
            }
            
            CurrentSite = GetNextSite();
        }
        else if(PriorityQueue.IsEmpty() == false)
        {
            // Current intersection is the smallest
            LeftBound = PriorityQueue.RemoveAndReturnMinimum();
            LeftLeftBound = LeftBound->EdgeListLeft;
            RightBound = LeftBound->EdgeListRight;
            RightRightBound = RightBound->EdgeListRight;
            BottomSite = GetLeftRegion(LeftBound);
            TopSite = GetRightRegion(RightBound);
            
            // These three sites define a Delaunay triangle
            // Bottom, Top, EdgeList.GetRightRegion(RightBound);
//            UE_LOG(LogVoronoiDiagram, Log, TEXT("Delaunay triagnle: (%f, %f), (%f, %f), (%f, %f)"),
//                Bottom->Coordinate.X, Bottom->Coordinate.Y,
//                Top->Coordinate.X, Top->Coordinate.Y,
//                EdgeList.GetRightRegion(LeftBound)->Coordinate.X,
//                EdgeList.GetRightRegion(LeftBound)->Coordinate.Y);
            
            v = LeftBound->Vertex;
            v->Index = NumGeneratedVertices;
            NumGeneratedVertices++;
            
            LeftBound->Edge->SetEndpoint(v, LeftBound->EdgeType);
            RightBound->Edge->SetEndpoint(v, RightBound->EdgeType);
            
            EdgeList.Delete(LeftBound);
            
            PriorityQueue.Delete(RightBound);
            EdgeList.Delete(RightBound);
            
            EdgeType = EVoronoiDiagramEdge::Left;
            if(BottomSite->GetCoordinate().Y > TopSite->GetCoordinate().Y)
            {
                TempSite = BottomSite;
                BottomSite = TopSite;
                TopSite = TempSite;
                EdgeType = EVoronoiDiagramEdge::Right;
            }
            
            Edge = FVoronoiDiagramEdge::Bisect(BottomSite, TopSite);
            Edge->Index = NumGeneratedEdges;
            NumGeneratedEdges++;
            
            GeneratedEdges.Add(Edge);
            
            Bisector = FVoronoiDiagramHalfEdge::CreatePtr(Edge, EdgeType);
            EdgeList.Insert(LeftLeftBound, Bisector);
            
            if(EdgeType == EVoronoiDiagramEdge::Left)
            {
                Edge->SetEndpoint(v, EVoronoiDiagramEdge::Right);
            }
            else
            {
                Edge->SetEndpoint(v, EVoronoiDiagramEdge::Left);
            }
            
            Vertex = FVoronoiDiagramVertex::Intersect(LeftLeftBound, Bisector);
            if(Vertex.IsValid())
            {
                PriorityQueue.Delete(LeftLeftBound);

                LeftLeftBound->Vertex = Vertex;
                LeftLeftBound->YStar = Vertex->GetCoordinate().Y + BottomSite->GetDistanceFrom(Vertex);

                PriorityQueue.Insert(LeftLeftBound);
            }
            
            Vertex = FVoronoiDiagramVertex::Intersect(Bisector, RightRightBound);
            if(Vertex.IsValid())
            {
                Bisector->Vertex = Vertex;
                Bisector->YStar = Vertex->GetCoordinate().Y + BottomSite->GetDistanceFrom(Vertex);
                
                PriorityQueue.Insert(Bisector);
            }
        }
        else
        {
            bDone = true;
        }
    }
    
    OutSites.Empty();
    // Bound the edges of the diagram
    for(auto Itr(GeneratedEdges.CreateConstIterator()); Itr; ++Itr)
    {
        TSharedPtr<FVoronoiDiagramEdge> Edge = (*Itr);
        Edge->GenerateClippedEndPoints(Bounds);
    }
    
    for(auto Itr(Sites.CreateConstIterator()); Itr; ++Itr)
    {
        TSharedPtr<FVoronoiDiagramSite> Site = (*Itr);
        Site->GenerateCentroid(Bounds);
    }
    
    for(auto SiteItr(Sites.CreateConstIterator()); SiteItr; ++SiteItr)
    {
        TSharedPtr<FVoronoiDiagramSite> Site = (*SiteItr);
        FVoronoiDiagramGeneratedSite GeneratedSite(Site->Index, Site->GetCoordinate(), Site->Centroid);
        GeneratedSite.Vertices.Append(Site->Vertices);
        
        for(auto EdgeItr(Site->Edges.CreateConstIterator()); EdgeItr; ++EdgeItr)
        {
            TSharedPtr<FVoronoiDiagramEdge> Edge = (*EdgeItr);
            GeneratedSite.Edges.Add(FVoronoiDiagramGeneratedEdge(Edge->Index, Edge->LeftClippedEndPoint, Edge->RightClippedEndPoint));
        }
        OutSites.Add(GeneratedSite);
        
        // Finished with the edges, remove the references so they can be removed at the end of the method
        Site->Edges.Empty();
    }
    
    
    // Clean up
    BottomMostSite.Reset();
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagram::GetNextSite()
{
    TSharedPtr<FVoronoiDiagramSite> NextSite;

    if (CurrentSiteIndex < Sites.Num())
    {
        NextSite = Sites[CurrentSiteIndex];
        CurrentSiteIndex++;
        return NextSite;
        }
    else
    {
        return nullptr;
    }
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagram::GetLeftRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    if(!HalfEdge->Edge.IsValid())
    {
        return BottomMostSite;
    }
    
    if(HalfEdge->EdgeType == EVoronoiDiagramEdge::Left)
    {
        return HalfEdge->Edge->LeftSite;
    }
    else
    {
        return HalfEdge->Edge->RightSite;
    }
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagram::GetRightRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    if(!HalfEdge->Edge.IsValid())
    {
        return BottomMostSite;
    }
    
    if(HalfEdge->EdgeType == EVoronoiDiagramEdge::Left)
    {
        return HalfEdge->Edge->RightSite;
    }
    else
    {
        return HalfEdge->Edge->LeftSite;
    }
}

void FVoronoiDiagramHelper::GenerateTexture(FVoronoiDiagram VoronoiDiagram, int32 RelaxationCycles, UTexture2D*& GeneratedTexture)
{
    GeneratedTexture = UTexture2D::CreateTransient(VoronoiDiagram.Bounds.Width(), VoronoiDiagram.Bounds.Height());

    FColor* MipData = static_cast<FColor*>(GeneratedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

    for(int32 x = 0; x < VoronoiDiagram.Bounds.Width(); ++x)
    {
        for(int32 y = 0; y < VoronoiDiagram.Bounds.Height(); ++y)
        {
            MipData[x + y * VoronoiDiagram.Bounds.Width()] = FColor::White;
        }
    }
    
    TArray<FVoronoiDiagramGeneratedSite> GeneratedSites;
    VoronoiDiagram.GenerateSites(GeneratedSites);
    // Lloyd's Algorithm
    for(int32 Cycles = 0; Cycles < RelaxationCycles; ++Cycles)
    {
        FVoronoiDiagram CurrentDiagram(VoronoiDiagram.Bounds);

        TArray<FIntPoint> CurrentPoints;
        for(auto Itr(GeneratedSites.CreateConstIterator()); Itr; ++Itr)
        {
            FVoronoiDiagramGeneratedSite CurrentSite = *Itr;
            CurrentPoints.Add(FIntPoint(FMath::RoundToInt(CurrentSite.Centroid.X), FMath::RoundToInt(CurrentSite.Centroid.Y)));
        }
        
        if(!CurrentDiagram.AddPoints(CurrentPoints))
        {
            UE_LOG(LogVoronoiDiagram, Error, TEXT("Issue with generating previous cycle's centroids. Aborting"));
            break;
        }
        
        GeneratedSites.Empty();
        CurrentDiagram.GenerateSites(GeneratedSites);
    }

    for(auto SiteItr(GeneratedSites.CreateConstIterator()); SiteItr; ++SiteItr)
    {
        FVoronoiDiagramGeneratedSite CurrentSite = *SiteItr;
        
        if(CurrentSite.Vertices.Num() == 0)
        {
            continue;
        }
        
        FVector2D MinimumVertex = CurrentSite.Vertices[0];
        FVector2D MaximumVertex = CurrentSite.Vertices[0];
        
        for(int32 i = 1; i < CurrentSite.Vertices.Num(); ++i)
        {
            if( CurrentSite.Vertices[i].X < MinimumVertex.X )
            {
                MinimumVertex.X = CurrentSite.Vertices[i].X;
            }

            if( CurrentSite.Vertices[i].Y < MinimumVertex.Y )
            {
                MinimumVertex.Y = CurrentSite.Vertices[i].Y;
            }
            
            if( CurrentSite.Vertices[i].X > MaximumVertex.X )
            {
                MaximumVertex.X = CurrentSite.Vertices[i].X;
            }

            if( CurrentSite.Vertices[i].Y > MaximumVertex.Y )
            {
                MaximumVertex.Y = CurrentSite.Vertices[i].Y;
            }
        }
        
        if(MinimumVertex.X < 0.0f)
        {
           MinimumVertex.X = 0.0f;
        }
        
        if(MinimumVertex.Y < 0.0f)
        {
           MinimumVertex.Y = 0.0f;
        }
        
        if(MaximumVertex.X > VoronoiDiagram.Bounds.Width())
        {
           MaximumVertex.X = VoronoiDiagram.Bounds.Width();
        }
        
        if(MaximumVertex.Y > VoronoiDiagram.Bounds.Height())
        {
           MaximumVertex.Y = VoronoiDiagram.Bounds.Height();
        }
        
        for(int32 x = MinimumVertex.X; x <= MaximumVertex.X; ++x)
        {
            for(int32 y = MinimumVertex.Y; y <= MaximumVertex.Y; ++y)
            {
                if(FVoronoiDiagramHelper::PointInVertices(FIntPoint(x, y), CurrentSite.Vertices))
                {
                    FVoronoiDiagramHelper::DrawOnMipData(MipData, CurrentSite.Color, x, y, VoronoiDiagram.Bounds);
                }
            }
        }
    }
    
    // Unlock the texture
    GeneratedTexture->PlatformData->Mips[0].BulkData.Unlock();
    GeneratedTexture->UpdateResource();
}

void FVoronoiDiagramHelper::DrawOnMipData(FColor* MipData, FColor Color, int32 X, int32 Y, FIntRect Bounds)
{
    if(Bounds.Contains(FIntPoint(X, Y)))
    {
        int32 Index = X + Y * Bounds.Width();
        MipData[Index] = Color;
    }
}

bool FVoronoiDiagramHelper::PointInVertices(FIntPoint Point, TArray<FVector2D> Vertices)
{
    int32 i;
    int32 j = Vertices.Num() - 1;

    bool bOddNodes = false;

    for( i = 0 ; i < Vertices.Num(); ++i )
    {
        if(
            (Vertices[i].Y < Point.Y && Vertices[j].Y >= Point.Y || Vertices[j].Y < Point.Y && Vertices[i].Y >= Point.Y) &&
            (Vertices[i].X <= Point.X || Vertices[j].X <= Point.X)
        )
        {
            if( Vertices[i].X + (Point.Y - Vertices[i].Y) / (Vertices[j].Y - Vertices[i].Y) * (Vertices[j].X - Vertices[i].X) < Point.X)
            {
                bOddNodes = !bOddNodes;
            }
        }
        j = i;
    }

    return bOddNodes;
}























































