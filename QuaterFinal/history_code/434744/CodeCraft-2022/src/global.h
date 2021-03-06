//
// Created by war on 2022/3/31.
//

#ifndef QUATERFINAL_GLOBAL_H
#define QUATERFINAL_GLOBAL_H

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <memory>
#include <ctime>
#include <cmath>
#include <queue>
#include <stack>
#include <random>
#include <chrono>

// #define Debug 1
#define DEBUG1 1

using namespace std;
using ll = long long;

const string prefix = "/";    

//基本消耗为V
int V;

//总共的时刻，边缘节点的数目和客户的数目
int Times, serverNum,clientNum;
int five_percent;   //5%的次数
vector<int> Max_Stream; //最大的流

vector<string> clientID_to_Name;
vector<vector<unordered_map<string,int>>> demand;
vector<pair<string,int>> serverID_to_Val;

vector<vector<int>> server_list;    //边缘节点的邻接链表
vector<vector<int>> client_list;    //客户节点的邻接链表

struct stream_request_test{
    string streamId;
    int clientId;
    int need;

    stream_request_test() = default;
    stream_request_test(string sid,int cid,int n):streamId(std::move(sid)),clientId(cid),need(n){}

    bool operator<(const stream_request_test& b) const{
        // return need < b.need ;
        return need / client_list[clientId].size() < b.need / client_list[b.clientId].size();
    }
};

struct stream_request{
    string streamId;
    int clientId;
    int need;

    stream_request() = default;
    stream_request(string sid,int cid,int n):streamId(std::move(sid)),clientId(cid),need(n){}

    bool operator<(const stream_request& b) const{
        return need < b.need ;
    }
};
#endif //QUATERFINAL_GLOBAL_H
