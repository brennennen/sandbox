/*
g++ -std=c++17 104_binary_tree_max_depth.cpp && ./a.out

Given the root of a binary tree, return its maximum depth.

A binary tree's maximum depth is the number of nodes along the
longest path from the root node down to the farthest leaf node.

Example 1:
Input: root = [3,9,20,null,null,15,7]
Output: 3

Example 2:
Input: root = [1,null,2]
Output: 2

Constraints:
    The number of nodes in the tree is in the range [0, 104].
    -100 <= Node.val <= 100
*/

#include <cassert>
#include <cstdio>

#include <algorithm>
#include <cstdlib>
#include <vector>
#include <queue>

using namespace std;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

/**
 * Takes in an array of numbers representing the tree in a breadth first format,
 * with -1 representing null.
 *
 * input: "1, 2, 3"
 * output: a tree with the root node being "1", the left node being "2", and the right node
 * being "3".
 *
 * input: "3, 9, 20, -1, -1, 15, 7"
 * output:
 *  root: 3
 *    left: 9
 *      left: null
 *      right: null
 *    right: 20
 *      left: 15
 *      right: 7
 */
TreeNode* buildTreeBreadthFirst(const vector<int>& input) {
    queue<int> valueQueue;
    for (int element: input) {
        valueQueue.push(element);
    }

    queue<TreeNode*> nodeQueue;
    TreeNode* root = (TreeNode*) malloc(sizeof(TreeNode));
    root->val = valueQueue.front();
    valueQueue.pop();
    nodeQueue.push(root);

    while(!valueQueue.empty()) {
        TreeNode* node = nodeQueue.front();
        nodeQueue.pop();

        int leftValue = valueQueue.front();
        valueQueue.pop();
        int rightValue = valueQueue.front();
        valueQueue.pop();

        if (leftValue != -1) {
            node->left = (TreeNode*) malloc(sizeof(TreeNode));
            node->left->val = leftValue;
            nodeQueue.push(node->left);
        }
        if (rightValue != -1) {
            node->right = (TreeNode*) malloc(sizeof(TreeNode));
            node->right->val = rightValue;
            nodeQueue.push(node->right);
        }
    }
    return root;
}
// prints a tree as the breadth first input format described in the function above.
void printBreadthFirst(TreeNode* root) {
    queue<TreeNode*> queue;
    if (root) {
        queue.push(root);
        printf("%d ", root->val);
    }
    while (!queue.empty()) {
        const TreeNode* const node = queue.front();
        queue.pop();

        if (node->left) {
            queue.push(node->left);
            printf("%d ", node->left->val);
        } else {
            printf("-1 ");
        }

        if (node->right) {
            queue.push(node->right);
            printf("%d ", node->right->val);
        } else {
            printf("-1 ");
        }
    }
    printf("\n");
}

int maxDepth(TreeNode* root) {
    if (root == nullptr) {
        return 0;
    }
    return 1 + max(maxDepth(root->left), maxDepth(root->right));
}

int main(int argc, char* argv[]) {
    {
        vector<int> treeInput = {3, 9, 20, -1, -1, 15, 7};
        TreeNode* tree = buildTreeBreadthFirst(treeInput);
        printf("input->right->left->val: %d\n", tree->right->left->val); // sanity check
        printBreadthFirst(tree);
        int result = maxDepth(tree);
        printf("test1 result: %d\n", result);
        assert(result == 3);
    }
    {
        TreeNode input = TreeNode();
        int result = maxDepth(&input);
        printf("test3 result: %d\n", result);
        assert(result == 1);
    }
}
