#include <iostream>
#include <vector>

using namespace std;

int lowbit(int n){
    return n == INT_MIN ? n : n & (-n);
}

vector<int> findTwosingleNumber(vector<int>& nums) {
    int num1 = 0, num2 = 0;
    for(auto i : nums){
        num1 ^= i; // num1是nums中所有值进行异或的结果
    }
    int mask = lowbit(num1); // 获得上述循环结果的二进制表示最右边的一个1
    for(auto i : nums){
        if(i & mask)
            num2 ^= i; // 用这个结果进行分类, 可以得到其中一个只出现一次的数
    }
    num1 ^= num2; // 得到另一个只出现一次的数
    return vector<int>{num1, num2};
}
vector<int> singleNumber(vector<int> &nums) {
    int x = 0;
    for(auto i : nums){
        x ^= i;
    }
    int mask = 0;
    for(auto i : nums) {
        mask ^= lowbit(x^i);
    }
    mask = lowbit(mask);
    int num1 = 0;
    vector<int> n23;
    for(auto i : nums){
        if(mask & lowbit(x^i))
            num1 ^= i;
        else
            n23.push_back(i);
    }
    vector<int> result = findTwosingleNumber(n23);
    result.push_back(num1);
    return result;
}

int main(int argc, char const *argv[])
{
    vector<int> nums = {1, 2, 3, 4, 5, 10, 5, 4, 3, 2, 1, 20, 30};
    vector<int> result = singleNumber(nums);
    for(auto i : result){
        cout << i << endl;
    }
    return 0;
}
