#include "vl_set.h"
#include <stdlib.h>
#include <memory.h>

/**
 * vl_set is implemented as an red/black binary tree, which guarantees
 * *WORST CASE* performance of O(log(n)) in all cases, and best of O(1) in some cases.
 *
 * Based on the following documents:
 *  http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap13.htm
 *  http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap14.htm
 *
 * (Update 1/7/25: Links no longer work, use the wayback machine)
 *  http://web.archive.org/web/20240415162308/http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap13.htm
 *  http://web.archive.org/web/20210507110433/http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap14.htm
 */

enum vl_rbtree_color_ {
    vl_rbtree_color_red,
    vl_rbtree_color_black
};

/**
 * \brief Set tree node. Contains state and relationship information.
 * \private
 */
typedef struct vl_set_node {
    vl_usmall_t color;  //vl_rbtree_color_black
    vl_set_iter parent; //VL_SET_ITER_INVALID
    vl_set_iter left;   //VL_SET_ITER_INVALID
    vl_set_iter right;  //VL_SET_ITER_INVALID
} vl_set_node;

/**
 * \brief Getter for nodes which handles the null iterator case.
 * Returns null pointer if iter == VL_SET_ITER_INVALID.
 * \param set set pointer
 * \param iter node iterator.
 * \return set node pointer
 * \private
 */
static inline vl_set_node *vl_SetGetNodeAt(vl_set *set, vl_set_iter iter) {
    return iter == VL_SET_ITER_INVALID ? NULL : (vl_set_node *) vlPoolSample(&set->nodePool, iter);
}

vl_set_iter vlSetFront(vl_set *set) {
    vl_set_iter cur = set->root;
    vl_set_node *node = vl_SetGetNodeAt(set, cur);

    while (node ? node->left != VL_SET_ITER_INVALID : 0)
        node = vl_SetGetNodeAt(set, cur = node->left);

    return cur;
}

vl_set_iter vlSetBack(vl_set *set) {
    vl_set_iter cur = set->root;
    vl_set_node *node = vl_SetGetNodeAt(set, cur);

    while (node ? node->right != VL_SET_ITER_INVALID : 0)
        node = vl_SetGetNodeAt(set, cur = node->right);

    return cur;
}

/**
 * \private
 */
vl_set_iter vl_SetMinSubtree(vl_set *set, vl_set_iter cur) {
    if (cur == VL_SET_ITER_INVALID)
        return VL_SET_ITER_INVALID;

    vl_set_node *node = vl_SetGetNodeAt(set, cur);

    while (node->left != VL_SET_ITER_INVALID)
        node = vl_SetGetNodeAt(set, cur = node->left);

    return cur;
}

/**
 * \brief Performs a left-hand rotation.
 *
 * Rotations are a critical operation for RB binary trees.
 * Assumes right child of node is not null.
 *
 * If you would like to learn more, see the academic source listed at the top of this file.
 *
 * \param set pointer
 * \param nodeXIter target
 * \private
 */
void vl_SetTreeRotateLeft(vl_set *set, vl_set_iter nodeXIter) {
    vl_set_node *nodeX = vl_SetGetNodeAt(set, nodeXIter);
    vl_set_iter nodeYIter = nodeX->right;
    vl_set_node *nodeY = vl_SetGetNodeAt(set, nodeYIter);   //set y.

    nodeX->right = nodeY->left;             //Turn y's subtree into x's right subtree.
    if (nodeY->left != VL_SET_ITER_INVALID)
        vl_SetGetNodeAt(set, nodeY->left)->parent = nodeXIter;

    nodeY->parent = nodeX->parent;          //Link x's parent to y.

    if (nodeX->parent == VL_SET_ITER_INVALID) {
        set->root = nodeYIter;
    } else {
        vl_set_node *nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
        if (nodeXParent->left == nodeXIter)
            nodeXParent->left = nodeYIter;
        else
            nodeXParent->right = nodeYIter;
    }
    nodeY->left = nodeXIter;                // Put x on y's left.
    nodeX->parent = nodeYIter;
}


/**
 * \brief Performs a right-hand rotation.
 *
 * Rotations are a critical operation for RB binary trees.
 * Assumes left child of node is not null.
 *
 * If you would like to learn more, see the academic source listed at the top of this file.
 *
 * \param set pointer
 * \param nodeXIter target
 * \private
 */
void vl_SetTreeRotateRight(vl_set *set, vl_set_iter nodeXIter) {
    vl_set_node *nodeX = vl_SetGetNodeAt(set, nodeXIter);
    vl_set_iter nodeYIter = nodeX->left;
    vl_set_node *nodeY = vl_SetGetNodeAt(set, nodeYIter);   //set y.

    nodeX->left = nodeY->right;             //Turn y's subtree into x's left subtree.
    if (nodeY->right != VL_SET_ITER_INVALID)
        vl_SetGetNodeAt(set, nodeY->right)->parent = nodeXIter;

    nodeY->parent = nodeX->parent;          //Link x's parent to y.

    if (nodeX->parent == VL_SET_ITER_INVALID) {
        set->root = nodeYIter;
    } else {
        vl_set_node *nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
        if (nodeXParent->right == nodeXIter)
            nodeXParent->right = nodeYIter;
        else
            nodeXParent->left = nodeYIter;
    }
    nodeY->right = nodeXIter;                // Put x on y's right.
    nodeX->parent = nodeYIter;
}

/**
 * \private
 */
vl_set_iter vl_SetTreeSuccessor(vl_set *set, vl_set_iter nodeXIter) {
    vl_set_node *nodeX = vl_SetGetNodeAt(set, nodeXIter);
    if (nodeX->right != VL_SET_ITER_INVALID)
        return vl_SetMinSubtree(set, nodeX->right);

    vl_set_iter nodeYIter = nodeX->parent;
    while (nodeYIter != VL_SET_ITER_INVALID) {
        vl_set_node *nodeY = vl_SetGetNodeAt(set, nodeYIter);
        if (nodeY->right != nodeXIter)
            break;
        nodeXIter = nodeYIter;
        nodeYIter = nodeY->parent;
    }

    return nodeYIter;
}

void vlSetInit(vl_set *set, vl_memsize_t elementSize, vl_compare_function compFunc) {
    vlPoolInit(&set->nodePool, sizeof(vl_set_node) + elementSize);
    set->elementSize = elementSize;
    set->root = VL_SET_ITER_INVALID;
    set->comparator = compFunc;
    set->totalElements = 0;
}

void vlSetFree(vl_set *set) {
    vlPoolFree(&set->nodePool);
}

vl_set *vlSetNew(vl_memsize_t elementSize, vl_compare_function compFunc) {
    vl_set *set = malloc(sizeof(vl_set));
    vlSetInit(set, elementSize, compFunc);
    return set;
}

void vlSetDelete(vl_set *set) {
    vlSetFree(set);
    free(set);
}

void *vlSetSample(vl_set *set, vl_set_iter iter) {
    return (vl_memory *) (vlPoolSample(&set->nodePool, iter)) + sizeof(vl_set_node);
}

vl_set_iter vlSetNext(vl_set *set, vl_set_iter iter) {
    if (iter == VL_SET_ITER_INVALID)
        return VL_SET_ITER_INVALID;

    vl_set_node *node = vl_SetGetNodeAt(set, iter);

    if (node->right != VL_SET_ITER_INVALID) {
        vl_set_iter childIter = node->right;
        vl_set_node *child = vl_SetGetNodeAt(set, childIter);
        while (child->left != VL_SET_ITER_INVALID)
            child = vl_SetGetNodeAt(set, childIter = child->left);
        return childIter;
    }

    vl_set_iter parentIter = node->parent;
    vl_set_node *parent = vl_SetGetNodeAt(set, parentIter);

    if (parentIter == VL_SET_ITER_INVALID)
        return VL_SET_ITER_INVALID;

    if (parent->left == iter)
        return parentIter;

    //ascend the tree until we're no longer looking at a right-hand child
    while (parent->right == iter) {
        iter = parentIter;
        parentIter = parent->parent;

        if (parentIter == VL_SET_ITER_INVALID)
            break;

        parent = vl_SetGetNodeAt(set, parentIter);
    }

    //we must have ascended up to the root of the tree or found a suitable parent...
    return parentIter;
}

vl_set_iter vlSetPrev(vl_set *set, vl_set_iter iter) {
    if (iter == VL_SET_ITER_INVALID)
        return VL_SET_ITER_INVALID;

    vl_set_node *node = vl_SetGetNodeAt(set, iter);

    if (node->left != VL_SET_ITER_INVALID) {
        vl_set_iter childIter = node->left;
        vl_set_node *child = vl_SetGetNodeAt(set, childIter);
        while (child->right != VL_SET_ITER_INVALID)
            child = vl_SetGetNodeAt(set, childIter = child->right);
        return childIter;
    }

    vl_set_iter parentIter = node->parent;
    vl_set_node *parent = vl_SetGetNodeAt(set, parentIter);

    if (parentIter == VL_SET_ITER_INVALID)
        return VL_SET_ITER_INVALID;

    if (parent->right == iter) {
        return parentIter;
    }

    while (parent->left == iter) {
        iter = parentIter;
        parentIter = parent->parent;

        if (parentIter == VL_SET_ITER_INVALID)
            break;

        parent = vl_SetGetNodeAt(set, parentIter);
    }

    return parentIter;
}

vl_set_iter vlSetInsert(vl_set *set, const void *elem) {
    //Regular tree insertion...

    vl_set_iter resultIter = VL_SET_ITER_INVALID;
    vl_set_iter curIter = set->root;

    if (curIter == VL_SET_ITER_INVALID) {
        resultIter = vlPoolTake(&set->nodePool);
        vl_set_node *newNode = (vl_set_node *) vlPoolSample(&set->nodePool, resultIter);
        newNode->parent = VL_SET_ITER_INVALID;
        newNode->left = VL_SET_ITER_INVALID;
        newNode->right = VL_SET_ITER_INVALID;

        newNode->color = vl_rbtree_color_black;

        memcpy(newNode + 1, elem, set->elementSize);

        set->totalElements++;
        return set->root = resultIter;
    }

    while (curIter != VL_SET_ITER_INVALID) {
        vl_set_node *curNode = (vl_set_node *) vlPoolSample(&set->nodePool, curIter);
        int comp = set->comparator(curNode + 1, elem);

        if (comp == 0)
            return curIter;//Element already exists in the set...

        vl_set_iter *nextIter = comp > 0 ? &curNode->left : &curNode->right;

        if (*nextIter != VL_SET_ITER_INVALID) {
            curIter = *nextIter;
            continue;
        }

        resultIter = vlPoolTake(&set->nodePool);

        vl_set_node *newNode = (vl_set_node *) vlPoolSample(&set->nodePool, resultIter);
        newNode->parent = curIter;
        newNode->left = VL_SET_ITER_INVALID;
        newNode->right = VL_SET_ITER_INVALID;

        newNode->color = vl_rbtree_color_red;

        set->totalElements++;
        memcpy(newNode + 1, elem, set->elementSize);
        *nextIter = resultIter;
        break;
    }

    //Then, some fixup to retain RB tree rules...

    vl_set_iter nodeXIter = resultIter;
    while (nodeXIter != set->root) {
        vl_set_node *nodeX = vl_SetGetNodeAt(set, nodeXIter);
        vl_set_node *nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
        if (nodeXParent->color != vl_rbtree_color_red)
            break;

        vl_set_node *nodeXGrandparent = vl_SetGetNodeAt(set, nodeXParent->parent);
        if (nodeX->parent == nodeXGrandparent->left) {
            vl_set_iter nodeYIter = nodeXGrandparent->right;
            vl_set_node *nodeY = vl_SetGetNodeAt(set, nodeYIter);

            //All nil nodes are BLACK, thus the ternary if.
            if (nodeY ? nodeY->color == vl_rbtree_color_red : 0) {
                nodeXParent->color = vl_rbtree_color_black;
                nodeY->color = vl_rbtree_color_black;
                nodeXGrandparent->color = vl_rbtree_color_red;

                nodeXIter = nodeXParent->parent;
            } else {
                if (nodeXIter == nodeXParent->right) {
                    nodeXIter = nodeX->parent;
                    nodeX = vl_SetGetNodeAt(set, nodeXIter);
                    vl_SetTreeRotateLeft(set, nodeXIter);

                    nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
                    nodeXGrandparent = vl_SetGetNodeAt(set, nodeXParent->parent);
                }

                nodeXParent->color = vl_rbtree_color_black;
                nodeXGrandparent->color = vl_rbtree_color_red;
                vl_SetTreeRotateRight(set, nodeXParent->parent);
            }
        } else {
            vl_set_iter nodeYIter = nodeXGrandparent->left;
            vl_set_node *nodeY = vl_SetGetNodeAt(set, nodeYIter);

            //All nil nodes are BLACK, thus the ternary if.
            if (nodeY ? nodeY->color == vl_rbtree_color_red : 0) {
                nodeXParent->color = vl_rbtree_color_black;
                nodeY->color = vl_rbtree_color_black;
                nodeXGrandparent->color = vl_rbtree_color_red;

                nodeXIter = nodeXParent->parent;
            } else {
                if (nodeXIter == nodeXParent->left) {
                    nodeXIter = nodeX->parent;
                    nodeX = vl_SetGetNodeAt(set, nodeXIter);
                    vl_SetTreeRotateRight(set, nodeXIter);

                    nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
                    nodeXGrandparent = vl_SetGetNodeAt(set, nodeXParent->parent);
                }

                nodeXParent->color = vl_rbtree_color_black;
                nodeXGrandparent->color = vl_rbtree_color_red;
                vl_SetTreeRotateLeft(set, nodeXParent->parent);
            }
        }
    }

    vl_SetGetNodeAt(set, set->root)->color = vl_rbtree_color_black;
    return resultIter;
}

/**
 * \brief Removal fixup algorithm.
 *
 * Taken almost directly from the academic source listed at the top of this file.
 *
 * \param set pointer
 * \param nodeXIter fixup target
 * \private
 */
void vl_SetRemoveFixup(vl_set *set, vl_set_iter nodeXIter) {
    while (nodeXIter != set->root && nodeXIter != VL_SET_ITER_INVALID) {
        vl_set_node *nodeX = vl_SetGetNodeAt(set, nodeXIter);
        if (nodeX->color != vl_rbtree_color_black)
            break;

        vl_set_node *nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
        if (nodeXIter == nodeXParent->left) {
            vl_set_node *nodeW = vl_SetGetNodeAt(set, nodeXParent->left);
            vl_set_iter nodeWIter = nodeXParent->left;

            if (nodeW->color == vl_rbtree_color_red) {
                nodeW->color = vl_rbtree_color_black;
                nodeXParent->color = vl_rbtree_color_red;

                vl_SetTreeRotateLeft(set, nodeX->parent);
                nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);

                nodeW = vl_SetGetNodeAt(set, nodeXParent->right);
                nodeWIter = nodeXParent->right;
            }

            vl_set_node *nodeLeft = vl_SetGetNodeAt(set, nodeW->left);
            vl_set_node *nodeRight = vl_SetGetNodeAt(set, nodeW->right);
            const vl_usmall_t leftColor = nodeLeft ? nodeLeft->color : vl_rbtree_color_black;
            const vl_usmall_t rightColor = nodeRight ? nodeRight->color : vl_rbtree_color_black;

            if (leftColor == vl_rbtree_color_black && rightColor == vl_rbtree_color_black) {
                nodeW->color = vl_rbtree_color_red;
                nodeXIter = nodeX->parent;
            } else {
                if (rightColor == vl_rbtree_color_black) {
                    nodeW->color = vl_rbtree_color_red;
                    vl_SetTreeRotateRight(set, nodeWIter);
                    nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
                    nodeW = vl_SetGetNodeAt(set, nodeXParent->right);
                }

                nodeW->color = nodeXParent->color;
                nodeXParent->color = vl_rbtree_color_black;

                if (nodeW->right != VL_SET_ITER_INVALID)
                    vl_SetGetNodeAt(set, nodeW->right)->color = vl_rbtree_color_black;

                vl_SetTreeRotateLeft(set, nodeX->parent);

                nodeXIter = set->root;
            }
        } else {
            vl_set_node *nodeW = vl_SetGetNodeAt(set, nodeXParent->right);
            vl_set_iter nodeWIter = nodeXParent->right;

            if (nodeW->color == vl_rbtree_color_red) {
                nodeW->color = vl_rbtree_color_black;
                nodeXParent->color = vl_rbtree_color_red;

                vl_SetTreeRotateRight(set, nodeX->parent);
                nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);

                nodeW = vl_SetGetNodeAt(set, nodeXParent->left);
                nodeWIter = nodeXParent->left;
            }

            vl_set_node *nodeLeft = vl_SetGetNodeAt(set, nodeW->left);
            vl_set_node *nodeRight = vl_SetGetNodeAt(set, nodeW->right);
            const vl_usmall_t leftColor = nodeLeft ? nodeLeft->color : vl_rbtree_color_black;
            const vl_usmall_t rightColor = nodeRight ? nodeRight->color : vl_rbtree_color_black;

            if (leftColor == vl_rbtree_color_black && rightColor == vl_rbtree_color_black) {
                nodeW->color = vl_rbtree_color_red;
                nodeXIter = nodeX->parent;
            } else {
                if (leftColor == vl_rbtree_color_black) {
                    nodeW->color = vl_rbtree_color_red;
                    vl_SetTreeRotateLeft(set, nodeWIter);
                    nodeXParent = vl_SetGetNodeAt(set, nodeX->parent);
                    nodeW = vl_SetGetNodeAt(set, nodeXParent->left);
                }

                nodeW->color = nodeXParent->color;
                nodeXParent->color = vl_rbtree_color_black;

                if (nodeW->right != VL_SET_ITER_INVALID)
                    vl_SetGetNodeAt(set, nodeW->right)->color = vl_rbtree_color_black;

                vl_SetTreeRotateRight(set, nodeX->parent);

                nodeXIter = set->root;
            }
        }
    }

    if (nodeXIter != VL_SET_ITER_INVALID)
        vl_SetGetNodeAt(set, nodeXIter)->color = vl_rbtree_color_black;
}

void vlSetRemove(vl_set *set, vl_set_iter nodeZIter) {
    vl_set_node *nodeZ = vl_SetGetNodeAt(set, nodeZIter);

    vl_set_iter nodeYIter = VL_SET_ITER_INVALID;
    if (nodeZ->left == VL_SET_ITER_INVALID || nodeZ->right == VL_SET_ITER_INVALID) {
        nodeYIter = nodeZIter;
    } else {
        nodeYIter = vl_SetTreeSuccessor(set, nodeZIter);
    }

    vl_set_node *nodeY = vl_SetGetNodeAt(set, nodeYIter);

    vl_set_iter nodeXIter = nodeY->left != VL_SET_ITER_INVALID ? nodeY->left : nodeY->right;

    vl_set_node *nodeX = vl_SetGetNodeAt(set, nodeXIter);
    if (nodeX)
        nodeX->parent = nodeY->parent;

    if (nodeY->parent == VL_SET_ITER_INVALID) {
        set->root = nodeXIter;
    } else {
        vl_set_node *nodeYParent = vl_SetGetNodeAt(set, nodeY->parent);
        if (nodeYIter == nodeYParent->left)
            nodeYParent->left = nodeXIter;
        else
            nodeYParent->right = nodeXIter;
    }

    if (nodeYIter != nodeZIter)
        memcpy(nodeZ + 1, nodeY + 1, set->elementSize);

    if (nodeY->color == vl_rbtree_color_black)
        vl_SetRemoveFixup(set, nodeXIter);

    set->totalElements--;
    vlPoolReturn(&set->nodePool, nodeYIter);
}

void vlSetRemoveElem(vl_set *set, const void *elem) {
    vlSetRemove(set, vlSetFind(set, elem));
}

void vlSetClear(vl_set *set) {
    vlPoolClear(&set->nodePool);
    set->root = VL_SET_ITER_INVALID;
    set->totalElements = 0;
}

vl_set *vlSetClone(const vl_set *src, vl_set *dest) {
    if (dest == NULL)
        dest = vlSetNew(src->elementSize, src->comparator);

    vlPoolClone(&src->nodePool, &dest->nodePool);
    dest->totalElements = src->totalElements;
    dest->elementSize = src->elementSize;
    dest->comparator = src->comparator;
    dest->root = src->root;

    return dest;
}

int vlSetCopy(vl_set *src, vl_set_iter begin, vl_set_iter end, vl_set *dest) {
    if (src->elementSize != dest->elementSize || src->comparator != dest->comparator)
        return 0;

    if (begin == VL_SET_ITER_INVALID)
        begin = vlSetFront(src);

    if (end == VL_SET_ITER_INVALID)
        end = vlSetBack(src);

    int totalCopied = 0;
    vl_set_iter curIter = begin;

    while (curIter != VL_SET_ITER_INVALID) {
        vlSetInsert(dest, vlSetSample(src, curIter));
        totalCopied++;

        if (curIter == end)
            break;
        curIter = vlSetNext(src, curIter);
    }

    return totalCopied;
}

vl_set_iter vlSetFind(vl_set *set, const void *elem) {
    vl_set_iter result = set->root;

    while (result != VL_SET_ITER_INVALID) {
        vl_set_node *node = vl_SetGetNodeAt(set, result);
        const int comp = set->comparator(node + 1, elem);

        if (comp == 0)
            break;

        result = comp > 0 ? node->left : node->right;
    }

    return result;
}

vl_set *vlSetUnion(vl_set *a, vl_set *b, vl_set *dest) {
    if ((a->elementSize != b->elementSize) || (a->comparator != b->comparator))
        //Mismatched input set properties.
        return NULL;

    if (dest == NULL)
        dest = vlSetNew(a->elementSize, a->comparator);

    VL_SET_FOREACH(a, curIter)vlSetInsert(dest, vlSetSample(a, curIter));
    VL_SET_FOREACH(b, curIter)//Duplicate values are implicitly ignored.
        vlSetInsert(dest, vlSetSample(b, curIter));

    return dest;
}

vl_set *vlSetIntersection(vl_set *a, vl_set *b, vl_set *dest) {
    if ((a->elementSize != b->elementSize) || (a->comparator != b->comparator))
        //Mismatched input set properties.
        return NULL;

    if (dest == NULL)
        dest = vlSetNew(a->elementSize, a->comparator);


    VL_SET_FOREACH(a, curIter) {
        const void *const sample = vlSetSample(a, curIter);
        if (vlSetFind(b, sample) != VL_SET_ITER_INVALID)
            vlSetInsert(dest, sample);
    }

    return dest;
}

vl_set *vlSetDifference(vl_set *a, vl_set *b, vl_set *dest) {
    if ((a->elementSize != b->elementSize) || (a->comparator != b->comparator))
        //Mismatched input set properties.
        return NULL;

    if (dest == NULL)
        dest = vlSetNew(a->elementSize, a->comparator);

    VL_SET_FOREACH(a, curIter) {
        const void *const sample = vlSetSample(a, curIter);

        if (vlSetFind(b, sample) == VL_SET_ITER_INVALID)
            vlSetInsert(dest, sample);
    }

    return dest;
}