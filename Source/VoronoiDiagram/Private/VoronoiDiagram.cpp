// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramModule.h"
#include "VoronoiDiagram.h"

const int32 FVoronoiDiagram::LeftEdge = 0;
const int32 FVoronoiDiagram::RightEdge = 1;

////////////////////////////////////////////////////////////////////////////////
// FVoronoiDiagramHalfEdgePriorityQueue

class FVoronoiDiagramHalfEdgePriorityQueue
{
public:
    FVoronoiDiagramHalfEdgePriorityQueue(float InMinY, float InMaxY, float InDeltaY, int32 SqrtNumSites);
    
    void Insert(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge);
    void Remove(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge);
    
    bool IsEmpty();
    
    // Returns coordinate of the half edge's vertex in the transformed voronoi diagram
    FVector2D Min();
    
    // Remove and return the min half edge
    TSharedPtr<FVoronoiDiagramHalfEdge> ExtractMin();
    
private:
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> Hash;
    float MinY;
    float MaxY;
    float DeltaY;
    int32 NumberUsed;
    int32 MinBucket;
    
    int32 GetBucket(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge);
};

FVoronoiDiagramHalfEdgePriorityQueue::FVoronoiDiagramHalfEdgePriorityQueue(float InMinY, float InMaxY, float InDeltaY, int32 SqrtNumSites)
: MinY(InMinY)
, MaxY(InMaxY)
, DeltaY(InDeltaY)
, NumberUsed(0)
, MinBucket(0)
{
    Hash.Empty();
    for(int32 i = 0; i < 4 * SqrtNumSites; ++i)
    {
        Hash.Add(FVoronoiDiagramHalfEdge::CreateEmptyHalfEdgePtr());
    }
}

void FVoronoiDiagramHalfEdgePriorityQueue::Insert(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    TSharedPtr<FVoronoiDiagramHalfEdge> Previous, Next;
    int32 IntersectionBucket = GetBucket(HalfEdge);
    
    Previous = Hash[IntersectionBucket];
    Next = Previous->GetNextInQueue();
    while(
        Next.IsValid() &&
        (
            HalfEdge->GetYStar() > Next->GetYStar() ||
            (HalfEdge->GetYStar() == Next->GetYStar() && HalfEdge->GetVertex()->GetCoordinate().X > Next->GetVertex()->GetCoordinate().X)
        )
    )
    {
        Previous = Next;
        Next = Previous->GetNextInQueue();
    }
    
    HalfEdge->SetNextInQueue(Previous->GetNextInQueue());
    Previous->SetNextInQueue(HalfEdge);
    NumberUsed++;
}

void FVoronoiDiagramHalfEdgePriorityQueue::Remove(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    TSharedPtr<FVoronoiDiagramHalfEdge> Previous;
    int32 RemovalBucket = GetBucket(HalfEdge);
    
    if(HalfEdge->GetVertex().IsValid())
    {
        Previous = Hash[RemovalBucket];
        while(Previous->GetNextInQueue() != HalfEdge)
        {
            Previous = Previous->GetNextInQueue();
        }
        
        Previous->SetNextInQueue(HalfEdge->GetNextInQueue());
        NumberUsed--;
        
        HalfEdge->SetVertex(nullptr);
        HalfEdge->SetNextInQueue(nullptr);
        FVoronoiDiagramHalfEdge::AttemptToReset(HalfEdge);
    }
}

bool FVoronoiDiagramHalfEdgePriorityQueue::IsEmpty()
{
    return NumberUsed == 0;
}


FVector2D FVoronoiDiagramHalfEdgePriorityQueue::Min()
{
    while(MinBucket < Hash.Num() - 1 && !Hash[MinBucket]->GetNextInQueue().IsValid())
    {
        MinBucket++;
    }
    
    TSharedPtr<FVoronoiDiagramHalfEdge> NewMinHalfEdge = Hash[MinBucket]->GetNextInQueue();
    
    return FVector2D(NewMinHalfEdge->GetVertex()->GetCoordinate().X, NewMinHalfEdge->GetYStar());

}

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramHalfEdgePriorityQueue::ExtractMin()
{
    TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
    
    HalfEdge = Hash[MinBucket]->GetNextInQueue();
    
    Hash[MinBucket]->SetNextInQueue(HalfEdge->GetNextInQueue());
    NumberUsed--;
    
    HalfEdge->SetNextInQueue(nullptr);
    
    return HalfEdge;
}

int32 FVoronoiDiagramHalfEdgePriorityQueue::GetBucket(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    int32 Bucket;
    
    if(HalfEdge->GetYStar() < MinY)
    {
        Bucket = 0;
    }
    else if(HalfEdge->GetYStar() >= MaxY)
    {
        Bucket = Hash.Num() - 1;
    }
    else
    {
        Bucket = (HalfEdge->GetYStar() - MinY) / DeltaY * Hash.Num();
    }
    
    if(Bucket < 0)
    {
        Bucket = 0;
    }
    
    if(Bucket >= Hash.Num())
    {
        Bucket = Hash.Num() - 1;
    }
    
    if(Bucket < MinBucket)
    {
        MinBucket = Bucket;
    }
    
    return Bucket;
}

// End of FVoronoiDiagramHalfEdgePriorityQueue
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FVoronoiDiagramEdgeList

class FVoronoiDiagramEdgeList
{
public:
    FVoronoiDiagramEdgeList(float InMinX, float InDeltaX, int32 SqrtNumSites);

    void Insert(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge, TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge);
    void Remove(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge);
    
    TSharedPtr<FVoronoiDiagramHalfEdge> GetLeftNeighborOf(FVector2D Point);

private:
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> Hash;
    TSharedPtr<FVoronoiDiagramHalfEdge> LeftEnd;
    TSharedPtr<FVoronoiDiagramHalfEdge> RightEnd;
    float MinX;
    float DeltaX;
    
    TSharedPtr<FVoronoiDiagramHalfEdge> GetEdgeFromHash(int32 Index);
};

FVoronoiDiagramEdgeList::FVoronoiDiagramEdgeList(float InMinX, float InDeltaX, int32 SqrtNumSites)
: MinX(InMinX)
, DeltaX(InDeltaX)
{
    Hash.Empty();
    for(int32 i = 0; i < 2 * SqrtNumSites; ++i)
    {
        Hash.Add(nullptr);
    }
    
    LeftEnd = FVoronoiDiagramHalfEdge::CreateEmptyHalfEdgePtr();
    RightEnd = FVoronoiDiagramHalfEdge::CreateEmptyHalfEdgePtr();
    
    LeftEnd->SetLeftNeighbor(nullptr);
    LeftEnd->SetRightNeighbor(RightEnd);
    
    RightEnd->SetLeftNeighbor(LeftEnd);
    RightEnd->SetRightNeighbor(nullptr);
    
    Hash[0] = LeftEnd;
    Hash[Hash.Num() - 1] = RightEnd;
}

void FVoronoiDiagramEdgeList::Insert(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge, TSharedPtr<FVoronoiDiagramHalfEdge> NewHalfEdge)
{
    NewHalfEdge->SetLeftNeighbor(HalfEdge);
    NewHalfEdge->SetRightNeighbor(HalfEdge->GetRightNeighbor());
    HalfEdge->GetRightNeighbor()->SetLeftNeighbor(NewHalfEdge);
    HalfEdge->SetRightNeighbor(NewHalfEdge);
}

void FVoronoiDiagramEdgeList::Remove(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    HalfEdge->GetLeftNeighbor()->SetRightNeighbor(HalfEdge->GetRightNeighbor());
    HalfEdge->GetRightNeighbor()->SetLeftNeighbor(HalfEdge->GetLeftNeighbor());
    HalfEdge->GetEdge()->MarkReadyForDeletion();
    HalfEdge->SetLeftNeighbor(nullptr);
    HalfEdge->SetRightNeighbor(nullptr);
}

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramEdgeList::GetLeftNeighborOf(FVector2D Point)
{
    int32 Index, Bucket;
    TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
    
    // Use hash table to get close to desired half edge
    Bucket = (Point.X - MinX) / DeltaX * Hash.Num();
    
    if(Bucket < 0)
    {
        Bucket = 0;
    }
    
    if(Bucket >= Hash.Num())
    {
        Bucket = Hash.Num() - 1;
    }
    
    HalfEdge = GetEdgeFromHash(Bucket);
    if(!HalfEdge.IsValid())
    {
        int32 Index = 1;
        while(true)
        {
            HalfEdge = GetEdgeFromHash(Bucket - Index);
            if(HalfEdge.IsValid())
            {
                break;
            }
            
            HalfEdge = GetEdgeFromHash(Bucket + Index);
            if(HalfEdge.IsValid())
            {
                break;
            }
            Index++;
            
            // Infinite loop check
            check(Bucket - Index >= 0 && Bucket + Index < Hash.Num());
        }
    }
    
    // Search linearly for the correct one
    if(
        HalfEdge == LeftEnd ||
        (HalfEdge != RightEnd && HalfEdge->IsLeftOf(Point)))
    {
        do
        {
            HalfEdge = HalfEdge->GetRightNeighbor();
        }while(HalfEdge != RightEnd && HalfEdge->IsLeftOf(Point));
        
        HalfEdge = HalfEdge->GetLeftNeighbor();
    }
    else
    {
        do
        {
            HalfEdge = HalfEdge->GetLeftNeighbor();
        }while(HalfEdge != LeftEnd && !HalfEdge->IsLeftOf(Point));
    }
    
    // Update has table
    if(Bucket > 0 && Bucket < Hash.Num() - 1)
    {
        Hash[Bucket] = HalfEdge;
    }
    
    return HalfEdge;
}

TSharedPtr<FVoronoiDiagramHalfEdge> FVoronoiDiagramEdgeList::GetEdgeFromHash(int32 Index)
{
    TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
    
    if(Index < 0 || Index >= Hash.Num())
    {
        return nullptr;
    }
    
    HalfEdge = Hash[Index];
    
    if(HalfEdge.IsValid() && HalfEdge->GetEdge().IsValid() && HalfEdge->GetEdge()->IsReadyForDeletion())
    {
        Hash[Index].Reset();
        Hash[Index] = nullptr;
        
        return nullptr;
    }
    
    return HalfEdge;
}

// End of FVoronoiDiagramEdgeList
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FVoronoiDiagramPoint

namespace EVoronoiDiagramPointType
{
    enum Type
    {
        Site,
        Intersection,
        Vertex
    };
}

class FVoronoiDiagramPoint
{
public:
    static TSharedPtr<FVoronoiDiagramPoint> CreatePointPtr(int32 Index, FVector2D Coordinate, EVoronoiDiagramPointType Type);

    int32 GetIndex() const;
    FVector2D GetCoordinate() const;
    
    FString ToString();
private:
    int32 Index;
    FVector2D Coordinate;
    EVoronoiDiagramPointType Type;
    
    FVoronoiDiagramSite(int32 InIndex, FVector2D InCoordinate, EVoronoiDiagramPointType Type);
};

// End of FVoronoiDiagramPoint
////////////////////////////////////////////////////////////////////////////////


FVoronoiDiagram::FVoronoiDiagram(FIntRect InBounds)
: Bounds(InBounds)
{}

bool FVoronoiDiagram::AddPoints(TArray<FIntPoint>& Points)
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
        Sites.Add(FVoronoiDiagramSite::CreateSitePtr(Sites.Num(), CurrentPoint));
    }
    
    struct FSortSite
    {
        bool operator()(const TSharedPtr<FVoronoiDiagramSite>& A, const TSharedPtr<FVoronoiDiagramSite>& B) const
        {
            if(A->GetCoordinate().Y < B->GetCoordinate().Y)
            {
                return true;
            }
            
            if(A->GetCoordinate().Y > B->GetCoordinate().Y)
            {
                return false;
            }
            
            if(A->GetCoordinate().X < B->GetCoordinate().X)
            {
                return true;
            }
            
            if(A->GetCoordinate().X > B->GetCoordinate().X)
            {
                return false;
            }
            
            return false;
        }
    };
    
    Sites.Sort(FSortSite());
    
    FVector2D CurrentMin(INT_MAX, INT_MAX);
    FVector2D CurrentMax(INT_MIN, INT_MIN);
    for(auto Itr(Sites.CreateConstIterator()); Itr; ++Itr)
    {
//        UE_LOG(LogVoronoiDiagram, Log, TEXT("Site: %i @ %i, %i"), (*Itr)->GetIndex(), (*Itr)->GetCoordinate().X, (*Itr)->GetCoordinate().Y);
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
    
    MinValues = CurrentMin;
    MaxValues = CurrentMax;
    DeltaValues = FVector2D(CurrentMax.X - CurrentMin.X, CurrentMax.Y - CurrentMin.Y);
    
    return true;
}

bool FVoronoiDiagram::GetEdges(TArray<TArray<FIntPoint>>& OutEdges)
{
    // Initialize priority queue with all sites
    TArray<TSharedPtr<FVoronoiDiagramPoint>> PriorityQueue;
    int32 Index = 0;
    for(auto Itr(Sites.CreateConstIterator(); Itr; ++Itr)
    {
        PriorityQueue.Add(FVoronoiDiagramPoint::CreatePointPtr(Index, (*Itr)->GetCoordinate(), EVoronoiDiagramPointType::Site));
        Index++;
    }
    
    
    
    

//    TSharedPtr<FVoronoiDiagramSite> NewSite, BottomSite, TopSite, TempSite;
//    TSharedPtr<FVoronoiDiagramHalfEdge> LeftBound, RightBound, LeftBoundLeftBound, RightBoundRightBound, Bisector;
//    TSharedPtr<FVoronoiDiagramEdge> Edge;
//    TSharedPtr<FVoronoiDiagramVertex> Vertex, TempVertex;
//    FVector2D NewIntStar;
//    int32 LeftRight;
//    
//    int32 SqrtNumSites = FMath::RoundToInt(FMath::Sqrt(Sites.Num()));
//    
//    // Initialize priority queue
//    FVoronoiDiagramHalfEdgePriorityQueue PriorityQueue(MinValues.Y, MaxValues.X, DeltaValues.Y, SqrtNumSites);
//    
//    // List of edges during generation
//    FVoronoiDiagramEdgeList EdgeList(MinValues.X, DeltaValues.X, SqrtNumSites);
//    
//    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> HalfEdges;
//    TArray<TSharedPtr<FVoronoiDiagramVertex>> Vertices;
//    
//    CurrentSiteIndex = 0;
//    BottomMostSite = GetNextSiteAndIncrementCount();
//    NewSite = GetNextSiteAndIncrementCount();
//    
//    bool bDone = false;
//    while(!bDone)
//    {
//        if(PriorityQueue.IsEmpty() == false)
//        {
//            NewIntStar = PriorityQueue.Min();
//        }
//
//        if(
//            NewSite.IsValid() &&
//            (
//                PriorityQueue.IsEmpty() == true ||
//                NewSite->GetCoordinate().Y < NewIntStar.Y ||
//                (
//                    NewSite->GetCoordinate().Y == NewIntStar.Y &&
//                    NewSite->GetCoordinate().X < NewIntStar.X
//                )
//            )
//            
//        )
//        {
////            UE_LOG(LogVoronoiDiagram, Log, TEXT("Smallest site: %s"), *(NewSite->ToString()));
//        
//            LeftBound = EdgeList.GetLeftNeighborOf(NewSite->GetCoordinate());
////            UE_LOG(LogVoronoiDiagram, Log, TEXT("Left bound: %s"), *(LeftBound->ToString()));
//
//            RightBound = LeftBound->GetRightNeighbor();
////            UE_LOG(LogVoronoiDiagram, Log, TEXT("Right bound: %s"), *(RightBound->ToString()));
//            
//            BottomSite = GetRightRegion(LeftBound);
////            UE_LOG(LogVoronoiDiagram, Log, TEXT("Bottom site: %s"), *(BottomSite->ToString()));
//            
//            Edge = FVoronoiDiagramEdge::CreateBisectingEdge(BottomSite, NewSite);
////            UE_LOG(LogVoronoiDiagram, Log, TEXT("New bisecting edge: %s"), *(Edge->ToString()));
//
//            GeneratedEdges.Add(Edge);
//            
//            Bisector = FVoronoiDiagramHalfEdge::CreateHalfEdgePtr(Edge, FVoronoiDiagram::LeftEdge);
//            HalfEdges.Add(Bisector);
//            
//            EdgeList.Insert(LeftBound, Bisector);
//            
//            Vertex = FVoronoiDiagramVertex::Intersect(LeftBound, Bisector);
//            if(Vertex.IsValid())
//            {
//                Vertices.Add(Vertex);
//                PriorityQueue.Remove(LeftBound);
//                LeftBound->SetVertex(Vertex);
//                LeftBound->SetYStar(Vertex->GetCoordinate().Y + NewSite->DistanceFrom(Vertex->GetCoordinate()));
//                PriorityQueue.Insert(LeftBound);
//            }
//            
//            LeftBound = Bisector;
//            Bisector = FVoronoiDiagramHalfEdge::CreateHalfEdgePtr(Edge, FVoronoiDiagram::RightEdge);
//            HalfEdges.Add(Bisector);
//            
//            EdgeList.Insert(LeftBound, Bisector);
//            
//            Vertex = FVoronoiDiagramVertex::Intersect(Bisector, RightBound);
//            if(Vertex.IsValid())
//            {
//                Vertices.Add(Vertex);
//                Bisector->SetVertex(Vertex);
//                Bisector->SetYStar(Vertex->GetCoordinate().Y + NewSite->DistanceFrom(Vertex->GetCoordinate()));
//                PriorityQueue.Insert(Bisector);
//            }
//            
//            NewSite = GetNextSiteAndIncrementCount();
//        }
//        else if(PriorityQueue.IsEmpty() == false)
//        {
//            LeftBound = PriorityQueue.ExtractMin();
//            LeftBoundLeftBound = LeftBound->GetLeftNeighbor();
//            RightBound = LeftBound->GetRightNeighbor();
//            RightBoundRightBound = RightBound->GetRightNeighbor();
//            BottomSite = GetLeftRegion(LeftBound);
//            TopSite = GetRightRegion(RightBound);
//            
//            TempVertex = LeftBound->GetVertex();
//            
//            if(LeftBound->GetLeftRight() == FVoronoiDiagram::LeftEdge)
//            {
//                LeftBound->GetEdge()->SetLeftVertex(TempVertex);
//            }
//            else // Right Edge
//            {
//                LeftBound->GetEdge()->SetRightVertex(TempVertex);
//            }
//            
//            if(RightBound->GetLeftRight() == FVoronoiDiagram::LeftEdge)
//            {
//                RightBound->GetEdge()->SetLeftVertex(TempVertex);
//            }
//            else // Right Edge
//            {
//                RightBound->GetEdge()->SetRightVertex(TempVertex);
//            }
//            
//            EdgeList.Remove(LeftBound);
//            PriorityQueue.Remove(RightBound);
//            EdgeList.Remove(RightBound);
//
//            LeftRight = FVoronoiDiagram::LeftEdge;
//            if(BottomSite->GetCoordinate().Y > TopSite->GetCoordinate().Y)
//            {
//                TempSite = BottomSite;
//                BottomSite = TopSite;
//                TopSite = TempSite;
//                LeftRight = FVoronoiDiagram::RightEdge;
//            }
//            
//            Edge = FVoronoiDiagramEdge::CreateBisectingEdge(BottomSite, TopSite);
//            GeneratedEdges.Add(Edge);
//            
//            Bisector = FVoronoiDiagramHalfEdge::CreateHalfEdgePtr(Edge, LeftRight);
//            HalfEdges.Add(Bisector);
//            EdgeList.Insert(LeftBoundLeftBound, Bisector);
//
//            if(LeftRight == FVoronoiDiagram::LeftEdge)
//            {
//                Edge->SetRightVertex(TempVertex);
//            }
//            else // Right Edge
//            {
//                Edge->SetLeftVertex(TempVertex);
//            }
//            
//            Vertex = FVoronoiDiagramVertex::Intersect(LeftBoundLeftBound, Bisector);
//            if(Vertex.IsValid())
//            {
//                Vertices.Add(Vertex);
//                PriorityQueue.Remove(LeftBoundLeftBound);
//                LeftBoundLeftBound->SetVertex(Vertex);
//                LeftBoundLeftBound->SetYStar(Vertex->GetCoordinate().Y + BottomSite->DistanceFrom(Vertex->GetCoordinate()));
//                PriorityQueue.Insert(LeftBoundLeftBound);
//            }
//            
//            Vertex = FVoronoiDiagramVertex::Intersect(Bisector, RightBoundRightBound);
//            if(Vertex.IsValid())
//            {
//                Vertices.Add(Vertex);
//                Bisector->SetVertex(Vertex);
//                Bisector->SetYStar(Vertex->GetCoordinate().Y + BottomSite->DistanceFrom(Vertex->GetCoordinate()));
//                PriorityQueue.Insert(Bisector);
//            }
//        }
//        else
//        {
//            bDone = true;
//        }
//    }
//    
//    // Clip the edges and return
//    OutEdges.Empty();
//    for(auto Itr(GeneratedEdges.CreateIterator()); Itr; ++Itr)
//    {
//        TSharedPtr<FVoronoiDiagramEdge> CurrentEdge = *Itr;
//
//        TArray<FIntPoint> CurrentPoints;
//        if(CurrentEdge->GetLeftVertex().IsValid() && CurrentEdge->GetRightVertex().IsValid())
//        {
//            CurrentPoints.Add(FIntPoint(FMath::RoundToInt(CurrentEdge->GetLeftVertex()->GetCoordinate().X), FMath::RoundToInt(CurrentEdge->GetLeftVertex()->GetCoordinate().Y)));
//            CurrentPoints.Add(FIntPoint(FMath::RoundToInt(CurrentEdge->GetRightVertex()->GetCoordinate().X), FMath::RoundToInt(CurrentEdge->GetLeftVertex()->GetCoordinate().Y)));
//            OutEdges.Add(CurrentPoints);
//        }
//
//        
////        FString LeftSite = CurrentEdge->GetLeftSite().IsValid() ? CurrentEdge->GetLeftSite()->ToString() : "null";
////        FString RightSite = CurrentEdge->GetRightSite().IsValid() ? CurrentEdge->GetRightSite()->ToString() : "null";
////        FString LeftVertex = CurrentEdge->GetLeftVertex().IsValid() ? CurrentEdge->GetLeftVertex()->ToString() : "null";
////        FString RightVertex = CurrentEdge->GetRightVertex().IsValid() ? CurrentEdge->GetRightVertex()->ToString() : "null";
////        
////        UE_LOG(LogVoronoiDiagram, Log, TEXT("Generated Edges - Left Site: %s; Right Site: %s; Left Vertex: %s; Right Vertex: %s;"), *(LeftSite), *(RightSite), *(LeftVertex), *(RightVertex));
////        //CurrentEdge->ClipVertices(Bounds);
////        //OutEdges.Add(CurrentEdge->GetClippedVertices());
//    }
    
    return true;
}

FIntRect FVoronoiDiagram::GetBounds()
{
    return Bounds;
}

TArray<TSharedPtr<class FVoronoiDiagramSite>> FVoronoiDiagram::GetSites()
{
    return Sites;
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagram::GetNextSiteAndIncrementCount()
{
    if(CurrentSiteIndex < Sites.Num())
    {
        TSharedPtr<FVoronoiDiagramSite> ReturnSite = Sites[CurrentSiteIndex];
        CurrentSiteIndex++;
        return ReturnSite;
    }
    
    return nullptr;
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagram::GetLeftRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    TSharedPtr<FVoronoiDiagramEdge> Edge = HalfEdge->GetEdge();
    
    if(!Edge.IsValid())
    {
        return BottomMostSite;
    }
    
    if(HalfEdge->GetLeftRight() == FVoronoiDiagram::LeftEdge)
    {
        return Edge->GetLeftSite();
    }
    else
    {
        return Edge->GetRightSite();
    }
}

TSharedPtr<FVoronoiDiagramSite> FVoronoiDiagram::GetRightRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
{
    TSharedPtr<FVoronoiDiagramEdge> Edge = HalfEdge->GetEdge();
    
    if(!Edge.IsValid())
    {
        return BottomMostSite;
    }
    
    if(HalfEdge->GetLeftRight() == FVoronoiDiagram::LeftEdge)
    {
        return Edge->GetRightSite();
    }
    else
    {
        return Edge->GetLeftSite();
    }

}

void FVoronoiDiagramHelper::GenerateTexture(FVoronoiDiagram VoronoiDiagram, UTexture2D*& GeneratedTexture)
{
    TArray<TArray<FIntPoint>> GeneratedEdges;
    VoronoiDiagram.GetEdges(GeneratedEdges);
    
    GeneratedTexture = UTexture2D::CreateTransient(VoronoiDiagram.GetBounds().Width(), VoronoiDiagram.GetBounds().Height());
    
    FColor* MipData = static_cast<FColor*>(GeneratedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
    
    for(int32 RowNumber = 0; RowNumber < GeneratedTexture->GetSizeX(); ++RowNumber)
    {
        for(int32 ColumnNumber = 0; ColumnNumber < GeneratedTexture->GetSizeY(); ++ColumnNumber)
        {
            MipData[RowNumber + ColumnNumber * GeneratedTexture->GetSizeX()] = FColor::White;
        }
    }
    
    for(auto Itr(VoronoiDiagram.GetSites().CreateConstIterator()); Itr; ++Itr)
    {
        TSharedPtr<FVoronoiDiagramSite> CurrentSite = *Itr;
        MipData[FMath::RoundToInt(CurrentSite->GetCoordinate().X) + FMath::RoundToInt(CurrentSite->GetCoordinate().Y) * GeneratedTexture->GetSizeX()] = FColor::Blue;
    }
    
    for(auto EdgeItr(GeneratedEdges.CreateConstIterator()); EdgeItr; ++EdgeItr)
    {
        if((*EdgeItr).Num() != 2)
        {
            continue;
        }
    
        FIntPoint PointA = (*EdgeItr)[0];
        FIntPoint PointB = (*EdgeItr)[1];
    
        if(VoronoiDiagram.GetBounds().Contains(PointA))
        {
            MipData[PointA.X + PointA.Y * GeneratedTexture->GetSizeX()] = FColor::Red;
        }
        
        if(VoronoiDiagram.GetBounds().Contains(PointB))
        {
            MipData[PointB.X + PointB.Y * GeneratedTexture->GetSizeX()] = FColor::Red;
        }
        
        // Bresenham's line algorithm
        bool Steep = FMath::Abs(PointB.Y - PointA.Y) > FMath::Abs(PointB.X - PointA.X);
    
        if(Steep)
        {
            PointA = FIntPoint(PointA.Y, PointA.X);
            PointB = FIntPoint(PointB.Y, PointB.X);
        }

        if(PointA.X > PointB.X)
        {
            int32 Temp;
            Temp = PointA.X;
            PointA.X = PointB.X;
            PointB.X = Temp;

            Temp = PointA.Y;
            PointA.Y = PointB.Y;
            PointB.Y = Temp;
            
        }

        FIntPoint Delta(PointB.X - PointA.X, FMath::Abs(PointB.Y - PointA.Y));
        int32 Error = Delta.X / 2;

        int32 YStep;
        
        int y = PointA.Y;
        if( PointA.Y < PointB.Y)
        {
            YStep = 1;
        }
        else
        {
            YStep = -1;
        }
        
        if(PointA.X > PointB.X)
        {
            // Remove me after debugging complete
            continue;
        }
        
        for(int32 x = PointA.X; x <= PointB.X; ++x)
        {
            if(!VoronoiDiagram.GetBounds().Contains(FIntPoint(x, y)))
            {
                continue;
            }
        
            if(Steep)
            {
                MipData[y + x * GeneratedTexture->GetSizeX()] = FColor::Black;
            }
            else
            {
                MipData[x + y * GeneratedTexture->GetSizeX()] = FColor::Black;
            }
            
            Error = Error - Delta.Y;
            
            if(Error < 0)
            {
                y += YStep;
                Error = Error + Delta.X;
            }
        }
    }
    
    // Unlock the texture
    GeneratedTexture->PlatformData->Mips[0].BulkData.Unlock();
    GeneratedTexture->UpdateResource();
}
