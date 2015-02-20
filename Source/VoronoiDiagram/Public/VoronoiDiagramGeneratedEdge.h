// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

/*
 *  Stores final information about an edge that is returned in GenerateEdges
 */
class FVoronoiDiagramGeneratedEdge
{
public:
    FVoronoiDiagramGeneratedEdge(int32 InIndex, FVector2D InLeftEndPoint, FVector2D InRightEndPoint);

    int32 Index;
    FVector2D LeftEndPoint;
    FVector2D RightEndPoint;
};