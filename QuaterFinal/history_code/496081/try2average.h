//
// Created by war on 2022/4/1.
//

#ifndef QUATERFINAL_TRY2AVERAGE_H
#define QUATERFINAL_TRY2AVERAGE_H
#include "global.h"
#include "IterativeOptimization.h"

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

vector<vector<int>> GetServerTotal(vector<vector<unordered_map<string,int>>>& ans){

    vector<vector<int>> server_total;
    for(int t =0;t<Times;++t){
        vector<int> v(serverNum, 0);
        for(int clientId=0;clientId<clientNum;++clientId){
            for(auto s:ans[t][clientId]){
                v[s.second] += demand[t][clientId][s.first];
            }
        }
        server_total.push_back(v);
    }
    return server_total;
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
        priority_queue<stream_request_test> pq_curAllStreams;
        for (int i = 0; i < curDemand.size(); i++) {
            for (auto& k: curDemand[i]) {
                if(k.second==0){
                    continue;
                }
                pq_curAllStreams.push(stream_request_test(k.first, i, k.second));
            }
        }

        while (!pq_curAllStreams.empty()) {
            stream_request_test curStream = pq_curAllStreams.top();
            pq_curAllStreams.pop();
            bool flag = false;
            int best_server = -1;
            int max_remain = -1;
            // for(auto& sId: client_list[curStream.clientId]) {
            //     int curUsed = serverID_to_Val[sId].second - curServer[sId]; //当前已经使用的部分
            //     if (curServer[sId] >= curStream.need && server_95per[sId] >= curStream.need+curUsed) {
            //         if(server_95per[sId] - curStream.need-curUsed >max_remain){
            //             best_server = sId;
            //             max_remain = server_95per[sId] - curStream.need-curUsed;
            //             flag = true;
            //         }
            //     }
            // }
            // if(flag){
            //     curServer[best_server] -= curStream.need;                                   //更新服务器剩余带宽
            //     curDemand[curStream.clientId][curStream.streamId]-=curStream.need;      //更新当前的需求，分配完成的需求不再考虑
            //     ans[t][curStream.clientId][curStream.streamId] = best_server;//记录结果
            // }

            // //如果当前最大流k被分配后没有超过当前边缘节点sId的p95，则分配给边缘节点sId；
            // for(auto& sId: client_list[curStream.clientId]) {
            //     int curUsed = serverID_to_Val[sId].second - curServer[sId]; //当前已经使用的部分
            //     if (curServer[sId] >= curStream.need && server_95per[sId] >= curStream.need+curUsed) {
            //         curServer[sId] -= curStream.need;                                   //更新服务器剩余带宽
            //         curDemand[curStream.clientId][curStream.streamId]-=curStream.need;      //更新当前的需求，分配完成的需求不再考虑
            //         ans[t][curStream.clientId][curStream.streamId] = sId;//记录结果
            //         flag = true;
            //         break;
            //     }
            // }

            //如果当前最大流分配后都超过了所有边缘节点的p95,则计算增加成本，分配给增加成本最小的边缘节点，并更新它的p95
            if (!flag) {
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
                        double costAdd = calCostAdd(sId,server_95per[sId],serverID_to_Val[sId].second-curServer[sId]+curStream.need);
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
                    server_95per[sId] = serverID_to_Val[sId].second - curServer[sId] + curStream.need;//更新sId的当前p95
                }
            }

        }
    }
#ifdef DEBUG1
    cout<<"--------------befor-------------"<<endl;
    CulCost(server_95per);

    cout<<"---------------after-------------"<<endl;
#endif
    for(int l=0;l<4;++l){
        auto server_total = GetServerTotal(ans);
        for(int i=0;i<serverNum;++i){
            OptimizeOneServer(ans,i, server_95per, server_total);
        }
#ifdef DEBUG1
        CulCost(server_95per);
#endif
    }
}



#endif //QUATERFINAL_TRY2AVERAGE_H
