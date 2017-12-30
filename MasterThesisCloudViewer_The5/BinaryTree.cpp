#include "BinaryTree.h"
#include <iostream>

btree::btree()
{
	root = nullptr;
}

void btree::destroy_tree(node *leaf)
{
	if (leaf != nullptr)
	{
		destroy_tree(leaf->left);
		destroy_tree(leaf->right);
		delete leaf;
	}
}

void btree::insert(float key, node *leaf)
{

	if (leaf->left == nullptr && leaf->right == nullptr) {
		leaf->left = new node;
		leaf->left->key_value = key;
		leaf->left->left = nullptr;    //Sets the left child of the child node to null
		leaf->left->right = nullptr;   //Sets the right child of the child node to null
	}
	else if (leaf->left != nullptr && leaf->right == nullptr) {
		leaf->right = new node;
		leaf->right->key_value = key;
		leaf->right->left = nullptr;    //Sets the left child of the child node to null
		leaf->right->right = nullptr;   //Sets the right child of the child node to null
	}
	else if (leaf->left != nullptr && leaf->right != nullptr && leaf->left->left == nullptr) {
		insert(key, leaf->left);
	}
	else if (leaf->left != nullptr && leaf->right != nullptr && leaf->left->right == nullptr) {
		insert(key, leaf->left);
	}
	else {
		insert(key, leaf->right);
	}
}

node *btree::search(float key, node *leaf)
{
	if (leaf != nullptr)
	{
		if (key == leaf->key_value)
			return leaf;
		if (key<leaf->key_value)
			return search(key, leaf->left);
		else
			return search(key, leaf->right);
	}
	else return nullptr;
}

void btree::insert(float key)
{
	if (root != nullptr)
		insert(key, root);
	else
	{
		root = new node;
		root->key_value = key;
		root->left = nullptr;
		root->right = nullptr;
	}
}

node *btree::search(float key)
{
	return search(key, root);
}

void btree::destroy_tree()
{
	destroy_tree(root);
}



void btree::printNode(node * leaf)
{
	std::cout << leaf->key_value;

	if (!(leaf->left == nullptr)) {
		printNode(leaf->left);
	}
	else {
		std::cout << "N";
	}

	if (!(leaf->right == nullptr)) {
		printNode(leaf->right);
	}
	else {
		std::cout << "N";
	}
}

void btree::printTree()
{
	printNode(root);
}

