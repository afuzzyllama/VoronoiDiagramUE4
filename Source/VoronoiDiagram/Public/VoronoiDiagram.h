// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

#include "../Private/VoronoiDiagramPrivatePCH.h"
#include "../Private/VoronoiDiagramSite.h"
#include "../Private/VoronoiDiagramVertex.h"
#include "../Private/VoronoiDiagramEdge.h"
#include "../Private/VoronoiDiagramHalfEdge.h"
#include "VoronoiDiagramGeneratedEdge.h"
#include "VoronoiDiagramGeneratedSite.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVoronoiDiagram, Log, All);

////////////////////////////////////////////////////////////////////////////////
// FVoronoiDiagramEdgeList

class FVoronoiDiagramEdgeList
{
public:
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> Hash;
    TSharedPtr<FVoronoiDiagramHalfEdge> LeftEnd;
    TSharedPtr<FVoronoiDiagramHalfEdge> RightEnd;
    TSharedPtr<FVector2D> MinimumValues;
    TSharedPtr<FVector2D> DeltaValues;
    
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
                    UE_LOG(LogVoronoiDiagram, Error, TEXT("(Bucket - Index) < 0 && (Bucket + Index) >= Hash.Num())"));
                    UE_LOG(LogVoronoiDiagram, Error, TEXT("(%i) < 0 && (%i) >= %i)"), Bucket - Index, Bucket + Index, Hash.Num());
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
    int32 MinimumBucket;
    int32 Count;
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge>> Hash;
    TSharedPtr<FVector2D> MinimumValues;
    TSharedPtr<FVector2D> DeltaValues;
    
    FVoronoiDiagramPriorityQueue(int32 NumberOfSites, TSharedPtr<FVector2D> InMinimumValues, TSharedPtr<FVector2D> InDeltaValues)
    : MinimumBucket(0)
    , Count(0)
    , MinimumValues(InMinimumValues)
    , DeltaValues(InDeltaValues)
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
                (FMath::IsNearlyEqual(HalfEdge->YStar, Next->YStar) && HalfEdge->Vertex->GetCoordinate().X > Next->Vertex->GetCoordinate().X)
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


/*
 *  Generates Voronoi Diagram from a set of provided points
 */
class VORONOIDIAGRAM_API FVoronoiDiagram
{
public:
    /*
     *  Constructor
     *
     *  @param  InBounds    Rectangle that represents a 2D region for placing points
     */
    FVoronoiDiagram(FIntRect InBounds);
    
    /*
     *  Adds points to the diagram to be used at generation time
     *
     *  @param  Points  Array of integer points to be added.
     *  @return         True if added successful, false otherwise.  If false, no points are added.
     */
    bool AddPoints(const TArray<FIntPoint>& Points);
    
    /*
     *  Runs Fortune's Algorithm to generate sites with edges for the diagram
     *
     *  @param  OutSites    Array to fill with sites
     */
    template<class T>
    void GenerateSites(TArray<TSharedPtr<T>>& OutSites)
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
                        FMath::IsNearlyEqual(CurrentSite->GetCoordinate().Y, CurrentIntersectionStar.Y) &&
                        CurrentSite->GetCoordinate().X < CurrentIntersectionStar.X
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
            TSharedPtr<T> GeneratedSite( new T(Site->Index, Site->GetCoordinate(), Site->Centroid, FColor::White, Site->bIsCorner, Site->bIsEdge));
            GeneratedSite->Vertices.Append(Site->Vertices);
            
            for(auto EdgeItr(Site->Edges.CreateConstIterator()); EdgeItr; ++EdgeItr)
            {
                TSharedPtr<FVoronoiDiagramEdge> Edge = (*EdgeItr);
                GeneratedSite->Edges.Add(FVoronoiDiagramGeneratedEdge(Edge->Index, Edge->LeftClippedEndPoint, Edge->RightClippedEndPoint));
        
                if(Edge->LeftSite.IsValid())
                {
                    GeneratedSite->NeighborSites.Add(Edge->LeftSite->Index);
                }
                
                if(Edge->RightSite.IsValid())
                {
                    GeneratedSite->NeighborSites.Add(Edge->RightSite->Index);
                }
            }
            OutSites.Add(GeneratedSite);
            
            // Finished with the edges, remove the references so they can be removed at the end of the method
            Site->Edges.Empty();
        }
        
        
        // Clean up
        BottomMostSite.Reset();
    }
    
    /*
     *  Bounds of the Voronoi Diagram
     */
    FIntRect Bounds;
    
private:
    /*
     *  Stored added points as Sites.  Ordered lexigraphically by y and then x
     */
    TArray<TSharedPtr<class FVoronoiDiagramSite>> Sites;
    
    /*
     *  Stores the bottom most site when running GenerateEdges
     */
    TSharedPtr<class FVoronoiDiagramSite> BottomMostSite;
    
    /*
     *  Stores the current site index when running GenerateEdges
     */
    int32 CurrentSiteIndex;
    
    /*
     *  Stores the minimum values of the points in site array.  Declared as pointer because these values need to be shared between some data structures.
     */
    TSharedPtr<FVector2D> MinValues;
    
    /*
     *  Stores the maximum values of the points in the site array.  Declared as pointer because these values need to be shared between some data structures.
     */
    TSharedPtr<FVector2D> MaxValues;
    
    /*
     *  Stores the delta values of the minimum and maximum values. Declared as pointer because these values need to be shared between some data structures.
     */
    TSharedPtr<FVector2D> DeltaValues;

    /*
     *  Returns the next site and increments CurrentSiteIndex
     */
    TSharedPtr<class FVoronoiDiagramSite> GetNextSite();
    
    /*
     *  Returns the left region in relation to a half edge
     */
    TSharedPtr<class FVoronoiDiagramSite> GetLeftRegion(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdge);
    
    /*
     *  Returns the right region in relation to a half edge
     */
    TSharedPtr<class FVoronoiDiagramSite> GetRightRegion(TSharedPtr<class FVoronoiDiagramHalfEdge> HalfEdge);
    
    friend class FVoronoiDiagramHelper;
};

class VORONOIDIAGRAM_API FVoronoiDiagramHelper
{
public:
	/*
	* Creates an array of colors of the Voronoi Diagram to the supplied TArray<FColor>.  Assumes that points have already been added to Voronoi Diagram.
	* This can be safely called from a thread
	*/
	static void GenerateColorArray(FVoronoiDiagram VoronoiDiagram, int32 RelaxationCycles, TArray<FColor>& ColorData);
	
	/*
     *  Creates a texture images of the Voronoi Diagram in the passed texture.  Assumes that points have already been added to the Voronoi Diagram
	 *  This has to be called from the game thread
     */
    static void GenerateTexture(FVoronoiDiagram VoronoiDiagram, int32 RelaxationCycles, class UTexture2D*& GeneratedTexture);
			
	/*
	 * Creates a PNG file of the Voronoi Diagram to the supplied TArray<uint8>.  Assumes that points have already been added to Voronoi Diagram 
	 * This has to be called from the game thread
	 */
	static void GeneratePNG(FVoronoiDiagram VoronoiDiagram, int32 RelaxationCycles, TArray<uint8>& PNGData);

    /*
     *  Does the passed in point lie inside of the vertices passed in.  The verices are assumed to be sorted.
     */
    static bool PointInVertices(FIntPoint Point, TArray<FVector2D> Vertices);
    
private:
    struct FLineSegment
    {
        FLineSegment(float InY, float InXl, float InXr, float InDy)
        : y(InY)
        , xl(InXl)
        , xr(InXr)
        , dy(InDy)
        {}
        
        float y, xl, xr, dy;
    };
};
