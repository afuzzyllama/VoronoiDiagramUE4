VoronoiDiagram
==============

This repository contains code that can generate a [Voronoi Diagram](http://en.wikipedia.org/wiki/Voronoi_diagram) by using an implemtation of [Fortune's Algorithm](http://en.wikipedia.org/wiki/Fortune's_algorithm) for the Unreal Engine.  

The following code:

    UTexture2D* MyTexture;
    
    FVoronoiDiagram VoronoiDiagram(FIntRect(0, 0, 512, 512));
    TArray<FIntPoint> Points;
    
    for(int32 i = 0; i < 100; ++i)
    {
        Points.AddUnique(FIntPoint(FMath::RandRange(0, 511), FMath::RandRange(0, 511)));
    }
    VoronoiDiagram.AddPoints(Points);

    FVoronoiDiagramHelper::GenerateTexture(VoronoiDiagram, MyTexture);
    
Will create a texture similar to:
![Voronoi Diagram](../../../Screenshots/blob/master/VoronoiDiagram.png?raw=true "Voronoi Diagram")

Citations:
==========
Fortune's Algorithm as outlined in:
Steve J. Fortune (1987). "A Sweepline Algorithm for Voronoi Diagrams". Algorithmica 2, 153-174. 

Bresenham's line algorithm as outlined in:
Bresenham, J. E. (1 January 1965). "Algorithm for computer control of a digital plotter". IBM Systems Journal 4 (1): 25–30


