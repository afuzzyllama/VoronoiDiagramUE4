// Copyright 2015 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagram.h"

#include "ImageUtils.h"

FVoronoiDiagram::FVoronoiDiagram(FIntRect InBounds)
: Bounds(InBounds)
{
	GeneratedSites.Empty();
}

bool FVoronoiDiagram::AddPoints(const TArray<FIntPoint>& Points)
{
	for (auto Itr(Points.CreateConstIterator()); Itr; ++Itr)
	{
		FIntPoint CurrentPoint = *Itr;
		if (!Bounds.Contains(CurrentPoint))
		{
			UE_LOG(LogVoronoiDiagram, Error, TEXT("Point (%i, %i) out of diagram bounds (%i, %i)"), CurrentPoint.X, CurrentPoint.Y, Bounds.Width(), Bounds.Height());
			return false;
		}
	}

	for (auto Itr(Points.CreateConstIterator()); Itr; ++Itr)
	{
		FIntPoint CurrentPoint = *Itr;
		OriginalSites.Add(FVector2D(static_cast<float>(CurrentPoint.X), static_cast<float>(CurrentPoint.Y)));
	}

	return true;
}

void FVoronoiDiagram::GenerateSites(int32 RelaxationCycles)
{
	if (OriginalSites.Num() == 0)
	{
		UE_LOG(LogVoronoiDiagram, Error, TEXT("No points added to the diagram.  Sites cannot be generated"));
		return;
	}

	Sites.Empty();
	for (int32 i = 0; i < OriginalSites.Num(); ++i)
	{
		Sites.Add(FVoronoiDiagramSite::CreatePtr(Sites.Num(), OriginalSites[i]));
	}
	SortSitesAndSetValues();

	// Cycles related to Lloyd's algorithm
	for (int32 Cycles = 0; Cycles < RelaxationCycles; ++Cycles)
	{
		// Fortune's Algorithm
		int32 NumGeneratedEdges = 0;
		int32 NumGeneratedVertices = 0;
		CurrentSiteIndex = 0;

		FVoronoiDiagramPriorityQueue PriorityQueue(Sites.Num(), MinValues, DeltaValues);
		FVoronoiDiagramEdgeList EdgeList(Sites.Num(), MinValues, DeltaValues);

		FVector2D CurrentIntersectionStar;
		TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> CurrentSite, BottomSite, TopSite, TempSite;
		TSharedPtr<FVoronoiDiagramVertex, ESPMode::ThreadSafe> v, Vertex;
		TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> LeftBound, RightBound, LeftLeftBound, RightRightBound, Bisector;
		TSharedPtr<FVoronoiDiagramEdge, ESPMode::ThreadSafe> Edge;
		EVoronoiDiagramEdge::Type EdgeType;

		TArray<TSharedPtr<FVoronoiDiagramEdge, ESPMode::ThreadSafe>> GeneratedEdges;

		bool bDone = false;
		BottomMostSite = GetNextSite();
		CurrentSite = GetNextSite();
		while (!bDone)
		{
			if (!PriorityQueue.IsEmpty())
			{
				CurrentIntersectionStar = PriorityQueue.GetMinimumBucketFirstPoint();
			}

			if (
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
				if (Vertex.IsValid())
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
				if (Vertex.IsValid())
				{
					Bisector->Vertex = Vertex;
					Bisector->YStar = Vertex->GetCoordinate().Y + CurrentSite->GetDistanceFrom(Vertex);

					PriorityQueue.Insert(Bisector);
				}

				CurrentSite = GetNextSite();
			}
			else if (PriorityQueue.IsEmpty() == false)
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
				if (BottomSite->GetCoordinate().Y > TopSite->GetCoordinate().Y)
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

				if (EdgeType == EVoronoiDiagramEdge::Left)
				{
					Edge->SetEndpoint(v, EVoronoiDiagramEdge::Right);
				}
				else
				{
					Edge->SetEndpoint(v, EVoronoiDiagramEdge::Left);
				}

				Vertex = FVoronoiDiagramVertex::Intersect(LeftLeftBound, Bisector);
				if (Vertex.IsValid())
				{
					PriorityQueue.Delete(LeftLeftBound);

					LeftLeftBound->Vertex = Vertex;
					LeftLeftBound->YStar = Vertex->GetCoordinate().Y + BottomSite->GetDistanceFrom(Vertex);

					PriorityQueue.Insert(LeftLeftBound);
				}

				Vertex = FVoronoiDiagramVertex::Intersect(Bisector, RightRightBound);
				if (Vertex.IsValid())
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

		GeneratedSites.Empty();
		// Bound the edges of the diagram
		for (auto Itr(GeneratedEdges.CreateConstIterator()); Itr; ++Itr)
		{
			TSharedPtr<FVoronoiDiagramEdge, ESPMode::ThreadSafe> Edge = (*Itr);
			Edge->GenerateClippedEndPoints(Bounds);
		}

		for (auto Itr(Sites.CreateConstIterator()); Itr; ++Itr)
		{
			TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> Site = (*Itr);
			Site->GenerateCentroid(Bounds);
		}

		for (auto SiteItr(Sites.CreateConstIterator()); SiteItr; ++SiteItr)
		{
			TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> Site = (*SiteItr);
			FVoronoiDiagramGeneratedSite GeneratedSite(Site->Index, Site->GetCoordinate(), Site->Centroid, FColor::White, Site->bIsCorner, Site->bIsEdge);
			GeneratedSite.Vertices.Append(Site->Vertices);

			for (auto EdgeItr(Site->Edges.CreateConstIterator()); EdgeItr; ++EdgeItr)
			{
				TSharedPtr<FVoronoiDiagramEdge, ESPMode::ThreadSafe> Edge = (*EdgeItr);
				GeneratedSite.Edges.Add(FVoronoiDiagramGeneratedEdge(Edge->Index, Edge->LeftClippedEndPoint, Edge->RightClippedEndPoint));

				if (Edge->LeftSite.IsValid())
				{
					GeneratedSite.NeighborSites.Add(Edge->LeftSite->Index);
				}

				if (Edge->RightSite.IsValid())
				{
					GeneratedSite.NeighborSites.Add(Edge->RightSite->Index);
				}
			}
			GeneratedSites.Add(GeneratedSite);

			// Finished with the edges, remove the references so they can be removed at the end of the method
			Site->Edges.Empty();
		}

		// Clean up
		BottomMostSite.Reset();
		Sites.Empty();

		// Lloyd's Algorithm
		TArray<FVector2D> NewPoints;
		for (auto Itr(GeneratedSites.CreateConstIterator()); Itr; ++Itr)
		{
			FVoronoiDiagramGeneratedSite CurrentSite = *Itr;

			FIntPoint CentroidPoint(FMath::RoundToInt(CurrentSite.Centroid.X), FMath::RoundToInt(CurrentSite.Centroid.Y));
			if (!Bounds.Contains(CentroidPoint))
			{
				UE_LOG(LogVoronoiDiagram, Error, TEXT("Centroid point outside of diagram bounds"));
				return;
			}

			Sites.Add(FVoronoiDiagramSite::CreatePtr(Sites.Num(), FVector2D(static_cast<float>(CentroidPoint.X), static_cast<float>(CentroidPoint.Y))));
		}
		SortSitesAndSetValues();
	}
}

void FVoronoiDiagram::SortSitesAndSetValues()
{
	struct FSortSite
	{
		bool operator()(const TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe>& A, const TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe>& B) const
		{
			if (FMath::RoundToInt(A->GetCoordinate().Y) < FMath::RoundToInt(B->GetCoordinate().Y))
			{
				return true;
			}

			if (FMath::RoundToInt(A->GetCoordinate().Y) > FMath::RoundToInt(B->GetCoordinate().Y))
			{
				return false;
			}

			if (FMath::RoundToInt(A->GetCoordinate().X) < FMath::RoundToInt(B->GetCoordinate().X))
			{
				return true;
			}

			if (FMath::RoundToInt(A->GetCoordinate().X) > FMath::RoundToInt(B->GetCoordinate().X))
			{
				return false;
			}

			return false;
		}
	};
	Sites.Sort(FSortSite());

	FVector2D CurrentMin(FLT_MAX, FLT_MAX);
	FVector2D CurrentMax(FLT_MIN, FLT_MIN);
	for (auto Itr(Sites.CreateConstIterator()); Itr; ++Itr)
	{
		FVector2D CurrentPoint = (*Itr)->GetCoordinate();
		if (CurrentPoint.X < CurrentMin.X)
		{
			CurrentMin.X = CurrentPoint.X;
		}

		if (CurrentPoint.X > CurrentMax.X)
		{
			CurrentMax.X = CurrentPoint.X;
		}

		if (CurrentPoint.Y < CurrentMin.Y)
		{
			CurrentMin.Y = CurrentPoint.Y;
		}

		if (CurrentPoint.Y > CurrentMax.Y)
		{
			CurrentMax.Y = CurrentPoint.Y;
		}
	}

	MinValues = MakeShareable(new FVector2D(CurrentMin));
	MaxValues = MakeShareable(new FVector2D(CurrentMax));
	DeltaValues = MakeShareable(new FVector2D(CurrentMax.X - CurrentMin.X, CurrentMax.Y - CurrentMin.Y));

}

TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> FVoronoiDiagram::GetNextSite()
{
	TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> NextSite;

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

TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> FVoronoiDiagram::GetLeftRegion(TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge)
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

TSharedPtr<FVoronoiDiagramSite, ESPMode::ThreadSafe> FVoronoiDiagram::GetRightRegion(TSharedPtr<FVoronoiDiagramHalfEdge, ESPMode::ThreadSafe> HalfEdge)
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

void FVoronoiDiagramHelper::GenerateColorArray(TSharedPtr<FVoronoiDiagram> VoronoiDiagram, TArray<FColor>& ColorData)
{
	ColorData.Empty();
	ColorData.SetNum(VoronoiDiagram->Bounds.Width() * VoronoiDiagram->Bounds.Height());

	for (int32 x = 0; x < VoronoiDiagram->Bounds.Width(); ++x)
	{
		for (int32 y = 0; y < VoronoiDiagram->Bounds.Height(); ++y)
		{
			ColorData[x + y * VoronoiDiagram->Bounds.Width()] = FColor::White;
		}
	}

	for (auto SiteItr(VoronoiDiagram->GeneratedSites.CreateConstIterator()); SiteItr; ++SiteItr)
	{
		FVoronoiDiagramGeneratedSite CurrentSite = *SiteItr;

		if (CurrentSite.Vertices.Num() == 0)
		{
			continue;
		}
		
		FVector2D MinimumVertex = CurrentSite.Vertices[0];
		FVector2D MaximumVertex = CurrentSite.Vertices[0];

		for (int32 i = 1; i < CurrentSite.Vertices.Num(); ++i)
		{
			if (CurrentSite.Vertices[i].X < MinimumVertex.X)
			{
				MinimumVertex.X = CurrentSite.Vertices[i].X;
			}

			if (CurrentSite.Vertices[i].Y < MinimumVertex.Y)
			{
				MinimumVertex.Y = CurrentSite.Vertices[i].Y;
			}

			if (CurrentSite.Vertices[i].X > MaximumVertex.X)
			{
				MaximumVertex.X = CurrentSite.Vertices[i].X;
			}

			if (CurrentSite.Vertices[i].Y > MaximumVertex.Y)
			{
				MaximumVertex.Y = CurrentSite.Vertices[i].Y;
			}
		}

		if (MinimumVertex.X < 0.0f)
		{
			MinimumVertex.X = 0.0f;
		}

		if (MinimumVertex.Y < 0.0f)
		{
			MinimumVertex.Y = 0.0f;
		}

		if (MaximumVertex.X > VoronoiDiagram->Bounds.Width())
		{
			MaximumVertex.X = VoronoiDiagram->Bounds.Width();
		}

		if (MaximumVertex.Y > VoronoiDiagram->Bounds.Height())
		{
			MaximumVertex.Y = VoronoiDiagram->Bounds.Height();
		}

		for (int32 x = MinimumVertex.X; x <= MaximumVertex.X; ++x)
		{
			for (int32 y = MinimumVertex.Y; y <= MaximumVertex.Y; ++y)
			{
				if (FVoronoiDiagramHelper::PointInVertices(FIntPoint(x, y), CurrentSite.Vertices))
				{
					if (VoronoiDiagram->Bounds.Contains(FIntPoint(x, y)))
					{
						int32 Index = x + y * VoronoiDiagram->Bounds.Width();
						ColorData[Index] = CurrentSite.Color;
					}
				}
			}
		}
	}

}

void FVoronoiDiagramHelper::GenerateTexture(TSharedPtr<FVoronoiDiagram> VoronoiDiagram, UTexture2D*& GeneratedTexture)
{
    TArray<FColor> ColorData;
	FVoronoiDiagramHelper::GenerateColorArray(VoronoiDiagram, ColorData);

	GeneratedTexture = UTexture2D::CreateTransient(VoronoiDiagram->Bounds.Width(), VoronoiDiagram->Bounds.Height());
    FColor* MipData = static_cast<FColor*>(GeneratedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
    for(int32 x = 0; x < VoronoiDiagram->Bounds.Width(); ++x)
    {
        for(int32 y = 0; y < VoronoiDiagram->Bounds.Height(); ++y)
        {
            MipData[x + y * VoronoiDiagram->Bounds.Width()] = ColorData[x + y * VoronoiDiagram->Bounds.Width()];
        }
    }
    
    // Unlock the texture
    GeneratedTexture->PlatformData->Mips[0].BulkData.Unlock();
    GeneratedTexture->UpdateResource();
}

void FVoronoiDiagramHelper::GeneratePNG(TSharedPtr<FVoronoiDiagram> VoronoiDiagram, TArray<uint8>& PNGData)
{
	TArray<FColor> ColorData;
	FVoronoiDiagramHelper::GenerateColorArray(VoronoiDiagram, ColorData);

	PNGData.Empty();
	FImageUtils::CompressImageArray(
		VoronoiDiagram->Bounds.Width(),
		VoronoiDiagram->Bounds.Height(),
		ColorData,
		PNGData
	);

	return;
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
