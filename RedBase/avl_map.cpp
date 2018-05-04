#include <stdlib.h>
#include "avl_map.h"


struct _AVLMapNode {
	AVLMapNode *children[2];
	AVLMapNode *parent;
	AVLMapKey key;
	AVLMapValue value;
	int height;
};

struct _AVLMap {
	AVLMapNode *root_node;
	AVLMapCompareFunc compare_func;
	AVLMapValue param1;
	AVLMapValue param2;
	unsigned int num_nodes;
};
static AVLMapNode *avl_map_lookup_node(AVLMap *tree, AVLMapKey key);

static int avl_map_subtree_height(AVLMapNode *node)
{
	if (node == NULL) {
		return 0;
	}
	else {
		return node->height;
	}
}

AVLMap *avl_map_new(AVLMapCompareFunc compare_func, AVLMapValue param1, AVLMapValue param2)
{
	AVLMap *new_tree;

	new_tree = (AVLMap *)malloc(sizeof(AVLMap));

	if (new_tree == NULL) {
		return NULL;
	}

	new_tree->root_node = NULL;
	new_tree->compare_func = compare_func;
	new_tree->num_nodes = 0;
	new_tree->param1 = param1;
	new_tree->param2 = param2;

	return new_tree;
}

static void avl_map_free_subtree(AVLMap *tree, AVLMapNode *node)
{
	if (node == NULL) {
		return;
	}

	avl_map_free_subtree(tree, node->children[AVL_MAP_NODE_LEFT]);
	avl_map_free_subtree(tree, node->children[AVL_MAP_NODE_RIGHT]);

	free(node);
}

void avl_map_free(AVLMap *tree)
{
	/* Destroy all nodes */

	avl_map_free_subtree(tree, tree->root_node);

	/* Free back the main tree data structure */

	free(tree);
}

/* Update the "height" variable of a node, from the heights of its
* children.  This does not update the height variable of any parent
* nodes. */

static void avl_map_update_height(AVLMapNode *node)
{
	AVLMapNode *left_subtree;
	AVLMapNode *right_subtree;
	int left_height, right_height;

	left_subtree = node->children[AVL_MAP_NODE_LEFT];
	right_subtree = node->children[AVL_MAP_NODE_RIGHT];
	left_height = avl_map_subtree_height(left_subtree);
	right_height = avl_map_subtree_height(right_subtree);

	if (left_height > right_height) {
		node->height = left_height + 1;
	}
	else {
		node->height = right_height + 1;
	}
}

/* Find what side a node is relative to its parent */

static AVLMapNodeSide avl_map_node_parent_side(AVLMapNode *node)
{
	if (node->parent->children[AVL_MAP_NODE_LEFT] == node) {
		return AVL_MAP_NODE_LEFT;
	}
	else {
		return AVL_MAP_NODE_RIGHT;
	}
}

/* Replace node1 with node2 at its parent. */

static void avl_map_node_replace(AVLMap *tree, AVLMapNode *node1,
	AVLMapNode *node2)
{
	int side;

	/* Set the node's parent pointer. */

	if (node2 != NULL) {
		node2->parent = node1->parent;
	}

	/* The root node? */

	if (node1->parent == NULL) {
		tree->root_node = node2;
	}
	else {
		side = avl_map_node_parent_side(node1);
		node1->parent->children[side] = node2;

		avl_map_update_height(node1->parent);
	}
}

/* Rotate a section of the tree.  'node' is the node at the top
* of the section to be rotated.  'direction' is the direction in
* which to rotate the tree: left or right, as shown in the following
* diagram:
*
* Left rotation:              Right rotation:
*
*      B                             D
*     / \                           / \
*    A   D                         B   E
*       / \                       / \
*      C   E                     A   C

* is rotated to:              is rotated to:
*
*        D                           B
*       / \                         / \
*      B   E                       A   D
*     / \                             / \
*    A   C                           C   E
*/

static AVLMapNode *avl_map_rotate(AVLMap *tree, AVLMapNode *node,
	AVLMapNodeSide direction)
{
	AVLMapNode *new_root;

	/* The child of this node will take its place:
	for a left rotation, it is the right child, and vice versa. */

	new_root = node->children[1 - direction];

	/* Make new_root the root, update parent pointers. */

	avl_map_node_replace(tree, node, new_root);

	/* Rearrange pointers */

	node->children[1 - direction] = new_root->children[direction];
	new_root->children[direction] = node;

	/* Update parent references */

	node->parent = new_root;

	if (node->children[1 - direction] != NULL) {
		node->children[1 - direction]->parent = node;
	}

	/* Update heights of the affected nodes */

	avl_map_update_height(new_root);
	avl_map_update_height(node);

	return new_root;
}


/* Balance a particular tree node.
*
* Returns the root node of the new subtree which is replacing the
* old one. */

static AVLMapNode *avl_map_node_balance(AVLMap *tree, AVLMapNode *node)
{
	AVLMapNode *left_subtree;
	AVLMapNode *right_subtree;
	AVLMapNode *child;
	int diff;

	left_subtree = node->children[AVL_MAP_NODE_LEFT];
	right_subtree = node->children[AVL_MAP_NODE_RIGHT];

	/* Check the heights of the child trees.  If there is an unbalance
	* (difference between left and right > 2), then rotate nodes
	* around to fix it */

	diff = avl_map_subtree_height(right_subtree)
		- avl_map_subtree_height(left_subtree);

	if (diff >= 2) {

		/* Biased toward the right side too much. */

		child = right_subtree;

		if (avl_map_subtree_height(child->children[AVL_MAP_NODE_RIGHT])
			< avl_map_subtree_height(child->children[AVL_MAP_NODE_LEFT])) {

			/* If the right child is biased toward the left
			* side, it must be rotated right first (double
			* rotation) */

			avl_map_rotate(tree, right_subtree,
				AVL_MAP_NODE_RIGHT);
		}

		/* Perform a left rotation.  After this, the right child will
		* take the place of this node.  Update the node pointer. */

		node = avl_map_rotate(tree, node, AVL_MAP_NODE_LEFT);

	}
	else if (diff <= -2) {

		/* Biased toward the left side too much. */

		child = node->children[AVL_MAP_NODE_LEFT];

		if (avl_map_subtree_height(child->children[AVL_MAP_NODE_LEFT])
			< avl_map_subtree_height(child->children[AVL_MAP_NODE_RIGHT])) {

			/* If the left child is biased toward the right
			* side, it must be rotated right left (double
			* rotation) */

			avl_map_rotate(tree, left_subtree,
				AVL_MAP_NODE_LEFT);
		}

		/* Perform a right rotation.  After this, the left child will
		* take the place of this node.  Update the node pointer. */

		node = avl_map_rotate(tree, node, AVL_MAP_NODE_RIGHT);
	}

	/* Update the height of this node */

	avl_map_update_height(node);

	return node;
}

/* Walk up the tree from the given node, performing any needed rotations */

static void avl_map_balance_to_root(AVLMap *tree, AVLMapNode *node)
{
	AVLMapNode *rover;

	rover = node;

	while (rover != NULL) {

		/* Balance this node if necessary */

		rover = avl_map_node_balance(tree, rover);

		/* Go to this node's parent */

		rover = rover->parent;
	}
}

bool avl_map_insert(AVLMap *tree, AVLMapKey key, AVLMapValue value)
{
	AVLMapNode **rover;
	AVLMapNode *new_node;
	AVLMapNode *previous_node;
	int res;
	/* Walk down the tree until we reach a NULL pointer */

	rover = &tree->root_node;
	previous_node = NULL;

	while (*rover != NULL) {
		previous_node = *rover;
		res = tree->compare_func(key, (*rover)->key, tree->param1, tree->param2);
		if (res < 0) {
			rover = &((*rover)->children[AVL_MAP_NODE_LEFT]);
		}
		else if (res > 0) {
			rover = &((*rover)->children[AVL_MAP_NODE_RIGHT]);
		}
		else {
			return false; /* ²åÈëÊ§°Ü */
		}
	}

	/* Create a new node.  Use the last node visited as the parent link. */

	new_node = (AVLMapNode *)malloc(sizeof(AVLMapNode));

	if (new_node == NULL) {
		return NULL;
	}

	new_node->children[AVL_MAP_NODE_LEFT] = NULL;
	new_node->children[AVL_MAP_NODE_RIGHT] = NULL;
	new_node->parent = previous_node;
	new_node->key = key;
	new_node->value = value;
	new_node->height = 1;

	/* Insert at the NULL pointer that was reached */

	*rover = new_node;

	/* Rebalance the tree, starting from the previous node. */

	avl_map_balance_to_root(tree, previous_node);

	/* Keep track of the number of entries */

	++tree->num_nodes;

	return true;
}



/* Find the nearest node to the given node, to replace it.
* The node returned is unlinked from the tree.
* Returns NULL if the node has no children. */

static AVLMapNode *avl_map_node_get_replacement(AVLMap *tree,
	AVLMapNode *node)
{
	AVLMapNode *left_subtree;
	AVLMapNode *right_subtree;
	AVLMapNode *result;
	AVLMapNode *child;
	int left_height, right_height;
	int side;

	left_subtree = node->children[AVL_MAP_NODE_LEFT];
	right_subtree = node->children[AVL_MAP_NODE_RIGHT];

	/* No children? */

	if (left_subtree == NULL && right_subtree == NULL) {
		return NULL;
	}

	/* Pick a node from whichever subtree is taller.  This helps to
	* keep the tree balanced. */

	left_height = avl_map_subtree_height(left_subtree);
	right_height = avl_map_subtree_height(right_subtree);

	if (left_height < right_height) {
		side = AVL_MAP_NODE_RIGHT;
	}
	else {
		side = AVL_MAP_NODE_LEFT;
	}

	/* Search down the tree, back towards the center. */

	result = node->children[side];

	while (result->children[1 - side] != NULL) {
		result = result->children[1 - side];
	}

	/* Unlink the result node, and hook in its remaining child
	* (if it has one) to replace it. */

	child = result->children[side];
	avl_map_node_replace(tree, result, child);

	/* Update the subtree height for the result node's old parent. */

	avl_map_update_height(result->parent);

	return result;
}

/* Remove a node from a tree */

static void avl_map_remove_node(AVLMap *tree, AVLMapNode *node)
{
	AVLMapNode *swap_node;
	AVLMapNode *balance_startpoint;
	int i;

	/* The node to be removed must be swapped with an "adjacent"
	* node, ie. one which has the closest key to this one. Find
	* a node to swap with. */

	swap_node = avl_map_node_get_replacement(tree, node);

	if (swap_node == NULL) {

		/* This is a leaf node and has no children, therefore
		* it can be immediately removed. */

		/* Unlink this node from its parent. */

		avl_map_node_replace(tree, node, NULL);

		/* Start rebalancing from the parent of the original node */

		balance_startpoint = node->parent;

	}
	else {
		/* We will start rebalancing from the old parent of the
		* swap node.  Sometimes, the old parent is the node we
		* are removing, in which case we must start rebalancing
		* from the swap node. */

		if (swap_node->parent == node) {
			balance_startpoint = swap_node;
		}
		else {
			balance_startpoint = swap_node->parent;
		}

		/* Copy references in the node into the swap node */

		for (i = 0; i<2; ++i) {
			swap_node->children[i] = node->children[i];

			if (swap_node->children[i] != NULL) {
				swap_node->children[i]->parent = swap_node;
			}
		}

		swap_node->height = node->height;

		/* Link the parent's reference to this node */

		avl_map_node_replace(tree, node, swap_node);
	}

	/* Destroy the node */

	free(node);

	/* Keep track of the number of nodes */

	--tree->num_nodes;

	/* Rebalance the tree */

	avl_map_balance_to_root(tree, balance_startpoint);
}

/* Remove a node by key */

int avl_map_remove(AVLMap *tree, AVLMapKey key)
{
	AVLMapNode *node;

	/* Find the node to remove */

	node = avl_map_lookup_node(tree, key);

	if (node == NULL) {
		/* Not found in tree */

		return 0;
	}

	/* Remove the node */

	avl_map_remove_node(tree, node);

	return 1;
}

static AVLMapNode *avl_map_lookup_node(AVLMap *tree, AVLMapKey key)
{
	AVLMapNode *node;
	int diff;

	/* Search down the tree and attempt to find the node which
	* has the specified key */

	node = tree->root_node;

	while (node != NULL) {

		diff = tree->compare_func(key, node->key, tree->param1, tree->param2);

		if (diff == 0) {
			/* Keys are equal: return this node */
			return node;
		}
		else if (diff < 0) {
			node = node->children[AVL_MAP_NODE_LEFT];
		}
		else {
			node = node->children[AVL_MAP_NODE_RIGHT];
		}
	}

	/* Not found */

	return NULL;
}

AVLMapValue avl_map_lookup(AVLMap *tree, AVLMapKey key)
{
	AVLMapNode *node;

	/* Find the node */

	node = avl_map_lookup_node(tree, key);

	if (node == NULL) {
		return AVL_MAP_NULL;
	}
	else {
		return node->value;
	}
}

static void avl_map_traver_subtree(AVLMap *tree, AVLMapNode *node, AVLMapVisitFunc func)
{
	if (node == NULL) return;
	avl_map_traver_subtree(tree, node->children[AVL_MAP_NODE_LEFT], func);
	avl_map_traver_subtree(tree, node->children[AVL_MAP_NODE_RIGHT], func);
	func(node->value);
}

void avl_map_traverse(AVLMap *tree, AVLMapVisitFunc func)
{
	avl_map_traver_subtree(tree, tree->root_node, func);
}
