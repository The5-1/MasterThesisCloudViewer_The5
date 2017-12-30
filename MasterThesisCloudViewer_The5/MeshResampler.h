#pragma once
#include "halfEdgeMesh.h"

using namespace std;

class MeshResampler {

public:

	MeshResampler() {};
	~MeshResampler() {}

	void upsample(HalfedgeMesh& mesh);
	void downsample(HalfedgeMesh& mesh);
	void resample(HalfedgeMesh& mesh);
};

