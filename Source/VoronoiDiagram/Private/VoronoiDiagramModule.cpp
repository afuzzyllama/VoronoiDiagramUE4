// Copyright 2014 afuzzyllama. All Rights Reserved.

#include "VoronoiDiagramPrivatePCH.h"
#include "VoronoiDiagramModule.h"

IMPLEMENT_MODULE( FVoronoiDiagramModule, VoronoiDiagram )
DEFINE_LOG_CATEGORY(LogVoronoiDiagram);

void FVoronoiDiagramModule::StartupModule()
{}

void FVoronoiDiagramModule::ShutdownModule()
{}

FVoronoiDiagram Generate(FIntRect Bounds, TArray<FIntPoint>& Points)
{
    FVoronoiDiagram VoronoiDiagram(Bounds);
    VoronoiDiagram.AddPoints(Points);
    
    return VoronoiDiagram;

}
#undef LOCTEXT_NAMESPACE
