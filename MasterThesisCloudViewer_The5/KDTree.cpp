#include "KDTree.h"
#include "tree.hh"
#include "tree_util.hh"


struct sortByX
{
	inline bool operator() (const glm::vec3& struct1, const glm::vec3& struct2)
	{
		return (struct1.x < struct2.x);
	}
};

struct sortByY
{
	inline bool operator() (const glm::vec3& struct1, const glm::vec3& struct2)
	{
		return (struct1.y < struct2.y);
	}
};

struct sortByZ
{
	inline bool operator() (const glm::vec3& struct1, const glm::vec3& struct2)
	{
		return (struct1.z < struct2.z);
	}
};


KDTree::KDTree(std::vector<glm::vec3> vertices){
	tree<int> binaryDepthFour;
	tree<int>::iterator zero = binaryDepthFour.insert(binaryDepthFour.begin(), -0.0f);

	tree<int>::iterator first = binaryDepthFour.append_child(zero, -1.0f);
	tree<int>::iterator second = binaryDepthFour.append_child(zero, -2.0f);

	tree<int>::iterator third = binaryDepthFour.append_child(first, -3.0f);
	tree<int>::iterator fourth = binaryDepthFour.append_child(first, -4.0f);

	tree<int>::iterator fifth = binaryDepthFour.append_child(second, -5.0f);
	tree<int>::iterator sixth = binaryDepthFour.append_child(second, -6.0f);


	binaryDepthFour.append_child(third, -7.0f);
	binaryDepthFour.append_child(third, -8.0f);

	binaryDepthFour.append_child(fourth, -9.0f);
	binaryDepthFour.append_child(fourth, -10.0f);

	binaryDepthFour.append_child(fifth, -11.0f);
	binaryDepthFour.append_child(fifth, -12.0f);

	binaryDepthFour.append_child(sixth, -13.0f);
	binaryDepthFour.append_child(sixth, -14.0f);

	kptree::print_tree_bracketed(binaryDepthFour, std::cout);

	this->indices.resize(vertices.size());
	this->indicesTemp.resize(vertices.size());
}

