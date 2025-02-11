/*
g++ -std=c++17 226_invert_binary_tree.cpp && ./a.out

Given the root of a binary tree, invert the tree, and return its root.

Example 1:
Input: root = [4,2,7,1,3,6,9]
Output: [4,7,2,9,6,3,1]

Example 2:
Input: root = [2,1,3]
Output: [2,3,1]

Example 3:
Input: root = []
Output: []

Constraints:
    The number of nodes in the tree is in the range [0, 100].
    -100 <= Node.val <= 100
*/


#include <cassert>
#include <cstdio>
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

TreeNode* invertTree(TreeNode* root) {
    if (root == nullptr) {
        return nullptr;
    }

    // Swap the left and right nodes
    TreeNode* temp = root->left;
    root->left = root->right;
    root->right = temp;

    // Recurse
    invertTree(root->left);
    invertTree(root->right);
    return root;
}

int main(int argc, char* argv[]) {
    {
        vector<int> treeInput = {4, 2, 7, 1, 3, 6, 9};
        TreeNode* tree = buildTreeBreadthFirst(treeInput);
        printBreadthFirst(tree);
        TreeNode* result = invertTree(tree);
        printBreadthFirst(result);
        assert(result->val == 4);
        assert(result->left->val == 7);
        assert(result->right->val == 2);
        assert(result->left->left->val == 9);
        assert(result->left->right->val == 6);
        assert(result->right->left->val == 3);
        assert(result->right->right->val == 1);
    }
}
