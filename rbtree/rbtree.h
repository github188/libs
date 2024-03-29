/*
 * rbtree.h -- Red Black Trees
 *
 * Copyright (C) 2016 liyunteng
 * Auther: liyunteng <li_yunteng@163.com>
 * License: GPL
 * Update time:  2016/04/17 20:49:06
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef RBTREE_H
#define RBTREE_H
#include "types.h"

struct rb_node {
	unsigned long  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
}__attribute__((aligned(sizeof(long))));


struct rb_root {
	struct rb_node *rb_node;
};


#define rb_parent(r)	((struct rb_node *) ((r)->__rb_parent_color & ~3))

#define RB_ROOT (struct rb_root) { NULL, }
#define rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root) (READ_ONCE((root)->rb_node) == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node)			\
	((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node)			\
	((node)->__rb_parent_color = (unsigned long)(node))


extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);

/* Find logical next and previous nodes in a tree */
extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct rb_root *);

/* Postorder interation - always visit the parent after its children */
extern struct rb_node *rb_first_postorder(const struct rb_root *);
extern struct rb_node *rb_next_postorder(const struct rb_node *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
extern void rb_replace_node(struct rb_node *victim, struct rb_node *new,
			struct rb_root *root);

static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
	struct rb_node **rb_link)
{
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

static inline void rb_link_node_rcu(struct rb_node *node, struct rb_node *parent,
			struct rb_node **rb_link)
{
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	rcu_assign_pointer(*rb_link, node);
}

#define rb_entry_safe(ptr, type, member)				\
	({								\
		____ptr ? rb_entry(____ptr, type, member) : NULL ;	\
	})
/**
 * rbtree_postorder_for_each_entry_safe -
 * iterateinpost-orderoverrb_root of given type allowing the backing
 * memory of @pos to be invalidated
 *
 * @pos:    the 'type *' to use as a loop cursor.
 * @n: another 'type *' to use as temporary storage.
 * @root: 'rb_root *' of the rbtree.
 * @field: the name of the rb_node field within 'type'.
 *
 * Description:
 * rbtree_postorder_for_each_entry_safe() provides a similar guarantee
 * as list_for_each_entry_safe() and allows the iteration to continue
 * independent of change to @pos by the body of the loop.
 *
 * NOTE:
 * however, that it cannot handle other modifications ths re-order the
 * rbtree it is iterating over. This includes calling rb_erase() on
 * @pos, as rb_erase() may rebalance the tree, causing us to miss some nodes.
 */
#define rbtree_postorder_for_each_entry_safe(pos, n, root, field)	\
	for (pos = rb_entry_safe(rb_first_postorder(root), typeof(*pos), field); \
	     pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
						     typeof(*pos), field); 1; }); \
		pos = n)
#endif
