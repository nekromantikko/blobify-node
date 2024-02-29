#pragma once
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MIntArray.h>

struct MarchingCube {
	MPoint vertices[8];
	float cachedPointValues[8] = { 0,0,0,0,0,0,0,0 };

	MarchingCube(MPoint p, float d);
};

void polygonizeMetaballs(const MPointArray& metaballOrigins, float threshold, float triangleSize, MBoundingBox bounds, MFloatPointArray& outVertices, MFloatVectorArray& outNormals, MIntArray& outIndices);