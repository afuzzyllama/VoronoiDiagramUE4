// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

/*
 *  Interface for any class that needs to be a point in a Voronoi Diagram
 */
class IVoronoiDiagramPoint
{
public:
    virtual FVector2D GetCoordinate() const = 0;
};