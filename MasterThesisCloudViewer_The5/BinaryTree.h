#pragma once
//Source: https://www.cprogramming.com/tutorial/lesson18.html
struct node
{
	float key_value;
	node *left = nullptr;
	node *right = nullptr;
};

class btree
{
public:
	btree();
	~btree();

	void insert(float key);
	node *search(float key);
	void destroy_tree();

	void printNode(node *leaf);
	void printTree();

private:
	void destroy_tree(node *leaf);
	void insert(float key, node *leaf);
	node *search(float key, node *leaf);

	node *root;
};