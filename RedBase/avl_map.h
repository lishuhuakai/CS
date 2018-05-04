/*

Copyright (c) 2005-2008, Simon Howard

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice appear
in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/
#ifndef AVL_MAP_H
#define AVL_MAP_H

/**
 * AVL set是一个利用avl树实现的的一个集合.
 */

typedef struct _AVLMap AVLMap;
typedef void *AVLMapKey;
typedef void *AVLMapValue;
#define AVL_MAP_NULL ((void *) 0)
typedef struct _AVLMapNode AVLMapNode;
typedef enum {
	AVL_MAP_NODE_LEFT = 0,
	AVL_MAP_NODE_RIGHT = 1
} AVLMapNodeSide;

typedef void(*AVLMapVisitFunc)(AVLMapValue value);
// AVLMapCompareFunc 比较函数
typedef int(*AVLMapCompareFunc)(AVLMapValue value1, AVLMapValue value2, AVLMapValue param1, AVLMapValue param2);

AVLMap *avl_map_new(AVLMapCompareFunc compare_func, AVLMapValue param1, AVLMapValue param2);

void avl_map_free(AVLMap *tree);

bool avl_map_insert(AVLMap *tree, AVLMapKey key,
	AVLMapValue value);

int avl_map_remove(AVLMap *tree, AVLMapKey key);

AVLMapValue avl_map_lookup(AVLMap *tree, AVLMapKey key);

void avl_map_traverse(AVLMap *tree, AVLMapVisitFunc func);

#endif

