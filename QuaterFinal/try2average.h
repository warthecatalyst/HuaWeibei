//
// Created by war on 2022/4/1.
//

#ifndef QUATERFINAL_TRY2AVERAGE_H
#define QUATERFINAL_TRY2AVERAGE_H
#include "global.h"

bool curDemandOver(vector<unordered_map<string,int>>& curDemand){
    for(const auto& mp:curDemand){
        for(const auto& d:mp){
            if(d.second>0){
                return false;
            }
        }
    }
    return true;
}

void CulCost(vector<int> server_p95){
    int sum = 0;
    for(int i=0;i<serverNum;++i){
        int temp = V;
        if(server_p95[i]>V){
            temp = (server_p95[i]-V)*(server_p95[i]-V)/serverID_to_Val[i].second+server_p95[i];
        }
        sum += temp;
    }
    cout<<"total cost: "<<sum<<endl;
    
}


//服务器的Id，服务器的P95用量，以及即将需要用到的需求
double calCostAdd(int sId,double curUsed,double willUse){
    double lastCost;
    if(curUsed<=V){
        lastCost = V;
    }else{
        lastCost = curUsed+(1/(double)serverID_to_Val[sId].second)*(curUsed-V)*(curUsed-V);
    }
    double finalCost;
    if(willUse<=V){
        finalCost = V;
    }else{
        finalCost = willUse+(1/(double)serverID_to_Val[sId].second)*(willUse-V)*(willUse-V);
    }
    return finalCost-lastCost;
}

//input:最外层表示的是每个不同的时刻，中间表示不同的client，最内层的unordered_map则表示流ID->数据
void try2average(vector<vector<unordered_map<string,int>>> &restDemands, vector<vector<int>> &restServers,vector<vector<unordered_map<string,int>>>& ans) {
    vector<int> server_95per = vector<int>(serverNum,0);//记录每个边缘节点当前p95
    for (int t = 0; t < Times; t++) {
        vector<unordered_map<string,int>>& curDemand = restDemands[t];
        vector<int>& curServer = restServers[t];
        if (curDemandOver(curDemand))
            continue;

        //将所有的流从大到小排序
        priority_queue<stream_request> pq_curAllStreams;
        for (int i = 0; i < curDemand.size(); i++) {
            for (auto& k: curDemand[i]) {
                if(k.second==0){
                    continue;
                }
                pq_curAllStreams.push(stream_request(k.first, i, k.second));
            }
        }

        while (!pq_curAllStreams.empty()) {
            stream_request curStream = pq_curAllStreams.top();
            pq_curAllStreams.pop();
            pair<int, double> minValue(-1, INT32_MAX);//(边缘节点Id, 最小值)
            // //方案1：
            // for(auto& sId: client_list[curStream.clientId]) {
            //     if (curServer[sId] >= curStream.need && server_95per[sId] < serverID_to_Val[sId].second-curServer[sId]+curStream.need) {
            //         int temp = (server_95per[sId] - V) / serverID_to_Val[sId].second;
            //         if (minValue.second > temp){
            //             minValue.first = sId;
            //             minValue.second = temp;
            //         }
            //     }
            // }
            //方案2：计算增加需求的成本最低的节点
            for(auto& sId: client_list[curStream.clientId]) {
                if (curServer[sId] >= curStream.need) {
                    double costAdd = calCostAdd(sId,serverID_to_Val[sId].second-curServer[sId],serverID_to_Val[sId].second-curServer[sId]+curStream.need);
                    if(costAdd< minValue.second){
                        minValue.second = costAdd;
                        minValue.first = sId;
                    }
                }
            }
            if(minValue.first != -1) {
                int sId = minValue.first;
                curServer[sId] -= curStream.need;//更新服务器剩余带宽
                ans[t][curStream.clientId][curStream.streamId] = sId;//记录结果
            }
        }
    }
#ifdef Debug1
    vector<vector<int>> serverTotal(serverNum,vector<int>(Times));
    for(int i = 0;i<serverNum;i++){
        for(int day = 0;day<Times;day++){
            serverTotal[i][day] = serverID_to_Val[i].second-restServers[day][i];
        }
        sort(serverTotal[i].begin(),serverTotal[i].end(),greater<int>());
        server_95per[i] = serverTotal[i][five_percent];
    }
    CulCost(server_95per);
#endif
}



#endif //QUATERFINAL_TRY2AVERAGE_H
