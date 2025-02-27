/*
g++ ./83_remove_duplicates_from_sorted_list.cpp && ./a.out

https://leetcode.com/problems/remove-duplicates-from-sorted-list/

Given the head of a sorted linked list, delete all duplicates such that each element 
appears only once. Return the linked list sorted as well.

Example 1:
Input: head = [1,1,2]
Output: [1,2]

Example 2:
Input: head = [1,1,2,3,3]
Output: [1,2,3]

Constraints:
    The number of nodes in the list is in the range [0, 300].
    -100 <= Node.val <= 100
    The list is guaranteed to be sorted in ascending order.
*/

#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <cstdio>

// PROVIDED
class ListNode {
public:
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};
// END PROVIDED

/*
First thoughts:
* Walk the list, comparing the current value with the next value, if they are the
same, delete the next node and point the current's next field to 1 ahead.

Mistake:
* I moved the "current" forward even when deleting, this caused missing a delete
when there is a double repeat at the end of the list (test case: [1, 1, 1]). Only
moving the "current" pointer forward when NOT deleting was the fix.
*/

ListNode* deleteDuplicates(ListNode* head) {
    ListNode* current = head;
    while (current != nullptr) {
        if (current->next != nullptr && current->val == current->next->val) {
            // free current->next if the memory allocation model requires it.
            current->next = current->next->next;
        }
        else {
            current = current->next;
        }
    }
    return head;
}

int main(int argc, char* argv[]) {
    {
        ListNode third = ListNode(2);
        ListNode second = ListNode(1, &third);
        ListNode input = ListNode(1, &second);
        ListNode* result = deleteDuplicates(&input);
        assert(result->val == 1);
        assert(result->next->val == 2);
        assert(result->next->next == nullptr);
    }
    {
        ListNode third = ListNode(1);
        ListNode second = ListNode(1, &third);
        ListNode input = ListNode(1, &second);
        ListNode* result = deleteDuplicates(&input);
        assert(result->val == 1);
        assert(result->next == nullptr);
    }
}
