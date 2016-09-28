#ifndef __LIBS_SKEW_HEAP_H__
#define __LIBS_SKEW_HEAP_H__

// 这个文件提供了基本的优先队列数据结构

struct skew_heap_entry {
     struct skew_heap_entry *parent, *left, *right;
};

typedef struct skew_heap_entry skew_heap_entry_t;

typedef int(*compare_f)(void *a, void *b);

static inline void skew_heap_init(skew_heap_entry_t *a) __attribute__((always_inline));
static inline skew_heap_entry_t *skew_heap_merge(
     skew_heap_entry_t *a, skew_heap_entry_t *b,
     compare_f comp);
static inline skew_heap_entry_t *skew_heap_insert(
     skew_heap_entry_t *a, skew_heap_entry_t *b,
     compare_f comp) __attribute__((always_inline));
static inline skew_heap_entry_t *skew_heap_remove(
     skew_heap_entry_t *a, skew_heap_entry_t *b,
     compare_f comp) __attribute__((always_inline));

static inline void
skew_heap_init(skew_heap_entry_t *a)
{
     a->left = a->right = a->parent = NULL;
}

static inline skew_heap_entry_t *
skew_heap_merge(skew_heap_entry_t *a, skew_heap_entry_t *b,
                compare_f comp) // 用于合并
{
	// 这玩意到底是从小到大排还是从大到小排的？或者说，这里构建了一个最大堆
     if (a == NULL) return b;
     else if (b == NULL) return a;
     
     skew_heap_entry_t *l, *r;
     if (comp(a, b) == -1) // 右边比左边要大,翻译过来的话，就是a < b
     {
          r = a->left; 
          l = skew_heap_merge(a->right, b, comp); // 因为a < b,所以b要和a右边的元素进行比较,也就是说a->right > a
		  // skew_heap_merge(a->right, b, comp)假设已经排好了序
		  // 
          
          a->left = l; //  l > a
          a->right = r; // 
          if (l) l->parent = a; // 如果l不为空

          return a;
     }
     else // 左边大于右边,也就是说a > b
     {
          r = b->left;
          l = skew_heap_merge(a, b->right, comp);
          
          b->left = l;
          b->right = r;
          if (l) l->parent = b;

          return b;
     }
}

static inline skew_heap_entry_t *
skew_heap_insert(skew_heap_entry_t *a, skew_heap_entry_t *b,
                 compare_f comp) // a是队列的首部, *b是要插入的entry, comp是比较函数
{
     skew_heap_init(b);
     return skew_heap_merge(a, b, comp); // 用于合并
}

static inline skew_heap_entry_t *
skew_heap_remove(skew_heap_entry_t *a, skew_heap_entry_t *b,
                 compare_f comp)
{ // 从a这个链表中移除b
     skew_heap_entry_t *p   = b->parent;
     skew_heap_entry_t *rep = skew_heap_merge(b->left, b->right, comp);
     if (rep) rep->parent = p;
     
     if (p)
     {
          if (p->left == b)
               p->left = rep;
          else p->right = rep;
          return a;
     }
     else return rep;
}

#endif    /* !__LIBS_SKEW_HEAP_H__ */
