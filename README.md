VoronoiDiagram
==============

This repository contains code that can generate a [Voronoi Diagram](http://en.wikipedia.org/wiki/Voronoi_diagram) by using an implementation of [Fortune's Algorithm](http://en.wikipedia.org/wiki/Fortune's_algorithm) for the Unreal Engine.  

The parameter of `GenerateSites()` will run [Lloyd's Algorithm](http://en.wikipedia.org/wiki/Lloyd's_algorithm) the specified amount of times. 

To use, clone the code into a directory named 'VoronoiDiagram' in your Unreal project's plugin directory.  Don't forget to add a public dependency to your project for the plugin, or else your Unreal project will not be able to find it.

The following code:

    #include "VoronoiDiagram.h"

    UTexture2D* MyTexture;
    
    TSharedPtr<FVoronoiDiagram> VoronoiDiagram(new FVoronoiDiagram(FIntRect(0, 0, 4096, 4096)));
    TArray<FIntPoint> Points;
    
    for(int32 i = 0; i < 1000; ++i)
    {
        Points.AddUnique(FIntPoint(FMath::RandRange(0, 4095), FMath::RandRange(0, 4095)));
    }
    VoronoiDiagram.AddPoints(Points);
	
	VoronoiDiagram->GenerateSites(2);
	
	for(int32 i = 0; i < VoronoiDiagram->GeneratedSites.Num(); ++i)
	{
		VoronoiDiagram->GeneratedSites[i].Color = FColor::MakeRandomColor();
	}
	
    FVoronoiDiagramHelper::GenerateTexture(VoronoiDiagram, MyTexture);
    
Will create a texture similar to:

![Voronoi Diagram](../../../Screenshots/blob/master/VoronoiDiagram.png?raw=true "Voronoi Diagram")

Citations:
----------
Fortune's Algorithm as outlined in:
Steve J. Fortune (1987). "A Sweepline Algorithm for Voronoi Diagrams". Algorithmica 2, 153-174. 

Lloyd's algorithm as outlined in:
Lloyd, Stuart P. (1982), "Least squares quantization in PCM", IEEE Transactions on Information Theory 28 (2): 129–137

Monotone Chain Convex Hull Algorithm outlined in:
A. M. Andrew, "Another Efficient Algorithm for Convex Hulls in Two Dimensions", Info. Proc. Letters 9, 216-219 (1979)

Based off of:
---------
[as3delaunay](http://nodename.github.io/as3delaunay/)


