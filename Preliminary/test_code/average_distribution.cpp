#include <iostream>
#include <fstream>
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

#define Debug 1

using namespace std;
const string prefix = "";
int clientNum, serverNum;
//客户节点存储的数据
unordered_map<string,int> clientName_to_ID;
vector<string> clientID_to_Name;
vector<vector<int>> demand; //demand[i][j]表示第i个时间节点，第j个客户节点的流量需求

//边缘节点存储的数据
unordered_map<string,int> serverName_to_ID;
vector<pair<string,int>> serverID_to_Val;
vector<int> server_Best_Costs;
vector<int> server_95per;  //上一轮迭代得到的95百分位带宽

vector<vector<int>> server_list;    //边缘节点的邻接链表
vector<vector<int>> client_list;    //客户节点的邻接链表，现在可能暂时用不到，之后优化时候再考虑
vector<int> server_sorted_by_degree;

int five_percent;

inline void ProcessClients(istringstream& is){     //主要是处理clientName_to_ID和clientID_to_Name
    string stn;
    getline(is,stn,',');
    int i = 0;
    while(getline(is,stn,',')){
        clientName_to_ID[stn] = i;
        clientID_to_Name.push_back(stn);
        i++;
    }
}

inline vector<int> ProcessNames(istringstream& is){
    vector<int> ans;
    string stn;
    getline(is,stn,',');
    while(getline(is,stn,',')){
        ans.push_back(clientName_to_ID[stn]);
    }
    return ans;
}

void ProcessInput(){

    ifstream infile(prefix+"data/demand.csv",ios::in);
    if(!infile){
        cout<<"Can't open file"<<endl;
        exit(1);
    }
    string line;
    getline(infile,line);
    istringstream is(line);
    ProcessClients(is);

//    for(auto &it:clientName_to_ID){
//        cout<<it.first<<" "<<it.second<<endl;
//    }
    //处理带宽值
    while(getline(infile,line)){
        vector<int> client_demand;
        is.clear();
        is.str(line);
        string stn;
        getline(is,stn,',');
        while(getline(is,stn,',')){
            client_demand.push_back(stoi(stn));
        }
        demand.push_back(client_demand);
    }

    five_percent = demand.size()/20;

//    for(auto& vec:demand){
//        for(auto &i:vec){
//            cout<<i<<" ";
//        }
//        cout<<endl;
//    }

    infile.close();
    infile.open(prefix+"data/site_bandwidth.csv");
    if(!infile){
        cout<<"Can't open file"<<endl;
        exit(1);
    }
    getline(infile,line);   //第一列没用
    int i = 0;
    while(getline(infile,line)){
        is.clear();
        is.str(line);
        string name,bandwidth;
        getline(is,name,',');
        getline(is,bandwidth,',');
        serverName_to_ID[name] = i++;
        serverID_to_Val.emplace_back(name,stoi(bandwidth));
    }

    clientNum = clientID_to_Name.size();
    serverNum = serverID_to_Val.size();
    server_list = vector<vector<int>>(serverNum);
    client_list = vector<vector<int>>(clientNum);
    server_Best_Costs = vector<int>(serverNum);
    server_95per = vector<int>(serverNum);
    server_sorted_by_degree.reserve(serverNum);
    for(int t=0;t<serverNum;t++){
        server_sorted_by_degree.push_back(t);
    }
//    for(auto& vec:serverID_to_Val){
//        cout<<vec.first<<" "<<vec.second<<endl;
//    }

    infile.close();
    infile.open(prefix+"data/config.ini");
    if(!infile){
        cout<<"Can't open file"<<endl;
        exit(1);
    }
    getline(infile,line);
    getline(infile,line);
    is.clear();
    is.str(line);
    string qo;
    getline(is,qo,'=');
    getline(is,qo,'=');
    int QOS = stoi(qo);


    infile.close();
    infile.open(prefix + "data/qos.csv");
    if(!infile){
        cout<<"Can't open file"<<endl;
        exit(1);
    }
    getline(infile,line);
    is.clear();
    is.str(line);
    vector<int> clientIDs = ProcessNames(is);
    i = 0;
    while(getline(infile,line)){
        is.clear();
        is.str(line);
        string name,qos;
        getline(is,name,',');
        int j = 0,serverId = serverName_to_ID[name];
        while(getline(is,qos,',')){
            if(stoi(qos)<QOS){
                server_list[serverId].push_back(j);
                client_list[j].push_back(serverId);
            }
            j++;
        }
    }
//    for(auto& neighs:server_list){
//        for(auto& neigh:neighs){
//            cout<<neigh<<" ";
//        }
//        cout<<endl;
//    }

    //对于每一个边缘节点，将他的邻接链表按照入度进行排序，优先满足入度更小的节点
    for(int i=0;i<serverNum;i++){
        sort(server_list[i].begin(),server_list[i].end(),[&](const int& a,const int& b){
            return client_list[a].size()<client_list[b].size();
        });
    }

    //对于每一个客户节点，将他的邻接链表按照最大带宽进行排序，优先分配给带宽更大的节点
    for(int i=0;i<clientNum;i++){
        sort(client_list[i].begin(),client_list[i].end(),[&](const int& a,const int& b){
            return serverID_to_Val[a].second>serverID_to_Val[b].second;
        });
    }

    sort(server_sorted_by_degree.begin(),server_sorted_by_degree.end(),[&](const int& a,const int& b){
        return server_list[a].size()>server_list[b].size();
    });

    clientName_to_ID.clear();
    serverName_to_ID.clear();
}

void output(vector<vector<vector<pair<int,int>>>>& ans){    //用于输出的函数
    ofstream outfile(prefix+"output/solution.txt",ios::out);

    if(!outfile){
        cout<<"Can't open output file"<<endl;
        exit(1);
    }

    //最后整合进行输出
    for(const auto& vec:ans){
        //第i天的输出结果
        for(int i=0;i<vec.size();i++){
            const auto& v1 = vec[i];
            clientID_to_Name[i].erase(std::remove(clientID_to_Name[i].begin(), clientID_to_Name[i].end(), '\r'), clientID_to_Name[i].end());
            outfile<<clientID_to_Name[i]<<":";
            bool flag = false;
            for(int j = 0;j<v1.size();j++){
                if(v1[j].second==0){
                    continue;
                }
                if(flag){
                    outfile<<",";
                }
                const auto& v = v1[j];
                outfile<<"<"<<serverID_to_Val[v.first].first<<","<<v.second<<">";
                flag = true;
            }
            outfile<<endl;
        }
    }
    outfile.close();
}

//将所有的时间节点按照当时的消耗进行排序，得到处理的顺序
//将所有的边缘节点的每日消耗进行排序
vector<int> sortDemands(){
    vector<pair<int,int>> dailyDemands;
    vector<vector<int>> server_Costs(serverNum,vector<int>(demand.size()));
    for(int i = 0;i<demand.size();i++){
        int curNeed = 0;
        for(int j = 0;j<clientNum;j++){
            curNeed+=demand[i][j];
            for(int& ser:client_list[j]){
                server_Costs[ser][i]+=demand[i][j];
            }
        }
        dailyDemands.emplace_back(i,curNeed);
    }
    vector<int> ans;
    sort(dailyDemands.begin(),dailyDemands.end(),[](const pair<int,int>& a,const pair<int,int>&b){
        return a.second>b.second;
    });
    ans.reserve(dailyDemands.size());
    for(auto& p:dailyDemands){
        //cout<<p.first<<" "<<p.second<<endl;
        ans.push_back(p.first);
    }
    int ten_percent = demand.size()*0.731;//   0.3 0.6 0.8 0.68 0.666 0.75 0.7 0.72 0.735 0.732 0.729 0.73 0.731(效果依次递增)
    for(int i=0;i<serverNum;i++){
        sort(server_Costs[i].begin(),server_Costs[i].end(),greater<int>());
        server_Best_Costs[i] = server_Costs[i][ten_percent];
    }
    return ans;
}

bool curDemandOver(vector<int>& curDemand){
    for(const int& d:curDemand){
        if(d>0){
            return false;
        }
    }
    return true;
}


void solve(const vector<int>& sequence,vector<vector<int>>& serverTotal,vector<vector<int>>& record,bool isNextRound = false){   //用于求解第一轮的结果
    vector<vector<vector<pair<int,int>>>> ans(demand.size());              //最终记录的结果，目前可能不需要，但先暂时保存着
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    vector<int> serverSort;                                         //用于排序的数组
    serverSort.reserve(serverNum);
    for(int i=0;i<serverID_to_Val.size();i++){
        serverSort.push_back(i);
    }

    for(const int& day:sequence){
        vector<int> curDemand = demand[day];

        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的负载
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }

        while(!curDemandOver(curDemand)){
            vector<int> server_Cost(serverNum,0); //处理到目前，每个边缘节点的负载(负载定义为边缘节点在当前时刻与其相邻的客户节点发出的请求)
            //计算每个节点的负载
            int serverId = -1,P = 0,maxTimes = record[day].size();
            record[day].clear();
            if(isNextRound&&maxTimes-->0){    //是第二大轮次，以节点的度数排序，优先使用度数更高的节点
                for(int sId:server_sorted_by_degree){
                    if(serverTimes[sId]>0){
                        serverId = sId;
                        P++;
                        break;
                    }
                }
            }else if(!isNextRound){
                for(int i=0;i<serverNum;i++){
                    for(int& neigh:server_list[i]){
                        server_Cost[i]+=curDemand[neigh];
                    }
                }
                sort(serverSort.begin(),serverSort.end(),[&](const int& a,const int &b){
                    return server_Cost[a]>server_Cost[b];
                    // return lastRoundPosition(serverTotal,a,day)<lastRoundPosition(serverTotal, b, day);
                });
                for(int sId:serverSort){
                    if(serverTimes[sId]>0&&serverLoad[sId]>0){
                        serverId = sId;
                        break;
                    }
                }
            }

            if(serverId==-1||server_Cost[serverId]==0){
                //调试时输出
//                cout<<"log1"<<endl;
                //说明已经没有节点可以使用这次次数了，直接进行平均分配法
                int clientId = 0;
                for(int i=0;i<clientNum;i++){
                    if(curDemand[i]>0){
                        clientId = i;
                        break;
                    }
                }
//                cout<<"clientID :"<<clientID_to_Name[clientId]<<" ,its demands: "<<curDemand[clientId] << " ,and it's neighbours: "<<endl;
                int neighLoad = 0;
                for(int& neigh:client_list[clientId]){
                    neighLoad+=serverLoad[neigh];
//                    cout<<serverID_to_Val[neigh].first<<" : "<<serverLoad[neigh]<<endl;
                }
//                cout<<endl;

                double curN = curDemand[clientId];
                for(int i = 0;i<client_list[clientId].size()&& curDemand[clientId]>0;i++){
                    int neigh = client_list[clientId][i];
                    if(serverLoad[neigh]==0){
                        continue;
                    }
                    double cL = (double)serverLoad[neigh]/(double)neighLoad;
                    int cuL = ceil(cL*(double)curN);    //向上取整
                    if(cuL>curDemand[clientId]){
                        cuL = curDemand[clientId];
                    }
                    if(cuL>serverLoad[neigh]){
                        cuL = serverLoad[neigh];
                    }
                    curDemand[clientId]-=cuL;
                    serverLoad[neigh]-=cuL;
                    curAns[clientId].push_back({neigh,cuL});
                }
            }else{
                int curNeed = server_Cost[serverId];
                if(isNextRound||curNeed>=server_Best_Costs[serverId]){    //使用这一次的机会
                    if(isNextRound){
                        cout<<"log1"<<endl;
                    }
                    serverTimes[serverId]--;    //用掉一次次数，然后就尽量把该节点分配出去
                    record[day].push_back(serverId);
                    for(int& client:server_list[serverId]){
                        if(serverLoad[serverId]==0){
                            break;
                        }
                        int minV = min(curDemand[client],serverLoad[serverId]);
                        curDemand[client]-=minV;
                        serverLoad[serverId]-=minV;
                        curAns[client].push_back({serverId,minV});
                    }
                }else{  //如果不使用次数，就将节点的负载按加权比例均分到与他相连的边缘节点（其边缘节点还剩下多少）
                    int clientId = 0;
                    for(int& neigh:server_list[serverId]){
                        if(curDemand[neigh]>0){
                            clientId = neigh;
                            break;
                        }
                    }
                    int neighLoad = 0;
                    for(int& neigh:client_list[clientId]){
                        neighLoad+=serverLoad[neigh];
                    }
                    double curN = curDemand[clientId];
                    for(int i = 0;i<client_list[clientId].size()&& curDemand[clientId]>0;i++){
                        int neigh = client_list[clientId][i];
                        if(serverLoad[neigh]==0){
                            continue;
                        }
                        double cL = (double)serverLoad[neigh]/(double)neighLoad;
                        int cuL = ceil(cL*(double)curN);    //向上取整
                        if(cuL>curDemand[clientId]){
                            cuL = curDemand[clientId];
                        }
                        if(cuL>serverLoad[neigh]){
                            cuL = serverLoad[neigh];
                        }
                        curDemand[clientId]-=cuL;
                        serverLoad[neigh]-=cuL;
                        curAns[clientId].push_back({neigh,cuL});
                    }
                }
            }
        }
//        cout<<"current day is: "<<day<<endl;
//        for(int i = 0;i<curAns.size();i++){
//            const auto& v1 = curAns[i];
//            cout<<clientID_to_Name[i]<<":";
//            for(const auto& v:v1){
//                cout<<"<"<<serverID_to_Val[v.first].first<<","<<v.second<<">,";
//            }
//            cout<<endl;
//        }
        ans[day] = curAns;
        for(int i=0;i<serverNum;i++){
            serverTotal[i][day] = serverID_to_Val[i].second-serverLoad[i];
        }
    }

    //计算最终结果

    int last = 0,totalUse = 0;
    for(int i=0;i<serverNum;i++){
#ifdef Debug
        cout<<serverID_to_Val[i].first<<":";
#endif
        vector<int> seri = serverTotal[i];
        sort(seri.begin(),seri.end());  //从小到大排序
        server_95per[i] = seri[demand.size()-1-five_percent];   //取得一轮的p95值
#ifdef Debug
        for(int j : seri){
            cout<<j<<" ";
        }
        cout<<"final Cost:"<<seri[demand.size()-1-five_percent]<<"\t";
        totalUse+=five_percent-serverTimes[i];
        cout<<"times Used:"<<five_percent-serverTimes[i]<<endl;
        last+=seri[demand.size()-1-five_percent];
#endif
    }
#ifdef Debug
    cout<<"Total Cost = "<<last<<endl;
    cout<<"Total Use = "<<totalUse<<endl;
#endif
}

int lastRoundPosition(vector<vector<int>>& serverTotal,int serverId,int day){
    vector<int> seri = serverTotal[serverId];
    sort(seri.begin(),seri.end());
    int pos = std::find(seri.begin(), seri.end(), serverTotal[serverId][day])-seri.begin()+1;
    return pos;
    // double cL = 1/ sqrt((double)pos/(double)seri.size());
    // return cL;
}

void furtherImprovement(const vector<int>& sequence,vector<vector<int>>& serverTotal,vector<vector<int>>& record,bool Positive,bool isLastRound = false){    //之后的进步轮次
    vector<vector<vector<pair<int,int>>>> ans(demand.size());              //最终记录的结果
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    for(const int& day:sequence){
        for(int sId:record[day]){
            serverTimes[sId]--;
        }
    }
    vector<int> server_max_use(server_95per);
    vector<int> server_80_per(serverNum);
    for(int i=0;i<serverNum;i++){
        auto seri = serverTotal[i];
        sort(seri.begin(),seri.end());
        server_80_per[i] = seri[seri.size()*0.5-1];
    }
    for(const int& day:sequence){   //同样按照天数进行处理
        vector<int> server_max_use_today(serverNum, 0);
        vector<int> curDemand = demand[day];
        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的负载
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }
        vector<int> servers_sortBy_lastrank;
        for(int i=0;i<serverNum;++i){
            servers_sortBy_lastrank.push_back(i);
        }
        //把上一轮已经用过的先用了
        for(int serverId:record[day]){
            for(int& client:server_list[serverId]){
                if(serverLoad[serverId]==0){
                    break;
                }
                int minV = min(curDemand[client],serverLoad[serverId]);
                curDemand[client]-=minV;
                serverLoad[serverId]-=minV;
                curAns[client].push_back({serverId,minV});
            }
        }
        unordered_set<int> rec(record[day].begin(),record[day].end());  //在该日已经使用过次数的服务器
        map<int, int> use_count;
        int round = 5;
        for(int i=0;i<serverNum;++i){
            use_count[i] = 5;
        }
        for(auto r:record[day]){
            use_count[r] = 0;
        }
        //再查看是否有上一轮没有使用但是大于P95的节点，全部使用
        vector<int> prepare;
        for(int i=0;i<serverNum;i++){
            if(serverTimes[i]>0&&!rec.count(i)&&serverTotal[i][day]>server_95per[i]){
                prepare.push_back(i);
            }
        }
        while(!curDemandOver(curDemand)){
            map<int,int> server_Cost;
            for(int pId:prepare){
                int pCost = 0;
                for(int& cId:server_list[pId]){
                    pCost+=curDemand[cId];
                }
                server_Cost[pId] = pCost;
            }
            sort(prepare.begin(),prepare.end(),[&](const int& a,const int& b){
                return server_Cost[a]<server_Cost[b];
                // return lastRoundPosition(serverTotal,a,day)>lastRoundPosition(serverTotal, b, day);
            });
            if(!prepare.empty()&&serverTimes[prepare.back()]>0&&server_Cost[prepare.back()]>server_95per[prepare.back()]){   //在还能使用次数的情况下优先使用次数
                int serverId = prepare.back();
                serverTimes[serverId]--;
                record[day].push_back(serverId);
                rec.insert(serverId);
                use_count[serverId] = 0;
                prepare.pop_back();
                for(int& client:server_list[serverId]){
                    if(serverLoad[serverId]==0){
                        break;
                    }
                    int minV = min(curDemand[client],serverLoad[serverId]);
                    curDemand[client]-=minV;
                    serverLoad[serverId]-=minV;
                    curAns[client].push_back({serverId,minV});
                }
            }else{
                //按照剩余服务器的60百分位带宽给值，找到还有剩余节点没有满足的服务器，之前已经使用过次数的服务器不再考虑

                // cout<<endl;
                int serverId = -1;
                for (int i = 0; i < serverNum; i++) {
                    int index = i;
                    if (use_count[index] != round) {
                        continue;
                    }
                    for (int client: server_list[index]) {
                        if (curDemand[client] > 0) {
                            serverId = index;
                            break;
                        }
                    }
                    if (serverId == index) {
                        break;
                    }
                }
                if(serverId==-1){
                    if(round>1){
                        round --;
                        continue;
                    }
                    int clientId = 0;
                    for(int i=0;i<clientNum;i++){
                        if(curDemand[i]>0){
                            clientId = i;
                            break;
                        }
                    }
                    int neighLoad = 0;
                    // double tt = 0.0;
                    for(int& neigh:client_list[clientId]){
                        neighLoad+=serverLoad[neigh];
                        // double cL = server_max_use[neigh]-( serverID_to_Val[neigh].second-serverLoad[neigh]);
                        // if(cL<0){
                        //     cL = 0;
                        // }
                        // tt += cL;
                    }
                    double curN = curDemand[clientId];
                    for(int i = 0;i<client_list[clientId].size()&& curDemand[clientId]>0;i++){
                        int neigh = client_list[clientId][i];
                        if(serverLoad[neigh]==0){
                            continue;
                        }
                        double cL = (double)serverLoad[neigh]/(double)neighLoad;
                        // double cL = (server_max_use[neigh]-( serverID_to_Val[neigh].second-serverLoad[neigh]))/tt;
                        int cuL = ceil(cL*(double)curN);    //向上取整
                        if(cuL>curDemand[clientId]){
                            cuL = curDemand[clientId];
                        }
                        if(cuL>serverLoad[neigh]){
                            cuL = serverLoad[neigh];
                        }
                        curDemand[clientId]-=cuL;
                        serverLoad[neigh]-=cuL;
                        server_max_use_today[neigh] += cuL;
                        curAns[clientId].push_back({neigh,cuL});
                    }
                }else{
                    //找到节点的80带宽值
                    rec.insert(serverId);
                    use_count[serverId]--;
                    int MaxiGive = server_max_use[serverId]*0.2;     //找到节点的80带宽值  0.18 0.18 0.19 195 0.2(效果依次递增)
                    //将Maxigive按照需求平均分配给他的客户节点
                    int clientNeed = 0;
                    for(int& client:server_list[serverId]){
                        clientNeed += curDemand[client];
                    }
                    int serLeft = MaxiGive;
                    for(int& client:server_list[serverId]){
                        double cL = (double)curDemand[client]/(double)clientNeed;
                        int cuL = ceil(cL*(double)MaxiGive);
                        if(cuL>curDemand[client]){
                            cuL = curDemand[client];
                        }
                        if(cuL>serLeft){
                            cuL = serLeft;
                        }
                        serLeft-=cuL;
                        curDemand[client]-=cuL;
                        serverLoad[serverId]-=cuL;
                        server_max_use_today[serverId] += cuL;
                        curAns[client].push_back({serverId,cuL});
                    }
                }
            }
        }
        for(int i=0;i<serverNum;++i){
            if(server_max_use[i]< server_max_use_today[i]){
                server_max_use[i] = server_max_use_today[i];
            }
        }

        ans[day] = curAns;
        for(int i=0;i<serverNum;i++){
            serverTotal[i][day] = serverID_to_Val[i].second-serverLoad[i];
        }
    }
    int last = 0,totalUse = 0;
    for(int i=0;i<serverNum;i++){
#ifdef Debug
        cout<<"------------------"<<endl;
        cout<<serverID_to_Val[i].first<<":";
#endif
        vector<int> seri = serverTotal[i];
        sort(seri.begin(),seri.end());  //从小到大排序
        server_95per[i] = seri[demand.size()-1-five_percent];   //取得一轮的p95值
#ifdef Debug
        for(int j : seri){
            // cout<<j<<" ";
        }
        int k = 0;
        int count = 0;
        for(int i=0;i<demand.size()-1-five_percent;++i){
            k += seri[i];
            count ++;
        }
        cout<<"avg: "<<k/count<<endl;
        cout<<"final Cost:"<<seri[demand.size()-1-five_percent]<<endl;
        totalUse+=five_percent-serverTimes[i];
        cout<<"times Used:"<<five_percent-serverTimes[i]<<endl;
        last+=seri[demand.size()-1-five_percent];
#endif
    }
#ifdef Debug
    cout<<"Total Cost = "<<last<<endl;
    cout<<"Total Use = "<<totalUse<<endl;
#endif
    if(isLastRound){
        output(ans);
    }
}



int main() {
//    clock_t startTime,endTime;
//    startTime = clock();
    const int MAXRound = 15;
    ProcessInput(); //数据的输入处理
    vector<int> sequence = sortDemands();   //将每一天按照当天的请求总和排序，形成结果sequence
    vector<vector<int>> record(demand.size());
    vector<vector<int>> serverTotal(serverNum,vector<int>(demand.size()));
    int round = 0;
#ifdef Debug
    cout<<"current round is "<<round<<endl;
#endif
    solve(sequence,serverTotal,record);
    round++;
    for(;round<MAXRound;round++){
#ifdef Debug
        cout<<"current round is "<<round<<endl;
#endif
        if(round==MAXRound-1){
            furtherImprovement(sequence,serverTotal,record, round%2==1,true);
        }else{
            furtherImprovement(sequence,serverTotal,record,round%2==1);
        }
    }
    solve(sequence,serverTotal,record,true);
    round = 1;
    for(;round<MAXRound;round++){
#ifdef Debug
        cout<<"current round is "<<round<<endl;
#endif
        if(round==MAXRound-1){
            furtherImprovement(sequence,serverTotal,record, round%2==1,true);
        }else{
            furtherImprovement(sequence,serverTotal,record,round%2==1);
        }
    }
//    endTime = clock();
//    cout << "The run time is: " <<(double)(endTime - startTime)*1000 / CLOCKS_PER_SEC << "ms" << endl;
    return 0;
}