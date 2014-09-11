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
    TSharedPtr<FVoronoiDiagramPoint> BottomMostSite;
    TSharedPtr<FVector2D> MinimumValues, DeltaValues;
    
    FVoronoiDiagramEdgeList(int32 NumberOfSites, TSharedPtr<FVector2D> InMinimumValues, TSharedPtr<FVector2D> InDeltaValues)
    : MinimumValues(InMinimumValues)
    , DeltaValues(InDeltaValues)
    , BottomMostSite(nullptr)
    {
        check(NumberOfSites >= 1);
        
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
        
        HalfEdge->Edge->MarkForDeletion();
        
        HalfEdge->EdgeListLeft = nullptr;
        HalfEdge->EdgeListRight = nullptr;
    }
    
    TSharedPtr<FVoronoiDiagramPoint> GetLeftRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
    {
        if(HalfEdge->Edge.IsValid() == false)
        {
            return BottomMostSite;
        }
        
        if(HalfEdge->EdgeType == EVoronoiDiagramEdge::Left)
        {
            return HalfEdge->Edge->LeftRegion;
        }
        else
        {
            return HalfEdge->Edge->RightRegion;
        }
    }
    
    TSharedPtr<FVoronoiDiagramPoint> GetRightRegion(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
    {
        if(HalfEdge->Edge.IsValid() == false)
        {
            return BottomMostSite;
        }
        
        if(HalfEdge->EdgeType == EVoronoiDiagramEdge::Right)
        {
            return HalfEdge->Edge->LeftRegion;
        }
        else
        {
            return HalfEdge->Edge->RightRegion;
        }
    }
    
    TSharedPtr<FVoronoiDiagramHalfEdge> GetFromHash(int32 Bucket)
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
        
        if(Bucket < 0 || Bucket >= Hash.Num())
        {
            return nullptr;
        }
        
        HalfEdge = Hash[Bucket];
        if(HalfEdge.IsValid() && HalfEdge->Edge.IsValid() && HalfEdge->Edge->IsDeleted() == true)
        {
            // Edge ready for deletion, return null instead
            Hash[Bucket] = nullptr;
            
            // Cannot delete half edge yet, so just return nullptr at this point
            return nullptr;
        }

        return HalfEdge;
    }
    
    TSharedPtr<FVoronoiDiagramHalfEdge> GetLeftBoundFrom(TSharedPtr<FVoronoiDiagramPoint> Point)
    {
        int32 Bucket;
        TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge;
        
        Bucket = (Point->Coordinate.X - MinimumValues->X) / DeltaValues->X * Hash.Num();
        
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
        if(HalfEdge == LeftEnd || (HalfEdge != RightEnd && HalfEdge->IsLeftOf(Point)))
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
    int32 MinimumIndex, NumberUsed;
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> Hash;
    TSharedPtr<FVector2D> MinimumValues, DeltaValues;
    
    FVoronoiDiagramPriorityQueue(int32 NumberOfSites, TSharedPtr<FVector2D> InMinimumValues, TSharedPtr<FVector2D> InDeltaValues)
    : MinimumIndex(0)
    , NumberUsed(0)
    , MinimumValues(InMinimumValues)
    , DeltaValues(InDeltaValues)
    {
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
    
    void Insert(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge, TSharedPtr<FVoronoiDiagramPoint> Vertex, float Offset)
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> Prev, Next;
        int32 InsertionBucket = GetBucket(HalfEdge);
        
        if(InsertionBucket < MinimumIndex)
        {
            MinimumIndex = InsertionBucket;
        }
       
        HalfEdge->YStar = Vertex->Coordinate.Y + Offset;
        HalfEdge->Vertex = Vertex;
        
        Prev = Hash[InsertionBucket];
        Next = Prev->NextInPriorityQueue;
        while(
            Next.IsValid() &&
            (
                HalfEdge->YStar > Next->YStar ||
                (HalfEdge->YStar == Next->YStar && HalfEdge->Vertex->Coordinate.X > Next->Vertex->Coordinate.X)
            )
        )
        {
            Prev = Next;
            Next = Prev->NextInPriorityQueue;
        }
        
        HalfEdge->NextInPriorityQueue = Prev->NextInPriorityQueue;
        Prev->NextInPriorityQueue = HalfEdge;
        NumberUsed++;
    }
    
    void Delete(TSharedPtr<FVoronoiDiagramHalfEdge> HalfEdge)
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> Prev;
        int32 RemovalBucket = GetBucket(HalfEdge);
        
        if(HalfEdge->Vertex.IsValid())
        {
            Prev = Hash[RemovalBucket];
            while(Prev->NextInPriorityQueue != HalfEdge)
            {
                Prev = Prev->NextInPriorityQueue;
            }
            Prev->NextInPriorityQueue = HalfEdge->NextInPriorityQueue;
            NumberUsed--;
            
            HalfEdge->Vertex = nullptr;
            HalfEdge->NextInPriorityQueue = nullptr;
            
            if(!HalfEdge->EdgeListLeft.IsValid() && !HalfEdge->EdgeListRight.IsValid())
            {
                HalfEdge->Edge = nullptr;
                HalfEdge.Reset();
            }
        }
    }
    
    bool IsEmpty()
    {
        return NumberUsed == 0;
    }
    
    FVoronoiDiagramPoint GetMinimumPoint()
    {
        while(MinimumIndex < Hash.Num() - 1 && !Hash[MinimumIndex]->NextInPriorityQueue.IsValid())
        {
            MinimumIndex++;
        }

        FVoronoiDiagramPoint MinPoint;
        
        MinPoint.Coordinate = FVector2D(Hash[MinimumIndex]->NextInPriorityQueue->Vertex->Coordinate.X, Hash[MinimumIndex]->NextInPriorityQueue->YStar);

        return MinPoint;
    }
    
    TSharedPtr<FVoronoiDiagramHalfEdge> RemoveAndReturnMinimumEdge()
    {
        TSharedPtr<FVoronoiDiagramHalfEdge> MinEdge;
        
        MinEdge = Hash[MinimumIndex]->NextInPriorityQueue;
        Hash[MinimumIndex]->NextInPriorityQueue = MinEdge->NextInPriorityQueue;
        NumberUsed--;
        
        MinEdge->NextInPriorityQueue = nullptr;
        
        return MinEdge;
    }
};

// End of FVoronoiDiagramHalfEdgePriorityQueue
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
        Sites.Add(FVoronoiDiagramPoint::CreatePtr(Sites.Num(), EVoronoiDiagramPoint::Site, FVector2D(static_cast<float>(CurrentPoint.X), static_cast<float>(CurrentPoint.Y))));
    }
    
    struct FSortSite
    {
        bool operator()(const TSharedPtr<FVoronoiDiagramPoint>& A, const TSharedPtr<FVoronoiDiagramPoint>& B) const
        {
            if(FMath::RoundToInt(A->Coordinate.Y) < FMath::RoundToInt(B->Coordinate.Y))
            {
                return true;
            }
            
            if(FMath::RoundToInt(A->Coordinate.Y) > FMath::RoundToInt(B->Coordinate.Y))
            {
                return false;
            }
            
            if(FMath::RoundToInt(A->Coordinate.X) < FMath::RoundToInt(B->Coordinate.X))
            {
                return true;
            }
            
            if(FMath::RoundToInt(A->Coordinate.X) > FMath::RoundToInt(B->Coordinate.X))
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
//        UE_LOG(LogVoronoiDiagram, Log, TEXT("Site: %i @ %i, %i"), (*Itr)->GetIndex(), (*Itr)->GetCoordinate().X, (*Itr)->GetCoordinate().Y);
        FVector2D CurrentPoint = (*Itr)->Coordinate;
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
    
    for(auto Itr(Sites.CreateConstIterator()); Itr; ++Itr)
    {
        FVector2D CurrentPoint = (*Itr)->Coordinate;
        UE_LOG(LogVoronoiDiagram, Log, TEXT("Point (%f, %f)"), CurrentPoint.X, CurrentPoint.Y);
    }
    
    UE_LOG(LogVoronoiDiagram, Log, TEXT("Min Values: %f, %f"), MinValues->X, MinValues->Y);
    UE_LOG(LogVoronoiDiagram, Log, TEXT("Max Values: %f, %f"), MaxValues->X, MaxValues->Y);
    UE_LOG(LogVoronoiDiagram, Log, TEXT("Delta Values: %f, %f"), DeltaValues->X, DeltaValues->Y);
    return true;
}

void FVoronoiDiagram::GenerateEdges()
{
    int32 NumGeneratedEdges = 0;
    int32 NumGeneratedVertices = 0;
    CurrentSiteIndex = 0;

    // Fortune's Algorithm
    FVoronoiDiagramPriorityQueue PriorityQueue(Sites.Num(), MinValues, DeltaValues);
    FVoronoiDiagramEdgeList EdgeList(Sites.Num(), MinValues, DeltaValues);


    FVoronoiDiagramPoint CurrentIntersectionStar;
    TSharedPtr<FVoronoiDiagramPoint> Bottom, Top, TempPoint, v, Vertex;
    TSharedPtr<FVoronoiDiagramHalfEdge> LeftBound, RightBound, LeftLeftBound, RightRightBound, Bisector;
    TSharedPtr<FVoronoiDiagramEdge> Edge;
    EVoronoiDiagramEdge::Type EdgeType;

    bool bDone = false;
    EdgeList.BottomMostSite = GetNextSite();
    TSharedPtr<FVoronoiDiagramPoint> CurrentSite = GetNextSite();
    while(!bDone)
    {
        if(!PriorityQueue.IsEmpty())
        {
            CurrentIntersectionStar = PriorityQueue.GetMinimumPoint();
        }
        
        if(
            CurrentSite.IsValid() &&
            (
                PriorityQueue.IsEmpty() ||
                CurrentSite->Coordinate.Y < CurrentIntersectionStar.Coordinate.Y ||
                (
                    CurrentSite->Coordinate.Y == CurrentIntersectionStar.Coordinate.Y
                    && CurrentSite->Coordinate.X < CurrentIntersectionStar.Coordinate.X
                )
            )
        )
        {
            // Current processed site is the smallest
            LeftBound = EdgeList.GetLeftBoundFrom(CurrentSite);
            RightBound = LeftBound->EdgeListRight;
            Bottom = EdgeList.GetRightRegion(LeftBound);

            Edge = FVoronoiDiagramEdge::Bisect(Bottom, CurrentSite);
            Edge->EdgeIndex = NumGeneratedEdges;
            NumGeneratedEdges++;
            
            Bisector = FVoronoiDiagramHalfEdge::CreatePtr(Edge, EVoronoiDiagramEdge::Left);
            EdgeList.Insert(LeftBound, Bisector);
            
            Vertex = FVoronoiDiagramPoint::Intersect(LeftBound, Bisector);
            if(Vertex.IsValid())
            {
                PriorityQueue.Delete(LeftBound);
                PriorityQueue.Insert(LeftBound, Vertex, CurrentSite->GetDistanceFrom(Vertex));
            }
            
            LeftBound = Bisector;
            Bisector = FVoronoiDiagramHalfEdge::CreatePtr(Edge, EVoronoiDiagramEdge::Right);
            
            EdgeList.Insert(LeftBound, Bisector);
            
            Vertex = FVoronoiDiagramPoint::Intersect(Bisector, RightBound);
            if(Vertex.IsValid())
            {
                PriorityQueue.Insert(Bisector, Vertex, CurrentSite->GetDistanceFrom(Vertex));
            }
            
            CurrentSite = GetNextSite();
        }
        else if(PriorityQueue.IsEmpty() == false)
        {
            // Current intersection is the smallest
            LeftBound = PriorityQueue.RemoveAndReturnMinimumEdge();
            LeftLeftBound = LeftBound->EdgeListLeft;
            RightBound = LeftBound->EdgeListRight;
            RightRightBound = RightBound->EdgeListRight;
            Bottom = EdgeList.GetLeftRegion(LeftBound);
            Top = EdgeList.GetRightRegion(RightBound);
            
            v = LeftBound->Vertex;
            v->Index = NumGeneratedVertices;
            NumGeneratedVertices++;
            
            LeftBound->Edge->SetEndpoint(v, LeftBound->EdgeType);
            RightBound->Edge->SetEndpoint(v, RightBound->EdgeType);
            
            EdgeList.Delete(LeftBound);
            PriorityQueue.Delete(RightBound);
            EdgeList.Delete(RightBound);
            
            EdgeType = EVoronoiDiagramEdge::Left;
            if(Bottom->Coordinate.Y > Top->Coordinate.Y)
            {
                TempPoint = Bottom;
                Bottom = Top;
                Top = TempPoint;
                EdgeType = EVoronoiDiagramEdge::Right;
            }
            
            Edge = FVoronoiDiagramEdge::Bisect(Bottom, Top);
            Bisector = FVoronoiDiagramHalfEdge::CreatePtr(Edge, EdgeType);
            EdgeList.Insert(LeftLeftBound, Bisector);
            
            if(EdgeType == EVoronoiDiagramEdge::Left)
            {
                Edge->SetEndpoint(v, EVoronoiDiagramEdge::Right);
            }
            {
                Edge->SetEndpoint(v, EVoronoiDiagramEdge::Left);
            }
            
            Vertex = FVoronoiDiagramPoint::Intersect(LeftLeftBound, Bisector);
            if(Vertex.IsValid())
            {
                PriorityQueue.Delete(LeftLeftBound);
                PriorityQueue.Insert(LeftLeftBound, Vertex, Bottom->GetDistanceFrom(Vertex));
            }
            
            Vertex = FVoronoiDiagramPoint::Intersect(Bisector, RightRightBound);
            if(Vertex.IsValid())
            {
                PriorityQueue.Insert(Bisector, Vertex, Bottom->GetDistanceFrom(Vertex));
            }
        }
        else
        {
            bDone = true;
        }
    }
}

TSharedPtr<FVoronoiDiagramPoint> FVoronoiDiagram::GetNextSite()
{
    TSharedPtr<FVoronoiDiagramPoint> NextSite;

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
