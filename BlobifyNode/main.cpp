#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MBoundingBox.h>
#include "marching_cubes.h"

class BlobifyNode : public MPxNode {
public:

	static void* creator();
	static MStatus initialize();

	MStatus compute(const MPlug& plug, MDataBlock& data) override;

	static const MTypeId id;

	static MObject inMesh;
	static MObject inThreshold;
	static MObject inTriSize;

	static MObject outMesh;
};

const MTypeId BlobifyNode::id(0x70000);

MObject BlobifyNode::inMesh;
MObject BlobifyNode::inThreshold;
MObject BlobifyNode::inTriSize;
MObject BlobifyNode::outMesh;

void* BlobifyNode::creator() {
	return new BlobifyNode();
}

MStatus BlobifyNode::initialize() {
	MFnNumericAttribute fnAttrib;
	MFnTypedAttribute fnTypedAttrib;

	inMesh = fnTypedAttrib.create("inMesh", "im", MFnData::kMesh);
	addAttribute(inMesh);

	inThreshold = fnAttrib.create("threshold", "th", MFnNumericData::kFloat, 10.0f);
	fnAttrib.setReadable(false);
	fnAttrib.setKeyable(true);
	fnAttrib.setSoftMin(0.0f);
	fnAttrib.setSoftMax(25.0f);
	addAttribute(inThreshold);

	inTriSize = fnAttrib.create("meshTriangleSize", "ts", MFnNumericData::kFloat, 0.1f);
	fnAttrib.setReadable(false);
	fnAttrib.setKeyable(true);
	fnAttrib.setSoftMin(0.001f);
	fnAttrib.setSoftMax(10.0f);
	addAttribute(inTriSize);

	outMesh = fnTypedAttrib.create("outMesh", "om", MFnData::kMesh);
	fnTypedAttrib.setWritable(false);
	addAttribute(outMesh);

	attributeAffects(inMesh, outMesh);
	attributeAffects(inThreshold, outMesh);
	attributeAffects(inTriSize, outMesh);

	return MStatus::kSuccess;
}

MStatus BlobifyNode::compute(const MPlug& plug, MDataBlock& data) {
	if (plug != outMesh) {
		return MStatus::kUnknownParameter;
	}

	MPlug inMeshPlug(thisMObject(), inMesh);
	if (!inMeshPlug.isConnected()) {
		data.setClean(plug);
		return MStatus::kSuccess;
	}

	MDataHandle inMeshHandle = data.inputValue(inMesh);
	MObject mesh = inMeshHandle.asMesh();
	MFnMesh fnMesh(mesh);
	MPointArray meshPoints;
	fnMesh.getPoints(meshPoints, MSpace::kObject);

	MDataHandle inThresholdHandle = data.inputValue(inThreshold);
	const float threshold = inThresholdHandle.asFloat();

	MDataHandle inTriSizeHandle = data.inputValue(inTriSize);
	const float triSize = inTriSizeHandle.asFloat();

	// Test stuff
	MPoint corner(2.0f, 2.0f, 2.0f, 1.0f);
	MBoundingBox bounds(corner * -1, corner);

	MFloatPointArray outVertices;
	MFloatVectorArray outNormals;
	MIntArray outIndices;
	polygonizeMetaballs(meshPoints, threshold, triSize, bounds, outVertices, outNormals, outIndices);

	unsigned int triCount = outIndices.length() / 3;
	MIntArray polygonCounts(triCount, 3);

	MStatus status;

	MFnMeshData fnMeshData;
	MObject outMeshData = fnMeshData.create(&status);
	if (status != MStatus::kSuccess) {
		return status;
	}

	fnMesh.create(outVertices.length(), triCount, outVertices, polygonCounts, outIndices, outMeshData, &status);
	if (status != MStatus::kSuccess) {
		return status;
	}

	status = fnMesh.setNormals(outNormals);
	if (status != MStatus::kSuccess) {
		return status;
	}

	MDataHandle outMeshHandle = data.outputValue(outMesh);
	outMeshHandle.set(outMeshData);
	data.setClean(plug);
	return MStatus::kSuccess;
}

__declspec(dllexport) MStatus initializePlugin(MObject obj) {
	MFnPlugin plugin(obj);
	MStatus status;

	status = plugin.registerNode(
		"blobifyMesh",
		BlobifyNode::id,
		&BlobifyNode::creator,
		&BlobifyNode::initialize);

	return status;
}

__declspec(dllexport) MStatus uninitializePlugin(MObject obj) {
	MFnPlugin plugin(obj);
	MStatus status;

	status = plugin.deregisterNode(BlobifyNode::id);

	return status;
}
