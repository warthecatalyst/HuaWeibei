#include "ProcessInput.h"
bool curDemandOver(vector<unordered_map<string,int>>& curDemand){
    for(const int& d:curDemand){
        if(d.second>0){
            return false;
        }
    }
    return true;
}
//input:最外层表示的是每个不同的时刻，中间表示不同的client，最内层的unordered_map则表示流ID->数据
vector<vector<unordered_map<string,int>>> try2average(vector<vector<unordered_map<string,int>>> &restDemands, vector<vector<int>> &restServers) {
    vector<int> server_95per = vector<int>(serverNum,0);//记录每个边缘节点当前p95
    vector<vector<unordered_map<string,int>>> ans(demand.size(),vector<unordered_map<string,int>>(clientNum));  //第t天第i个客户节点的第k个流是否分配给第j个边缘节点
    for (int t = 0; t < Times; t++) {
        vector<unordered_map<string,int>> curDemand = restDemands[t];
        vector<int> curServer = restServers[t];
        if (curDemandOver(curDemand))
            continue;
        vector<stream_request>  curAllStreams;
        for (int i = 0; i < curDemand.size(); i++) {
            for (auto& k: curDemand[i]) {
                stream_request temp = new stream_request(k.first, i, k.second);
                curAllStreams.push(temp);
            }
        }
        //将所有的流从大到小排序
        priority_queue<stream_request> pq_curAllStreams;
        for (int k = 0; k < curAllStreams.size(); k++) {
            pq_curAllStreams.push(curAllStreams[k]);
        }
        while (!pq_curAllStreams.empty()) {
            stream_request curStream = pq_curAllStreams.top();
            bool flag = false;
            //如果当前最大流k被分配后没有超过当前边缘节点j，则分配给边缘节点sId；
            for(auto& sId: client_list[curStream.clientId]) {
                if (curServer[sId] >= curStream.need && server_95per[sId] >= curStream.need) {
                    curServer[sId] -= curStream.need;//更新服务器剩余带宽
                    ans[t][curStream.clientId][curStream.streamId] = sId;//记录结果
                    flag = true;
                    break;
                }
            }
            //如果当前最大流分配后都超过了所有边缘节点的p95,则计算导数，分配给导数最小的边缘节点，并更新它的p95
            if (!flag) {
                pair<int, int> minValue(-1, INT_MAX32);//(边缘节点Id, 最小值)
                // //方案1：
                // for(auto& sId: client_list[curStream.clientId]) {
                //     if (curServer[sId] >= curStream.need && server_95per[sId] < curStream.need) {
                //         int temp = (server_95per[sId] - V) / serverID_to_Val[sId];
                //         if (minValue.second > temp){
                //             minValue.first = sId;
                //             minValue.second = temp
                //         }
                //     }
                // }
                //方案2：
                for(auto& sId: client_list[curStream.clientId]) {
                    if (curServer[sId] >= curStream && server_95per[sId] < curStream.need) {
                        int temp = serverID_to_Val[sId] - curServer[sId] + curStream.need;//当天的边缘节点sId带宽消耗量 + 当前流
                        int diff = abs(temp - server_95per[sId]);
                        if (minValue.second > diff) {
                            minValue.first = sId;
                            minValue.second = diff;
                        }
                    }
                }
                if(minValue.first != -1) {
                    int sId = minValue.first;
                    curServer[sId] -= curStream.need;//更新服务器剩余带宽
                    ans[t][curStream.clientId][curStream.streamId] = sId;//记录结果
                    server_95per[sId] = serverID_to_Val[sId] - curServer[sId] + curStream.need;//更新sId的当前p95
                }
            }
            pq_curAllStreams.pop();
        }
        restServers[t] = curServer;//更新边缘节点剩余带宽（暂时没用）
        restDemands[t] = curDemand;
    }
    return ans;
}

int main() {
    ProcessInput();
    return 0;
}
