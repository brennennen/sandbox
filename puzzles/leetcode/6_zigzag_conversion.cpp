/*
P   A   H   N
A P L S I I G
Y   I   R
*/

#include <cstdio>
#include <string>
#include <vector>

using namespace std;


string convert(string s, int numRows) {
    if (numRows == 1) {
        return s;
    }
    if (s.length() <= numRows) {
        return s;
    }

    vector<vector<int>> grid = vector<vector<int>> {};
    grid.resize(numRows, vector<int>(numRows, 0));
    
    vector col1 = vector<int>{};
    col1.resize(numRows);
    for (int i = 0; i < numRows; i++) {
        col1[i] = s[i];
    }

    return "";
}

int main(int argc, char* argv[]) {

    string input1 = "PAYPALISHIRING";
    int input1_num_rows = 4;
    string output1 = convert(input1, input1_num_rows);
}
