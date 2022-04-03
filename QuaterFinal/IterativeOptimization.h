//
// Created by war on 2022/4/3.
//

#ifndef QUATERFINAL_ITERATIVEOPTIMIZATION_H
#define QUATERFINAL_ITERATIVEOPTIMIZATION_H

#include "global.h"
void IterativeOprimize(vector<int> server_p95, vector<vector<int>> server_total){
    for(int t=0;t<Times;++t){

    }
}

vector<int> SortTimeByServerCost(vector<vector<int>> server_total, int serverId){
    vector<int> sequence;
    for(int i=0;i<Times;++i){
        sequence.push_back(i);
    }
    sort(sequence.begin(), sequence.end(), [&](int t1, int t2){
        return server_total[t1][serverId]>server_total[t2][serverId];
    });
    return sequence;
}

void OptimizeOneServer(vector<vector<unordered_map<string,int>>>& ans, int serverId, vector<int> &server_p95, vector<vector<int>> &server_total){
    vector<int> sequence = SortTimeByServerCost(server_total, serverId);
    for(int k = five_percent;k<Times;k++){
        int t = sequence[k];
        priority_queue<stream_request_test> requests;
        for(int clientId=0;clientId<clientNum;++clientId){
            unordered_map<string, int> stream_list = ans[t][clientId];
            for(auto& stream:stream_list){
                if(stream.second==serverId){
                    requests.emplace(stream.first, clientId, demand[t][clientId][stream.first]);
                }
            }
        }
        bool flag = false;
        int cost_today = server_total[t][serverId];
        while(!requests.empty()){
            stream_request_test re = requests.top();
            requests.pop();
            for(int otherServer:client_list[re.clientId]){
                if(otherServer != serverId && server_p95[otherServer]>=server_total[t][otherServer]+re.need){
                    //cout<<re.need<<endl;
                    ans[t][re.clientId][re.streamId] = otherServer;
                    server_total[t][serverId] -= re.need;
                    server_total[t][otherServer] += re.need;
                    flag = true;
                    break;
                }
            }
            // if(flag)
            //     break;
        }
        if(!flag)
            break;
    }
    int begin = server_p95[serverId];
    vector<int> v;
    for(int t=0;t<Times;++t){
        v.push_back(server_total[t][serverId]);
    }
    sort(v.begin(), v.end());
    server_p95[serverId] = v[Times-1-five_percent];
    int end = server_p95[serverId];
    // cout<<end-begin<<"---"<<endl;

}

#endif //QUATERFINAL_ITERATIVEOPTIMIZATION_H
