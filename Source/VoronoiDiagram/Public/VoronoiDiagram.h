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
    TArray<TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe>> Hash;
	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> LeftEnd;
	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> RightEnd;
	TSharedPtr<FVector2D, ESPMode::ThreadSafe> MinimumValues;
	TSharedPtr<FVector2D, ESPMode::ThreadSafe> DeltaValues;
    
	FVoronoiDiagramEdgeList(int32 NumberOfSites, TSharedPtr<FVector2D, ESPMode::ThreadSafe> InMinimumValues, TSharedPtr<FVector2D, ESPMode::ThreadSafe> InDeltaValues)
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
    
	void Insert(TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> LeftBound, TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> NewHalfEdge)
    {
        NewHalfEdge->EdgeListLeft = LeftBound;
        NewHalfEdge->EdgeListRight = LeftBound->EdgeListRight;
        LeftBound->EdgeListRight->EdgeListLeft = NewHalfEdge;
        LeftBound->EdgeListRight = NewHalfEdge;
    }
    
	void Delete(TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge)
    {
        HalfEdge->EdgeListLeft->EdgeListRight = HalfEdge->EdgeListRight;
        HalfEdge->EdgeListRight->EdgeListLeft = HalfEdge->EdgeListLeft;
        HalfEdge->Edge = FVoronoiDiagramEdge::DELETED;
        HalfEdge->EdgeListLeft = nullptr;
        HalfEdge->EdgeListRight = nullptr;
    }

	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> GetFromHash(int32 Bucket)
    {
		TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge;
        
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
    
	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> GetLeftBoundFrom(FVector2D Point)
    {
        int32 Bucket;
		TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge;
        
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
	TArray<TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe>> Hash;
	TSharedPtr<FVector2D, ESPMode::ThreadSafe> MinimumValues;
	TSharedPtr<FVector2D, ESPMode::ThreadSafe> DeltaValues;
    
	FVoronoiDiagramPriorityQueue(int32 NumberOfSites, TSharedPtr<FVector2D, ESPMode::ThreadSafe> InMinimumValues, TSharedPtr<FVector2D, ESPMode::ThreadSafe> InDeltaValues)
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

	int32 GetBucket(TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge)
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
    
	void Insert(TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge)
    {
		TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> Previous, Next;
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
    
	void Delete(TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge)
    {
		TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> Previous;
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
    
	TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> RemoveAndReturnMinimum()
    {
		TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> MinEdge;

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
     */
	void GenerateSites(int32 RelaxationCycles);
    
    /*
     *  Bounds of the Voronoi Diagram
     */
    FIntRect Bounds;

	/*
	 * Generated sites.  Filled after GenerateSites() is called
	 */
	TMap<int32, FVoronoiDiagramGeneratedSite> GeneratedSites;

private:
	/*
	*  Original points added by the user.  Ordered lexigraphically by y and then x
	*/
	TArray<FVector2D> OriginalSites;
	
	/*
     *  Stored added points as Sites that are currently being processed.  Ordered lexigraphically by y and then x
     */
	TArray<TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe>> Sites;
    
    /*
     *  Stores the bottom most site when running GenerateEdges
     */
	TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> BottomMostSite;
    
    /*
     *  Stores the current site index when running GenerateEdges
     */
    int32 CurrentSiteIndex;
    
    /*
     *  Stores the minimum values of the points in site array.  Declared as pointer because these values need to be shared between some data structures.
     */
	TSharedPtr<FVector2D, ESPMode::ThreadSafe> MinValues;
    
    /*
     *  Stores the maximum values of the points in the site array.  Declared as pointer because these values need to be shared between some data structures.
     */
	TSharedPtr<FVector2D, ESPMode::ThreadSafe> MaxValues;
    
    /*
     *  Stores the delta values of the minimum and maximum values. Declared as pointer because these values need to be shared between some data structures.
     */
	TSharedPtr<FVector2D, ESPMode::ThreadSafe> DeltaValues;

	/*
	 *	Sorts sites and calculates MinValues, MaxValues, and DeltaValues
	 */
	void SortSitesAndSetValues();

    /*
     *  Returns the next site and increments CurrentSiteIndex
     */
	TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> GetNextSite();
    
    /*
     *  Returns the left region in relation to a half edge
     */
	TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> GetLeftRegion(TSharedPtr<class FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge);
    
    /*
     *  Returns the right region in relation to a half edge
     */
	TSharedPtr<class FVoronoiDiagramSite, ESPMode::ThreadSafe> GetRightRegion(TSharedPtr<class FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge);
    
    friend class FVoronoiDiagramHelper;
};

class VORONOIDIAGRAM_API FVoronoiDiagramHelper
{
public:
		
	/*
	* Creates an array of colors of the Voronoi Diagram to the supplied TArray<FColor>.  Assumes that diagram has been run through GenerateDiagram
	* This can be safely called from a thread
	*/
	static void GenerateColorArray(TSharedPtr<FVoronoiDiagram> VoronoiDiagram, TArray<FColor>& ColorData);
	
	/*
     *  Creates a texture images of the Voronoi Diagram in the passed texture.  Assumes that diagram has been run through GenerateDiagram
	 *  This has to be called from the game thread
     */
	static void GenerateTexture(TSharedPtr<FVoronoiDiagram> VoronoiDiagram, class UTexture2D*& GeneratedTexture);
			
	/*
	 * Creates a PNG file of the Voronoi Diagram to the supplied TArray<uint8>.  Assumes that diagram has been run through GenerateDiagram
	 * This has to be called from the game thread
	 */
	static void GeneratePNG(TSharedPtr<FVoronoiDiagram> VoronoiDiagram, TArray<uint8>& PNGData);

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
