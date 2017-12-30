#pragma once
#include "MeshResampler.h"

void MeshResampler::upsample(HalfedgeMesh& mesh)
	// This routine should increase the number of triangles in the mesh using Loop subdivision.
{
	// Each vertex and edge of the original surface can be associated with a vertex in the new (subdivided) surface.
	// Therefore, our strategy for computing the subdivided vertex locations is to *first* compute the new positions
	// using the connectity of the original (coarse) mesh; navigating this mesh will be much easier than navigating
	// the new subdivided (fine) mesh, which has more elements to traverse.  We will then assign vertex positions in
	// the new mesh based on the values we computed for the original mesh.


	// TODO Compute new positions for all the vertices in the input mesh, using the Loop subdivision rule,
	// TODO and store them in Vertex::newPosition. At this point, we also want to mark each vertex as being
	// TODO a vertex of the original mesh.
	for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++) {
		v->isNew = false;

		if (!v->isBoundary()) {
			float n = float(v->degree());
			float u;

			HalfedgeIter h = v->halfedge();

			if (n == 3.0f) {
				u = 3.0f / 16.0f;
			}
			else {
				u = 3.0f / (8.0f * n);
			}

			//Reset newPosition
			v->newPosition = glm::vec3(0.0f);
			v->newPosition += (1.0f - n * u) * v->position;

			do {
				HalfedgeIter hTwin = h->twin();
				v->newPosition += u * hTwin->vertex()->position;
				h = hTwin->next();
			} while (h != v->halfedge());
		}
		else {
			v->newPosition = v->position;
		}
	}

	// TODO Next, compute the updated vertex positions associated with edges, and store it in Edge::newPosition.
	for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
		e->isNew = false;

		glm::vec3 edgeV0 = e->halfedge()->vertex()->position;
		glm::vec3 edgeV1 = e->halfedge()->twin()->vertex()->position;

		glm::vec3 up = e->halfedge()->next()->next()->vertex()->position;
		glm::vec3 down = e->halfedge()->twin()->next()->next()->vertex()->position;

		e->newPosition = (3.0f / 8.0f) * (edgeV0 + edgeV1) + (1.0f / 8.0f) * (up + down);
	}


	// TODO Next, we're going to split every edge in the mesh, in any order.  For future
	// TODO reference, we're also going to store some information about which subdivided
	// TODO edges come from splitting an edge in the original mesh, and which edges are new,
	// TODO by setting the flat Edge::isNew.  Note that in this loop, we only want to iterate
	// TODO over edges of the original mesh---otherwise, we'll end up splitting edges that we
	// TODO just split (and the loop will never end!)

	EdgeIter eSplit = mesh.edgesBegin();

	//while (eSplit != eEnd)
	while (eSplit != mesh.edgesEnd())
	{
		EdgeIter nextEdge = eSplit;
		nextEdge++;

		//We only want to split the edges of the old Mesh (these are isNew = false)
		if (!eSplit->isNew) {

			glm::vec3 newVertexPos = eSplit->newPosition;

			VertexIter vSplit = mesh.splitEdge(eSplit);
			vSplit->newPosition = newVertexPos;
			vSplit->isNew = true;

			//All edgesconnected to the newly created Vertex are new Edges 
			HalfedgeIter hSplit = vSplit->halfedge();
			do {
				hSplit->edge()->isNew = true;

				hSplit = hSplit->twin();
				hSplit = hSplit->next();
			} while (hSplit != vSplit->halfedge());
		}

		eSplit = nextEdge;
	}

	// TODO Now flip any new edge that connects an old and new vertex.
	/*for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
		VertexIter v0 = e->halfedge()->vertex();
		VertexIter v1 = e->halfedge()->twin()->vertex();
		if ((v0->isNew && !v1->isNew && e->isNew) || (!v0->isNew && v1->isNew && e->isNew)) {
			mesh.flipEdge(e);
		}
	}*/

	// TODO Finally, copy the new vertex positions into final Vertex::position.
	//for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++) {
	//	//if (!v->isBoundary()) {
	//		v->position = v->newPosition;
	//	//}
	//}
}

void MeshResampler::downsample(HalfedgeMesh& mesh)
{
	// TODO Compute initial quadrics for each face by simply writing the plane
	// equation for the face in homogeneous coordinates.  These quadrics should
	// be stored in Face::quadric


	// TODO Compute an initial quadric for each vertex as the sum of the quadrics
	// associated with the incident faces, storing it in Vertex::quadric


	// TODO Build a priority queue of edges according to their quadric error cost,
	// TODO i.e., by building an EdgeRecord for each edge and sticking it in the queue.


	// TODO Until we reach the target edge budget, collapse the best edge.  Remember
	// TODO to remove from the queue any edge that touches the collapsing edge BEFORE
	// TODO it gets collapsed, and add back into the queue any edge touching the collapsed
	// TODO vertex AFTER it's been collapsed.  Also remember to assign a quadric to the
	// TODO collapsed vertex, and to pop the collapsed edge off the top of the queue.
}

void MeshResampler::resample(HalfedgeMesh& mesh)
{
	// TODO Compute the mean edge length.


	// TODO Repeat the four main steps for 5 or 6 iterations


	// TODO Split edges much longer than the target length (being careful about how the loop is written!)


	// TODO Collapse edges much shorter than the target length.  Here we need to be EXTRA careful about
	// TODO advancing the loop, because many edges may have been destroyed by a collapse (which ones?)

	//
	// TODO Now flip each edge if it improves vertex degree


	// TODO Finally, apply some tangential smoothing to the vertex positions
}
