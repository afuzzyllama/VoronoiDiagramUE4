// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramModule.h"
#include "VoronoiDiagram.h"

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

    for(auto Itr(Sites.CreateConstIterator()); Itr; ++Itr)
    {
        UE_LOG(LogVoronoiDiagram, Log, TEXT("Site #%i: (%f, %f)"), (*Itr)->Index, (*Itr)->GetCoordinate().X, (*Itr)->GetCoordinate().Y);
    }

    return true;
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
    
    TArray<TSharedPtr<FVoronoiDiagramGeneratedSite>> GeneratedSites;
    VoronoiDiagram.GenerateSites(GeneratedSites);
    // Lloyd's Algorithm
    for(int32 Cycles = 0; Cycles < RelaxationCycles; ++Cycles)
    {
        FVoronoiDiagram CurrentDiagram(VoronoiDiagram.Bounds);

        TArray<FIntPoint> CurrentPoints;
        for(auto Itr(GeneratedSites.CreateConstIterator()); Itr; ++Itr)
        {
            TSharedPtr<FVoronoiDiagramGeneratedSite> CurrentSite = *Itr;
            CurrentPoints.Add(FIntPoint(FMath::RoundToInt(CurrentSite->Centroid.X), FMath::RoundToInt(CurrentSite->Centroid.Y)));
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
        TSharedPtr<FVoronoiDiagramGeneratedSite> CurrentSite = *SiteItr;
        CurrentSite->Color = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255));
        
        if(CurrentSite->Vertices.Num() == 0)
        {
            continue;
        }
        
        FVector2D MinimumVertex = CurrentSite->Vertices[0];
        FVector2D MaximumVertex = CurrentSite->Vertices[0];
        
        for(int32 i = 1; i < CurrentSite->Vertices.Num(); ++i)
        {
            if( CurrentSite->Vertices[i].X < MinimumVertex.X )
            {
                MinimumVertex.X = CurrentSite->Vertices[i].X;
            }

            if( CurrentSite->Vertices[i].Y < MinimumVertex.Y )
            {
                MinimumVertex.Y = CurrentSite->Vertices[i].Y;
            }
            
            if( CurrentSite->Vertices[i].X > MaximumVertex.X )
            {
                MaximumVertex.X = CurrentSite->Vertices[i].X;
            }

            if( CurrentSite->Vertices[i].Y > MaximumVertex.Y )
            {
                MaximumVertex.Y = CurrentSite->Vertices[i].Y;
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
                if(FVoronoiDiagramHelper::PointInVertices(FIntPoint(x, y), CurrentSite->Vertices))
                {
                    FVoronoiDiagramHelper::DrawOnMipData(MipData, CurrentSite->Color, x, y, VoronoiDiagram.Bounds);
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























































