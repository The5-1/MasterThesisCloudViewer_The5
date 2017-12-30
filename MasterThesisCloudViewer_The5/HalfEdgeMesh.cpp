#pragma once
#include "halfEdgeMesh.h"

#define PI 3.14159265359

/**************************************************
*	Source: http://462cmu.github.io/asst2_meshedit/
**************************************************/

	bool Halfedge::isBoundary(void)
		// returns true if and only if this halfedge is on the boundary
	{
		return face()->isBoundary();
	}

	bool Edge::isBoundary(void)
	{
		return halfedge()->face()->isBoundary();
	}

	glm::vec3 Face::normal(void) const
	{
		glm::vec3 N(0., 0., 0.);

		HalfedgeCIter h = halfedge();
		do
		{
			glm::vec3 pi = h->vertex()->position;
			glm::vec3 pj = h->next()->vertex()->position;

			N += glm::cross(pi, pj);

			h = h->next();
		} while (h != halfedge());

		return glm::normalize(N);
	}

	void HalfedgeMesh::build(const vector< vector<Index> >& polygons,
		const vector<glm::vec3>& vertexPositions)
		// This method initializes the halfedge data structure from a raw list of polygons,
		// where each input polygon is specified as a list of vertex indices.  The input
		// must describe a manifold, oriented surface, where the orientation of a polygon
		// is determined by the order of vertices in the list.  Polygons must have at least
		// three vertices.  Note that there are no special conditions on the vertex indices,
		// i.e., they do not have to start at 0 or 1, nor does the collection of indices have
		// to be contiguous.  Overall, this initializer is designed to be robust but perhaps
		// not incredibly fast (though of course this does not affect the performance of the
		// resulting data structure).  One could also implement faster initializers that
		// handle important special cases (e.g., all triangles, or data that is known to be
		// manifold).
		//
		// Since there are no strong conditions on the indices of polygons, we assume that
		// the list of vertex positions is given in lexicographic order (i.e., that the
		// lowest index appearing in any polygon corresponds to the first entry of the list
		// of positions and so on).
	{
		// define some types, to improve readability
		typedef vector<Index> IndexList;
		typedef IndexList::const_iterator IndexListCIter;
		typedef vector<IndexList> PolygonList;
		typedef PolygonList::const_iterator PolygonListCIter;
		typedef pair<Index, Index> IndexPair; // ordered pair of vertex indices, corresponding to an edge of an oriented polygon

											  // Clear any existing elements.
		halfedges.clear();
		vertices.clear();
		edges.clear();
		faces.clear();
		boundaries.clear();

		// Since the vertices in our halfedge mesh are stored in a linked list,
		// we will temporarily need to keep track of the correspondence between
		// indices of vertices in our input and pointers to vertices in the new
		// mesh (which otherwise can't be accessed by index).  Note that since
		// we're using a general-purpose map (rather than, say, a vector), we can
		// be a bit more flexible about the indexing scheme: input vertex indices
		// aren't required to be 0-based or 1-based; in fact, the set of indices
		// doesn't even have to be contiguous.  Taking advantage of this fact makes
		// our conversion a bit more robust to different types of input, including
		// data that comes from a subset of a full mesh.
		map<Index, VertexIter> indexToVertex; // maps a vertex index to the corresponding vertex

											  // Also store the vertex degree, i.e., the number of polygons that use each
											  // vertex; this information will be used to check that the mesh is manifold.
		map<VertexIter, Size> vertexDegree;

		// First, we do some basic sanity checks on the input.
		for (PolygonListCIter p = polygons.begin(); p != polygons.end(); p++)
		{
			if (p->size() < 3)
			{
				// Refuse to build the mesh if any of the polygons have fewer than three vertices.
				// (Note that if we omit this check the code will still construct something fairly
				// meaningful for 1- and 2-point polygons, but enforcing this stricter requirement
				// on the input will help simplify code further downstream, since it can be certain
				// it doesn't have to check for these rather degenerate cases.)
				cerr << "Error converting polygons to halfedge mesh: each polygon must have at least three vertices." << endl;
				exit(1);
			}

			// We want to count the number of distinct vertex indices in this
			// polygon, to make sure it's the same as the number of vertices
			// in the polygon---if they disagree, then the polygon is not valid
			// (or at least, for simplicity we don't handle polygons of this type!).
			set<Index> polygonIndices;

			// loop over polygon vertices
			for (IndexListCIter i = p->begin(); i != p->end(); i++)
			{
				polygonIndices.insert(*i);

				// allocate one vertex for each new index we encounter
				if (indexToVertex.find(*i) == indexToVertex.end())
				{
					VertexIter v = newVertex();
					v->halfedge() = halfedges.end(); // this vertex doesn't yet point to any halfedge
					indexToVertex[*i] = v;
					vertexDegree[v] = 1; // we've now seen this vertex only once
				}
				else
				{
					// keep track of the number of times we've seen this vertex
					vertexDegree[indexToVertex[*i]]++;
				}

			} // end loop over polygon vertices

			  // check that all vertices of the current polygon are distinct
			Size degree = p->size(); // number of vertices in this polygon
			if (polygonIndices.size() < degree)
			{
				cerr << "Error converting polygons to halfedge mesh: one of the input polygons does not have distinct vertices!" << endl;
				cerr << "(vertex indices:";
				for (IndexListCIter i = p->begin(); i != p->end(); i++)
				{
					cerr << " " << *i;
				}
				cerr << ")" << endl;
				exit(1);
			} // end check that polygon vertices are distinct

		} // end basic sanity checks on input

		  // The number of vertices in the mesh is the
		  // number of unique indices seen in the input.
		Size nVertices = indexToVertex.size();

		// The number of faces is just the number of polygons in the input.
		Size nFaces = polygons.size();
		faces.resize(nFaces); // allocate storage for faces in our new mesh

							  // We will store a map from ordered pairs of vertex indices to
							  // the corresponding halfedge object in our new (halfedge) mesh;
							  // this map gets constructed during the next loop over polygons.
		map<IndexPair, HalfedgeIter> pairToHalfedge;

		// Next, we actually build the halfedge connectivity by again looping over polygons
		PolygonListCIter p;
		FaceIter f;
		for (p = polygons.begin(), f = faces.begin();
			p != polygons.end();
			p++, f++)
		{
			vector<HalfedgeIter> faceHalfedges; // cyclically ordered list of the half edges of this face
			Size degree = p->size(); // number of vertices in this polygon

									 // loop over the halfedges of this face (equivalently, the ordered pairs of consecutive vertices)
			for (Index i = 0; i < degree; i++)
			{
				Index a = (*p)[i]; // current index
				Index b = (*p)[(i + 1) % degree]; // next index, in cyclic order
				IndexPair ab(a, b);
				HalfedgeIter hab;

				// check if this halfedge already exists; if so, we have a problem!
				if (pairToHalfedge.find(ab) != pairToHalfedge.end())
				{
					cerr << "Error converting polygons to halfedge mesh: found multiple oriented edges with indices (" << a << ", " << b << ")." << endl;
					cerr << "This means that either (i) more than two faces contain this edge (hence the surface is nonmanifold), or" << endl;
					cerr << "(ii) there are exactly two faces containing this edge, but they have the same orientation (hence the surface is" << endl;
					cerr << "not consistently oriented." << endl;
					exit(1);
				}
				else // otherwise, the halfedge hasn't been allocated yet
				{
					// so, we point this vertex pair to a new halfedge
					hab = newHalfedge();
					pairToHalfedge[ab] = hab;

					// link the new halfedge to its face
					hab->face() = f;
					hab->face()->halfedge() = hab;

					// also link it to its starting vertex
					hab->vertex() = indexToVertex[a];
					hab->vertex()->halfedge() = hab;

					// keep a list of halfedges in this face, so that we can later
					// link them together in a loop (via their "next" pointers)
					faceHalfedges.push_back(hab);
				}

				// Also, check if the twin of this halfedge has already been constructed (during
				// construction of a different face).  If so, link the twins together and allocate
				// their shared halfedge.  By the end of this pass over polygons, the only halfedges
				// that will not have a twin will hence be those that sit along the domain boundary.
				IndexPair ba(b, a);
				map<IndexPair, HalfedgeIter>::iterator iba = pairToHalfedge.find(ba);
				if (iba != pairToHalfedge.end())
				{
					HalfedgeIter hba = iba->second;

					// link the twins
					hab->twin() = hba;
					hba->twin() = hab;

					// allocate and link their edge
					EdgeIter e = newEdge();
					hab->edge() = e;
					hba->edge() = e;
					e->halfedge() = hab;
				}
				else // If we didn't find a twin...
				{
					// ...mark this halfedge as being twinless by pointing
					// it to the end of the list of halfedges. If it remains
					// twinless by the end of the current loop over polygons,
					// it will be linked to a boundary face in the next pass.
					hab->twin() = halfedges.end();
				}

			} // end loop over the current polygon's halfedges

			  // Now that all the halfedges of this face have been allocated,
			  // we can link them together via their "next" pointers.
			for (Index i = 0; i < degree; i++)
			{
				Index j = (i + 1) % degree; // index of the next halfedge, in cyclic order
				faceHalfedges[i]->next() = faceHalfedges[j];
			}

		} // done building basic halfedge connectivity

		  // For each vertex on the boundary, advance its halfedge pointer to one that is also on the boundary.
		for (VertexIter v = verticesBegin(); v != verticesEnd(); v++)
		{
			// loop over halfedges around vertex
			HalfedgeIter h = v->halfedge();
			do
			{
				if (h->twin() == halfedges.end())
				{
					v->halfedge() = h;
					break;
				}

				h = h->twin()->next();
			} while (h != v->halfedge()); // end loop over halfedges around vertex

		} // done advancing halfedge pointers for boundary vertices

		  // Next we construct new faces for each boundary component.
		for (HalfedgeIter h = halfedgesBegin(); h != halfedgesEnd(); h++) // loop over all halfedges
		{
			// Any halfedge that does not yet have a twin is on the boundary of the domain.
			// If we follow the boundary around long enough we will of course eventually make a
			// closed loop; we can represent this boundary loop by a new face. To make clear the
			// distinction between faces and boundary loops, the boundary face will (i) have a flag
			// indicating that it is a boundary loop, and (ii) be stored in a list of boundaries,
			// rather than the usual list of faces.  The reason we need the both the flag *and* the
			// separate list is that faces are often accessed in two fundamentally different ways:
			// either by (i) local traversal of the neighborhood of some mesh element using the
			// halfedge structure, or (ii) global traversal of all faces (or boundary loops).
			if (h->twin() == halfedges.end())
			{
				FaceIter b = newBoundary();
				vector<HalfedgeIter> boundaryHalfedges; // keep a list of halfedges along the boundary, so we can link them together

														// We now need to walk around the boundary, creating new
														// halfedges and edges along the boundary loop as we go.
				HalfedgeIter i = h;
				do
				{
					// create a twin, which becomes a halfedge of the boundary loop
					HalfedgeIter t = newHalfedge();
					boundaryHalfedges.push_back(t); // keep a list of all boundary halfedges, in cyclic order
					i->twin() = t;
					t->twin() = i;
					t->face() = b;
					t->vertex() = i->next()->vertex();

					// create the shared edge
					EdgeIter e = newEdge();
					e->halfedge() = i;
					i->edge() = e;
					t->edge() = e;

					// Advance i to the next halfedge along the current boundary loop
					// by walking around its target vertex and stopping as soon as we
					// find a halfedge that does not yet have a twin defined.
					i = i->next();
					while (i != h && // we're done if we end up back at the beginning of the loop
						i->twin() != halfedges.end()) // otherwise, we're looking for the next twinless halfedge along the loop
					{
						i = i->twin();
						i = i->next();
					}
				} while (i != h);

				// The only pointers that still need to be set are the "next" pointers of the twins;
				// these we can set from the list of boundary halfedges, but we must use the opposite
				// order from the order in the list, since the orientation of the boundary loop is
				// opposite the orientation of the halfedges "inside" the domain boundary.
				Size degree = boundaryHalfedges.size();
				for (Index p = 0; p < degree; p++)
				{
					Index q = (p - 1 + degree) % degree;
					boundaryHalfedges[p]->next() = boundaryHalfedges[q];
				}

			} // end construction of one of the boundary loops

			  // Note that even though we are looping over all halfedges, we will still construct
			  // the appropriate number of boundary loops (and not, say, one loop per boundary
			  // halfedge).  The reason is that as we continue to iterate through halfedges, we
			  // check whether their twin has been assigned, and since new twins may have been
			  // assigned earlier in this loop, we will end up skipping many subsequent halfedges.

		} // done adding "virtual" faces corresponding to boundary loops

		  // To make later traversal of the mesh easier, we will now advance the halfedge
		  // associated with each vertex such that it refers to the *first* non-boundary
		  // halfedge, rather than the last one.
		for (VertexIter v = verticesBegin(); v != verticesEnd(); v++)
		{
			v->halfedge() = v->halfedge()->twin()->next();
		}

		// Finally, we check that all vertices are manifold.
		for (VertexIter v = vertices.begin(); v != vertices.end(); v++)
		{
			// First check that this vertex is not a "floating" vertex;
			// if it is then we do not have a valid 2-manifold surface.
			if (v->halfedge() == halfedges.end())
			{
				cerr << "Error converting polygons to halfedge mesh: some vertices are not referenced by any polygon." << endl;
				exit(1);
			}

			// Next, check that the number of halfedges emanating from this vertex in our half
			// edge data structure equals the number of polygons containing this vertex, which
			// we counted during our first pass over the mesh.  If not, then our vertex is not
			// a "fan" of polygons, but instead has some other (nonmanifold) structure.
			Size count = 0;
			HalfedgeIter h = v->halfedge();
			do
			{
				if (!h->face()->isBoundary())
				{
					count++;
				}
				h = h->twin()->next();
			} while (h != v->halfedge());

			if (count != vertexDegree[v])
			{
				cerr << "Error converting polygons to halfedge mesh: at least one of the vertices is nonmanifold." << endl;
				exit(1);
			}
		} // end loop over vertices

		  // Now that we have the connectivity, we copy the list of vertex
		  // positions into member variables of the individual vertices.
		if (vertexPositions.size() != vertices.size())
		{
			cerr << "Error converting polygons to halfedge mesh: number of vertex positions is different from the number of distinct vertices!" << endl;
			cerr << "(number of positions in input: " << vertexPositions.size() << ")" << endl;
			cerr << "(  number of vertices in mesh: " << vertices.size() << ")" << endl;
			exit(1);
		}
		// Since an STL map internally sorts its keys, we can iterate over the map from vertex indices to
		// vertex iterators to visit our (input) vertices in lexicographic order
		int i = 0;
		for (map<Index, VertexIter>::const_iterator e = indexToVertex.begin(); e != indexToVertex.end(); e++)
		{
			// grab a pointer to the vertex associated with the current key (i.e., the current index)
			VertexIter v = e->second;

			// set the position of this vertex to the corresponding position in the input
			v->position = vertexPositions[i];
			i++;
		}

	} // end HalfedgeMesh::build()

	EdgeIter HalfedgeMesh::flipEdge(EdgeIter e0)
	{

		/*
		Source: https://books.google.de/books?id=0lb4_pLIyP8C&pg=PA203&lpg=PA203&dq=flip+edge+manifold&source=bl&ots=TFyUGafG8A&sig=6a0-o-UcwzIGyV5UPCSsh3YGg6M&hl=en&sa=X&ved=0ahUKEwiAjZeviLTXAhXPKVAKHXSJD9sQ6AEIVTAK#v=onepage&q=flip%20edge%20manifold&f=false	
		*/
		// TODO This method should flip the given edge and return an iterator to the flipped edge.
		if (e0->isBoundary()) {
			return e0;
		}

		HalfedgeIter h0 = e0->halfedge();
		HalfedgeIter h1 = h0->next();
		HalfedgeIter h2 = h1->next();

		HalfedgeIter h3 = h0->twin();
		HalfedgeIter h4 = h3->next();
		HalfedgeIter h5 = h4->next();


		//setNeighbors(HalfedgeIter next, HalfedgeIter twin, VertexIter vertex, EdgeIter edge, FaceIter face)
		FaceIter f1 = h0->face();
		FaceIter f2 = h3->face();

		if (f1->isBoundary() || f2->isBoundary()) {
			return e0;
		}
		/**************************
		Vertex valence > 3
		***************************/
		if (h0->vertex()->degree() <= 3|| h3->vertex()->degree() <= 3) {
			return e0;
		}
		/**************************
		We dont want to have Triangles which have angle over 90°
		    o
		   /|\
		  / | \
		 /  |  \
		/   |   \
	   o---------o

	   If we flip the edge one of the resulting Triangles will be deformed
		***************************/

		glm::vec3 pos0 = h1->vertex()->position;
		glm::vec3 pos1 = h2->vertex()->position;
		glm::vec3 pos2 = h5->vertex()->position;

		float a0 = glm::length(pos0 - pos1);
		float b0 = glm::length(pos0 - pos2);
		float c0 = glm::length(pos1 - pos2);

		float alpha0 = glm::acos( (c0*c0 + b0*b0 - a0*a0) / ( 2 * c0 * b0));
		float beta0 = glm::acos((a0*a0 + c0*c0 - b0*b0) / (2 * a0 * c0));
		float gamma0 = glm::acos((a0*a0 + b0*b0 - c0*c0) / (2 * a0 * b0));

		glm::vec3 pos3 = h2->vertex()->position;
		glm::vec3 pos4 = h4->vertex()->position;
		glm::vec3 pos5 = h5->vertex()->position;

		float a1 = glm::length(pos3 - pos4);
		float b1 = glm::length(pos3 - pos5);
		float c1 = glm::length(pos4 - pos5);

		float alpha1 = glm::acos((c1*c1 + b1*b1 - a1*a1) / (2 * c1 * b1));
		float beta1 = glm::acos((a1*a1 + c1*c1 - b1*b1) / (2 * a1 * c1));
		float gamma1 = glm::acos((a1*a1 + b1*b1 - c1*c1) / (2 * a1 * b1));

		float maxAngle = PI * 0.66f;
		if (alpha0 > maxAngle || beta0 > maxAngle || gamma0 > maxAngle || alpha1 > maxAngle || beta1 > maxAngle || gamma1 > maxAngle) {
			//std::cout << alpha0 << " " << beta0 << " " << gamma0 << " = " << alpha0 + beta0 + gamma0 << std::endl;
			//std::cout << alpha1 << " " << beta1 << " " << gamma1 << " = " << alpha1 + beta1 + gamma1 << std::endl;
			return e0;
		}

		

		/**************************
		**************************/
		VertexIter v0 = h1->vertex();
		VertexIter v1 = h2->vertex();
		VertexIter v2 = h0->vertex();
		VertexIter v3 = h5->vertex();

		////New Triangle 1
		h0->setNeighbors(h5, h3, v1, e0, f1);
		h5->setNeighbors(h1, h5->twin(), h5->vertex(), h5->edge(), f1);
		h1->setNeighbors(h0, h1->twin(), h1->vertex(), h1->edge(), f1);
		

		////New Triangle 2
		h3->setNeighbors(h2, h0, v3, e0, f2);
		h2->setNeighbors(h4, h2->twin(), h2->vertex(), h2->edge(), f2);
		h4->setNeighbors(h3, h4->twin(), h4->vertex(), h4->edge(), f2);

		////Make sure all other points are correct
		f1->halfedge() = h0;
		f2->halfedge() = h3;

		v0->halfedge() = h1;
		v1->halfedge() = h2;
		v2->halfedge() = h4;
		v3->halfedge() = h5;
		
		e0->halfedge() = h0;

		//VertexIter v1 = h2->vertex();
		//VertexIter v2 = h5->vertex();

		////New Triangle 1
		//h0->setNeighbors(h5, h3, v1, e0, f1);
		//h1->setNeighbors(h0, h1->twin(), h1->vertex(), h1->edge(), f1);
		//h5->setNeighbors(h1, h5->twin(), h5->vertex(), h5->edge(), f1);

		////New Triangle 2
		//h3->setNeighbors(h2, h0, v2, e0, f2);
		//h2->setNeighbors(h4, h2->twin(), h2->vertex(), h2->edge(), f2);
		//h4->setNeighbors(h3, h4->twin(), h4->vertex(), h4->edge(), f2);

		////Make sure all other points are correct
		//f1->halfedge() = h0;
		//f2->halfedge() = h3;

		//v1->halfedge() = h0;
		//v2->halfedge() = h3;

		//e0->halfedge() = h0;

		return e0;
	}

	VertexIter HalfedgeMesh::splitEdge(EdgeIter e)
	{

		if (e->isBoundary()) {
			return e->halfedge()->vertex();
		}

		glm::vec3 startVertex = e->halfedge()->vertex()->position;
		glm::vec3 endVertex = e->halfedge()->twin()->vertex()->position;

		//VertexOld
		VertexIter v0_old = e->halfedge()->next()->vertex();
		VertexIter v1_old = e->halfedge()->next()->next()->vertex();
		VertexIter v2_old = e->halfedge()->vertex();
		VertexIter v3_old = e->halfedge()->twin()->next()->next()->vertex();

		//Face
		FaceIter f0 = e->halfedge()->face();
		FaceIter f1 = e->halfedge()->twin()->face();
		/***********************************************
		Do the check before creating new triangles!!
		************************************************/
		if (f0->isBoundary() || f1->isBoundary()) {
			return  e->halfedge()->vertex();
		}
		/***********************************************
		************************************************/
		FaceIter f2 = this->newFace();
		FaceIter f3 = this->newFace();

		

		//Create necessary datas
		//Vertex
		VertexIter v0 = this->newVertex();
		v0->position = 0.5f * (startVertex + endVertex);

		//Edge
		EdgeIter e0 = e;
		EdgeIter e1 = this->newEdge();
		EdgeIter e2 = this->newEdge();
		EdgeIter e3 = this->newEdge();

		//HalfEdge
		HalfedgeIter h0 = e->halfedge();
		HalfedgeIter h1 = e->halfedge()->next();
		HalfedgeIter h2 = this->newHalfedge();

		HalfedgeIter h3 = e->halfedge()->twin();
		HalfedgeIter h4 = this->newHalfedge();
		HalfedgeIter h5 = e->halfedge()->next()->next();

		HalfedgeIter h6 = this->newHalfedge();
		HalfedgeIter h7 = e->halfedge()->twin()->next();
		HalfedgeIter h8 = this->newHalfedge();

		HalfedgeIter h9 = this->newHalfedge();
		HalfedgeIter h10 = this->newHalfedge();
		HalfedgeIter h11 = e->halfedge()->twin()->next()->next();

		//setNeighbors(HalfedgeIter next, HalfedgeIter twin, VertexIter vertex, EdgeIter edge, FaceIter face)
		h0->setNeighbors(h1, h9, v0, e0, f0);
		h1->setNeighbors(h2, h1->twin(), v0_old, h1->edge(), f0);
		h2->setNeighbors(h0, h4, v1_old, e1, f0);

		h3->setNeighbors(h4, h6, v2_old, e2, f1);
		h4->setNeighbors(h5, h2, v0, e1, f1);
		h5->setNeighbors(h3, h5->twin(), v1_old, h5->edge(), f1);

		h6->setNeighbors(h7, h3, v0, e2, f2);
		h7->setNeighbors(h8, h7->twin(), v2_old, h7->edge(), f2);
		h8->setNeighbors(h6, h10, v3_old, e3, f2);
		
		h9->setNeighbors(h10, h0, v0_old, e0, f3);
		h10->setNeighbors(h11, h8, v0, e3, f3);
		h11->setNeighbors(h9, h11->twin(), v3_old, h11->edge(), f3);

		//Set Vertices
		v0->halfedge() = h0;
		v0_old->halfedge() = h1;
		v1_old->halfedge() = h5;
		v2_old->halfedge() = h7;
		v3_old->halfedge() = h11;

		//Set Faces
		f0->halfedge() = h0;
		f1->halfedge() = h3;
		f2->halfedge() = h6;
		f3->halfedge() = h9;

		//Set Edges
		e0->halfedge() = h0;
		e1->halfedge() = h4;
		e2->halfedge() = h6;
		e3->halfedge() = h8;

		/*e0->isNew = true;
		e1->isNew = true;
		e2->isNew = true;
		e3->isNew = true;*/

		return v0;
	}

	VertexIter HalfedgeMesh::collapseEdge(EdgeIter e)
	{
		if (e->isBoundary()) {
			return e->halfedge()->vertex();
		}
		//****************
		//Create helper Variables
		//****************
		glm::vec3 startVertex = e->halfedge()->vertex()->position;
		glm::vec3 endVertex = e->halfedge()->twin()->vertex()->position;
		glm::vec3 newVertexPos = 0.5f * (startVertex + endVertex);

		VertexIter v0 = e->halfedge()->vertex();
		VertexIter v1 = e->halfedge()->twin()->vertex();

		v0->position = newVertexPos;

		HalfedgeIter h0 = e->halfedge();
		HalfedgeIter h1 = h0->next();
		HalfedgeIter h2 = h1->next();

		HalfedgeIter h3 = e->halfedge()->twin();
		HalfedgeIter h4 = h3->next();
		HalfedgeIter h5 = h4->next();

		FaceIter f0 = h0->face();
		FaceIter f1 = h3->face();

		if (f0->isBoundary() || f1->isBoundary()) {
			return e->halfedge()->vertex();
		}

		//Shared vertices of both fans (the ones not on the given edge)
		VertexIter sharedUp = h0->next()->next()->vertex();
		sharedUp->halfedge()= h0->next()->twin();

		VertexIter sharedDown = h3->next()->next()->vertex();
		sharedDown->halfedge() = h3->next()->twin();

		//****************
		//Go through first fan
		//****************

		HalfedgeIter upLeft = h0->next()->twin();
		HalfedgeIter upRight = h0->next()->next()->twin();

		HalfedgeIter downLeft = h3->next()->twin();
		HalfedgeIter downRight = h3->next()->next()->twin();

		//Every half-Edge pointing to v1 has to be redirected to v0
		HalfedgeIter h1Fan = h3;
		do {
			HalfedgeIter h1_twin = h1Fan->twin(); // get the vertex of the current halfedge
			if (h1_twin->vertex() == v1) {
				h1_twin->vertex() = v0;
			}

			h1Fan = h1_twin->next(); // move to the next outgoing halfedge of the vertex.
			if (h1Fan->vertex() == v1) {
				h1Fan->vertex() = v0;
			}

		} while (h1Fan != h3);


		//Set Half-Edge
		upLeft->setNeighbors(upLeft->next(), upRight, upLeft->vertex(), upLeft->edge(), upLeft->face());
		//Set Edge
		upLeft->edge()->halfedge() = upLeft;
		//Set Half-Edge
		upRight->setNeighbors(upRight->next(), upLeft, upRight->vertex(), upRight->edge(), upRight->face());
		//Set Half-Edge
		downLeft->setNeighbors(downLeft->next(), downRight, downLeft->vertex(), downLeft->edge(), downLeft->face());
		//Set Edge
		downLeft->edge()->halfedge() = downLeft;
		//Set Half-Edge
		downRight->setNeighbors(downRight->next(), downLeft, downRight->vertex(), downRight->edge(), downRight->face());

		v0->halfedge() = h0->next()->next()->twin();
		v1->halfedge() = h3->next()->next()->twin();

		//Delete rest
		this->deleteHalfedge(h0->next()->next());
		this->deleteHalfedge(h0->next());
		this->deleteHalfedge(h0);

		this->deleteHalfedge(h3->next()->next());
		this->deleteHalfedge(h3->next());
		this->deleteHalfedge(h3);

		this->deleteEdge(e);

		this->deleteFace(f0);
		this->deleteFace(f1);

		this->deleteVertex(v1);

		return v0;
	}

	const HalfedgeMesh& HalfedgeMesh :: operator=(const HalfedgeMesh& mesh)
		// The assignment operator does a "deep" copy of the halfedge mesh data structure; in
		// other words, it makes new instances of each mesh element, and ensures that pointers
		// in the copy point to the newly allocated elements rather than elements in the original
		// mesh.  This behavior is especially important for making assignments, since the mesh
		// on the right-hand side of an assignment may be temporary (hence any pointers to elements
		// in this mesh will become invalid as soon as it is released.)
	{
		// Clear any existing elements.
		halfedges.clear();
		vertices.clear();
		edges.clear();
		faces.clear();
		boundaries.clear();

		// These maps will be used to identify elements of the old mesh
		// with elements of the new mesh.  (Note that we can use a single
		// map for both interior and boundary faces, because the map
		// doesn't care which list of faces these iterators come from.)
		map< HalfedgeCIter, HalfedgeIter > halfedgeOldToNew;
		map<   VertexCIter, VertexIter >   vertexOldToNew;
		map<     EdgeCIter, EdgeIter >     edgeOldToNew;
		map<     FaceCIter, FaceIter >     faceOldToNew;

		// Copy geometry from the original mesh and create a map from
		// pointers in the original mesh to those in the new mesh.
		for (HalfedgeCIter h = mesh.halfedgesBegin(); h != mesh.halfedgesEnd(); h++) halfedgeOldToNew[h] = halfedges.insert(halfedges.end(), *h);
		for (VertexCIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)   vertexOldToNew[v] = vertices.insert(vertices.end(), *v);
		for (EdgeCIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)     edgeOldToNew[e] = edges.insert(edges.end(), *e);
		for (FaceCIter f = mesh.facesBegin(); f != mesh.facesEnd(); f++)     faceOldToNew[f] = faces.insert(faces.end(), *f);
		for (FaceCIter b = mesh.boundariesBegin(); b != mesh.boundariesEnd(); b++)     faceOldToNew[b] = boundaries.insert(boundaries.end(), *b);

		// "Search and replace" old pointers with new ones.
		for (HalfedgeIter he = halfedgesBegin(); he != halfedgesEnd(); he++)
		{
			he->next() = halfedgeOldToNew[he->next()];
			he->twin() = halfedgeOldToNew[he->twin()];
			he->vertex() = vertexOldToNew[he->vertex()];
			he->edge() = edgeOldToNew[he->edge()];
			he->face() = faceOldToNew[he->face()];
		}
		for (VertexIter v = verticesBegin(); v != verticesEnd(); v++) v->halfedge() = halfedgeOldToNew[v->halfedge()];
		for (EdgeIter e = edgesBegin(); e != edgesEnd(); e++) e->halfedge() = halfedgeOldToNew[e->halfedge()];
		for (FaceIter f = facesBegin(); f != facesEnd(); f++) f->halfedge() = halfedgeOldToNew[f->halfedge()];
		for (FaceIter b = boundariesBegin(); b != boundariesEnd(); b++) b->halfedge() = halfedgeOldToNew[b->halfedge()];

		// Return a reference to the new mesh.
		return *this;
	}

	HalfedgeMesh::HalfedgeMesh(const HalfedgeMesh& mesh)
	{
		*this = mesh;
	}


/***************************************************************
SELFMADE
**************************************************************/


//#include "HalfEdgeMesh.h"
//#include <iostream>
//#include <set>
//#include <algorithm>
//HalfEdgeMesh::HalfEdgeMesh()
//{
//}
//
//HalfEdgeMesh::HalfEdgeMesh(std::vector<glm::vec3>_vertices, std::vector<unsigned int> _indices)
//{
//	this->indices = _indices;
//	this->vertices = _vertices;
//	this->vertexToHalfEdge.resize(this->vertices.size());
//
//	int nrTriangles = this->indices.size() / 3;
//	
//	int heIndex = 0;
//
//	/*
//	Test 1
//	*/
//	//std::vector<edge> edge_set = createEdgeList();
//	//for(int i = 0; i < edge_set.size(); i++){
//
//	//	halfEdge he_1;
//	//	he_1.vertexIndex = this->indices[edge_set[i].firstVertexIndex];
//	//	he_1.halfEdgeIndex = heIndex;
//	//	vertexToHalfEdge[edge_set[i].firstVertexIndex] = heIndex;
//	//	heIndex++;
//
//	//	if (edge_set[i].numHalfEdges > 1) {
//	//		halfEdge he_2;
//	//		he_2.vertexIndex = this->indices[edge_set[i].secondVertexIndex];
//	//		he_2.halfEdgeIndex = heIndex;
//	//		vertexToHalfEdge[edge_set[i].secondVertexIndex] = heIndex;
//	//		heIndex++;
//
//	//		he_1.oppositeHalfEdgeIndex = he_2.halfEdgeIndex;
//	//		he_2.oppositeHalfEdgeIndex = he_1.halfEdgeIndex;
//
//	//		halfEdges.push_back(he_2);
//	//	}
//	//}
//
//
//	/*
//	Test 2
//	*/
//	for(int i = 0; i < nrTriangles; i++){
//
//		halfEdge he_1;
//		he_1.halfEdgeIndex = heIndex;
//		he_1.vertexIndex = this->indices[3 * i + 0];
//		he_1.faceIndex = i;
//		he_1.nextHalfEdgeIndex = heIndex + 1;
//		halfEdges.push_back(he_1);
//		vertexToHalfEdge[this->indices[3 * i + 0]] = heIndex;
//		heIndex++;
//
//		halfEdge he_2;
//		he_2.halfEdgeIndex = heIndex;
//		he_2.vertexIndex = this->indices[3 * i + 1];
//		he_2.faceIndex = i;
//		he_2.nextHalfEdgeIndex = heIndex + 1;
//		halfEdges.push_back(he_2);
//		vertexToHalfEdge[this->indices[3 * i + 1]] = heIndex;
//		heIndex++;
//
//		halfEdge he_3;
//		he_3.halfEdgeIndex = heIndex;
//		he_3.vertexIndex = this->indices[3 * i + 2];
//		he_3.faceIndex = i;
//		he_3.nextHalfEdgeIndex = he_1.halfEdgeIndex;
//		halfEdges.push_back(he_3);
//		vertexToHalfEdge[this->indices[3 * i + 2]] = heIndex;
//		heIndex++;
//
//		for (int j = 0; j < halfEdges.size(); j++) {
//			if (j == heIndex - 2 || j == heIndex - 1 || j == heIndex) {
//			}
//			else{
//				if (halfEdges[j].vertexIndex == he_1.vertexIndex) {
//					halfEdges[he_3.halfEdgeIndex].oppositeHalfEdgeIndex = j;
//					halfEdges[j].oppositeHalfEdgeIndex = he_3.halfEdgeIndex;
//				}
//
//				if (halfEdges[j].vertexIndex == he_2.vertexIndex) {
//					halfEdges[he_1.halfEdgeIndex].oppositeHalfEdgeIndex = j;
//					halfEdges[j].oppositeHalfEdgeIndex = he_1.halfEdgeIndex;
//				}
//
//				if (halfEdges[j].vertexIndex == he_3.vertexIndex) {
//					halfEdges[he_2.halfEdgeIndex].oppositeHalfEdgeIndex = j;
//					halfEdges[j].oppositeHalfEdgeIndex = he_2.halfEdgeIndex;
//				}
//
//			}
//		}
//
//	}
//}
//
//
//HalfEdgeMesh::~HalfEdgeMesh()
//{
//
//}
//
////std::vector<edge> HalfEdgeMesh::createEdgeList()
////{
////	int num_triangles = this->indices.size() / 3;
////
////	std::vector<edge> edge_set;
////
////	for (int t = 0; t < num_triangles; ++t){
////		edge e1(std::min(this->indices[3 * t + 0], this->indices[3 * t + 1]), std::max(this->indices[3 * t + 0], this->indices[3 * t + 1]));
////		edge e2(std::min(this->indices[3 * t + 0], this->indices[3 * t + 2]), std::max(this->indices[3 * t + 0], this->indices[3 * t + 2]));
////		edge e3(std::min(this->indices[3 * t + 2], this->indices[3 * t + 1]), std::max(this->indices[3 * t + 2], this->indices[3 * t + 1]));
////
////		bool insert_e1 = true;
////		bool insert_e2 = true;
////		bool insert_e3 = true;
////
////		for (int i = 0; i < edge_set.size(); i++) {
////			if (e1 == edge_set[i]) {
////				insert_e1 = false;
////				edge_set[i].secondTriangle = t;
////				edge_set[i].numHalfEdges++;
////			}
////
////			if (e2 == edge_set[i]) {
////				insert_e2 = false;
////				edge_set[i].secondTriangle = t;
////				edge_set[i].numHalfEdges++;
////			}
////
////			if (e3 == edge_set[i]) {
////				insert_e3 = false;
////				edge_set[i].secondTriangle = t;
////				edge_set[i].numHalfEdges++;
////			}
////		}
////
////		if (insert_e1) {
////			e1.firstTriangle = t;
////			edge_set.push_back(e1);
////		}
////		if (insert_e2) {
////			e2.firstTriangle = t;
////			edge_set.push_back(e2);
////		}
////		if (insert_e3) {
////			e3.firstTriangle = t;
////			edge_set.push_back(e3);
////		}
////
////	}
////
////	for (int i = 0; i < edge_set.size(); i++) {
////		std::cout << "Edge " << i << ": " << edge_set[i].firstVertexIndex << " " << edge_set[i].secondVertexIndex << " mit " << edge_set[i].numHalfEdges << std::endl;
////	}
////
////	return edge_set;
////}
//
//
//
//
//
//
//
//halfEdge HalfEdgeMesh::getHalfEdge(int halfEdgeIndex)
//{
//	return halfEdges[halfEdgeIndex];
//}
//
//int HalfEdgeMesh::getNextHalfEdge(int halfEdgeIndex)
//{
//	return halfEdges[halfEdges[halfEdgeIndex].nextHalfEdgeIndex].halfEdgeIndex;
//}
//
//halfEdge HalfEdgeMesh::getNextHalfEdge(halfEdge he)
//{
//	return halfEdges[he.nextHalfEdgeIndex];
//}
//
//halfEdge HalfEdgeMesh::getOppositeHalfEdge(halfEdge he)
//{
//	return halfEdges[he.oppositeHalfEdgeIndex];
//}
//
//void HalfEdgeMesh::printTriangleFan(int vertexId)
//{
//	std::vector<unsigned int> vertexFan;
//
//	halfEdge he = halfEdges[this->vertexToHalfEdge[vertexId]];
//	int heStartIndex = he.halfEdgeIndex;
//
//	vertexFan.push_back(he.vertexIndex);
//
//	
//	do {
//
//		he = getNextHalfEdge(he);
//		if (he.vertexIndex != vertexId) {
//			vertexFan.push_back(he.vertexIndex);
//		}
//
//		std::cout << he.halfEdgeIndex << std::endl;
//
//		he = getNextHalfEdge(he);
//		if (he.vertexIndex != vertexId) {
//			vertexFan.push_back(he.vertexIndex);
//		}
//
//		std::cout << he.halfEdgeIndex << std::endl;
//
//		he = getOppositeHalfEdge(he);
//
//		std::cout << he.halfEdgeIndex << std::endl;
//
//	} while (he.halfEdgeIndex != heStartIndex);
//
//	std::cout << "Triangle fan: " << std::endl;
//	for (int i = 0; i < vertexFan.size(); i++) {
//		std::cout << vertexFan[i] << std::endl;
//	}
//}
