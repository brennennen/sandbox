/*
g++ -std=c++17 ./121_best_time_to_buy_and_sell_stock.cpp && ./a.out

https://leetcode.com/problems/best-time-to-buy-and-sell-stock

You are given an array prices where prices[i] is the price of a given stock on the 
ith day. You want to maximize your profit by choosing a single day to buy one stock 
and choosing a different day in the future to sell that stock. Return the maximum 
profit you can achieve from this transaction. If you cannot achieve any profit, 
return 0.

Example 1:
Input: prices = [7,1,5,3,6,4]
Output: 5
Explanation: Buy on day 2 (price = 1) and sell on day 5 (price = 6), profit = 6-1 = 5.
Note that buying on day 2 and selling on day 1 is not allowed because you must buy 
before you sell.

Example 2:
Input: prices = [7,6,4,3,1]
Output: 0
Explanation: In this case, no transactions are done and the max profit = 0.

Constraints:
    1 <= prices.length <= 105
    0 <= prices[i] <= 104
*/

#include <vector>

#include <cassert>
#include <cstdio>


using namespace std;

/*
First thoughts:
* Brute force/first pass, could be for each day, iterating forward finding the greatest
positive delta between days. O(n^2)
* Brute force worked for the first couple provided ~6 day test cases. Got time limit
exceeded issues on further test cases when submitting.
*/
int maxProfit__bad_exponential_time(vector<int>& prices) {
    int max_total_delta = 0;
    for (int i = 0; i < prices.size(); i++) {
        int max_day_delta = 0;
        for (int j = i; j < prices.size(); j++) {
            int delta = prices[j] - prices[i];
            if (delta > max_day_delta) {
                max_day_delta = delta;
            }
        }
        if (max_day_delta > max_total_delta) {
            max_total_delta = max_day_delta;
        }
    }
    return max_total_delta;
}
/*
Second pass:
* 2 routes to go: 
   * could try to finesse the brute force approach, find a way to reduce the 
   iterations/scope of data being processed.
   * or try to go back to square one and see if there is another approach, some
   way to exploit the format of the data or rules of the problem that I didn't catch
   the first time around.
* Maybe pre-iterate over the list, finding the min and max and use that somehow.
* Thinking through the problem a bit more, it looks like you don't need the second
loop. You can exploit the fact that this is time series data, so you can just keep
track of the current minimum day and current max profits.
*/
int maxProfit(vector<int>& prices) {
    int min_day = prices[0];
    int max_profit = 0;
    for (int i = 0; i < prices.size(); i++) {
        if (prices[i] < min_day) {
            min_day = prices[i];
        }
        if (prices[i] - min_day > max_profit) {
            max_profit = prices[i] - min_day ;
        }
    }
    return max_profit;
}

int main(int argc, char* argv[]) {
    {
        vector<int> input = {7, 1, 5, 3, 6, 4};
        int result = maxProfit(input);
        printf("test1: %d\n", result);
        assert(result == 5);
    }
    {
        vector<int> input = {7, 6, 4, 3, 1};
        int result = maxProfit(input);
        printf("test2: %d\n", result);
        assert(result == 0);
    }
    {
        vector<int> input = {1, 2};
        int result = maxProfit(input);
        printf("test2: %d\n", result);
        assert(result == 1);
    }
}
