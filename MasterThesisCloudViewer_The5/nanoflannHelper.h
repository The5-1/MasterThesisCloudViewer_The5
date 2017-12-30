//#pragma once
//#include "nanoflann.h"
//
//using namespace std;
//using namespace nanoflann;
//
//// This is an exampleof a custom data set class
//template <typename T>
//struct PointCloud
//{
//	typedef T coord_t; //!< The type of each coordinate
//
//	struct Point
//	{
//		T  x, y, z;
//	};
//
//	std::vector<Point>  pts;
//}; // end of PointCloud
//
//
//   // And this is the "dataset to kd-tree" adaptor class:
//template <typename Derived>
//struct PointCloudAdaptor
//{
//	typedef typename Derived::coord_t coord_t;
//
//	const Derived &obj; //!< A const ref to the data set origin
//
//						/// The constructor that sets the data set source
//	PointCloudAdaptor(const Derived &obj_) : obj(obj_) { }
//
//	/// CRTP helper method
//	inline const Derived& derived() const { return obj; }
//
//	// Must return the number of data points
//	inline size_t kdtree_get_point_count() const { return derived().pts.size(); }
//
//	// Returns the dim'th component of the idx'th point in the class:
//	// Since this is inlined and the "dim" argument is typically an immediate value, the
//	//  "if/else's" are actually solved at compile time.
//	inline coord_t kdtree_get_pt(const size_t idx, int dim) const
//	{
//		if (dim == 0) return derived().pts[idx].x;
//		else if (dim == 1) return derived().pts[idx].y;
//		else return derived().pts[idx].z;
//	}
//
//	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
//	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
//	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
//	template <class BBOX>
//	bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
//
//}; // end of PointCloudAdaptor
//
//template <typename T>
//void generateRandomPointCloud(PointCloud<T> &point, const size_t N, const T max_range = 10)
//{
//	std::cout << "Generating " << N << " point cloud...";
//	point.pts.resize(N);
//	for (size_t i = 0; i < N; i++)
//	{
//		point.pts[i].x = max_range * (rand() % 1000) / T(1000);
//		point.pts[i].y = max_range * (rand() % 1000) / T(1000);
//		point.pts[i].z = max_range * (rand() % 1000) / T(1000);
//	}
//
//	std::cout << "done\n";
//}
//
//template <typename T>
//void generatePointCloud(PointCloud<T> &point, const vector<glm::vec3> vertices)
//{
//	int N = vertices.size();
//	std::cout << "Generating " << N << " point cloud...";
//	point.pts.resize(N);
//	for (size_t i = 0; i < N; i++)
//	{
//		point.pts[i].x = vertices[i].x;
//		point.pts[i].y = vertices[i].y;
//		point.pts[i].z = vertices[i].z;
//	}
//
//	std::cout << "done\n";
//}
//
//
//
///*****************************************************************
//Demo to see if everything works
//*****************************************************************/
//template <typename num_t>
//void kdtree_demo(const size_t N)
//{
//	PointCloud<num_t> cloud;
//
//	// Generate points:
//	generateRandomPointCloud(cloud, N);
//
//	num_t query_pt[3] = { 0.5, 0.5, 0.5 };
//
//	typedef PointCloudAdaptor<PointCloud<num_t> > PC2KD;
//	const PC2KD  pc2kd(cloud); // The adaptor
//
//							   // construct a kd-tree index:
//	typedef KDTreeSingleIndexAdaptor<
//		L2_Simple_Adaptor<num_t, PC2KD >,
//		PC2KD,
//		3 /* dim */
//	> my_kd_tree_t;
//
//	//dump_mem_usage();
//
//	my_kd_tree_t   index(3 /*dim*/, pc2kd, KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
//	index.buildIndex();
//
//	index.
//	//dump_mem_usage();
//
//	// do a knn search
//	const size_t num_results = 1;
//	size_t ret_index;
//	num_t out_dist_sqr;
//	nanoflann::KNNResultSet<num_t> resultSet(num_results);
//	resultSet.init(&ret_index, &out_dist_sqr);
//
//
//	index.findNeighbors(resultSet, &query_pt[0], nanoflann::SearchParams(10));
//
//
//	std::cout << "knnSearch(nn=" << num_results << "): \n";
//	std::cout << "ret_index=" << ret_index << " out_dist_sqr=" << out_dist_sqr << endl;
//
//}