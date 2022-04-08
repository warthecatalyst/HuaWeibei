//
// Created by war on 2022/4/1.
//  

#ifndef QUATERFINAL_PROCESS_P5_H
#define QUATERFINAL_PROCESS_P5_H

#include "global.h"

// double param = 0.001;//0(513148)、0.001(498252)、0.002（515121）、0.005(539513)
int boundary;
double param = 0.183;
//10^8的搜索范围:0.2(506206)、、0.21（498008）、0.22（497753）、0.19（496650）、0.17（494161）、0.18（487281）、0.185（477389）、0.184（474509）、0.184-6（474218）、0.182-6（472836）、0.183-6（472185）、0.1835-6（472185）
bool CompBandwith(int s1, int s2){
    return serverID_to_Val[s1].second>serverID_to_Val[s2].second;
}



//在time时刻使用serverId的次数
//把serverId关联的所有流需求排序，每次寻找自己能够满足的最大需求，直到再也无法满足任何需求

void UseOutServer(int serverId, int time, vector<vector<unordered_map<string,int>>> &demand_remain, vector<vector<int>> &bandwith_remain, vector<vector<unordered_map<string,int>>> &result){
#ifdef Debug
    cout<<"------------------"<<endl;
    cout<<"day:"<<time<<", serverId:"<<serverId<<endl;
#endif
    //把关联的所有流按照需求大小排序，从大到小
    priority_queue<stream_request_test> request_queue;
    for(int clientId:server_list[serverId]){
        for(auto& stream:demand_remain[time][clientId]){
            stream_request_test r(stream.first, clientId, stream.second);
            request_queue.push(r);
        }
    }
    while(!request_queue.empty()){
        auto it = request_queue.top();
        request_queue.pop();

        if(bandwith_remain[time][serverId]>=it.need&&it.need>0){
            bandwith_remain[time][serverId] -= it.need;
            result[time][it.clientId][it.streamId] = serverId;
            demand_remain[time][it.clientId][it.streamId] = 0;
            //<流id，clientid，need> 剩余带宽
#ifdef Debug
            cout<<"<"<<it.streamId<<","<<it.clientId<<","<<it.need<<">   "<<bandwith_remain[time][serverId]<<endl;
#endif
        }
        else{
            continue;
        }
    }
}

//根据服务器关联的最大流的大小排序，从大到小
vector<pair<int, int>> SortTimeByMaxStream(int serverId, vector<vector<unordered_map<string,int>>> &demand_remain){
    vector<pair<int, int>> max_stream; //{时间， 总流数}
    for(int t=0;t<Times;++t){
        max_stream.emplace_back(t, 0);
        for(int clientId:server_list[serverId]){
            for(auto& stream:demand_remain[t][clientId]){
                if(stream.second>boundary){
                    max_stream[t].second += stream.second;
                }
            }
        }
    }
    sort(max_stream.begin(), max_stream.end(), [&](pair<int,int> a, pair<int, int> b){
        return a.second>b.second;
    });
    return max_stream;
}

void Process_p5(vector<vector<unordered_map<string,int>>> &demand_remain, vector<vector<int>> &bandwith_remain, vector<vector<unordered_map<string,int>>> &result){
    vector<int> serverSort;
    for(int i=0;i<serverNum;++i){
        serverSort.push_back(i);
    }
    //按照从边缘节点的带宽上限从大到小对边缘节点排序
    sort(serverSort.begin(), serverSort.end(), [&](const int& a,const int& b){
        return serverID_to_Val[a].second>serverID_to_Val[b].second;
    });

    // boundary = round(Max_Stream*param);
    sort(Max_Stream.begin(), Max_Stream.end());
    boundary = Max_Stream[(1 - param) * Max_Stream.size()];
    for(auto serverId:serverSort){
        vector<pair<int, int>> sequence = SortTimeByMaxStream(serverId, demand_remain);
        for(int k=0;k<five_percent;++k){
            int t = sequence[k].first;
            UseOutServer(serverId, t, demand_remain, bandwith_remain, result);
        }
    }
}

#endif //QUATERFINAL_PROCESS_P5_H
