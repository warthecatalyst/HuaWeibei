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

// #define Debug 1

using namespace std;
const string prefix = "/";
int clientNum, serverNum;
//客户节点存储的数据
unordered_map<string,int> clientName_to_ID;
vector<string> clientID_to_Name;
vector<vector<int>> demand; //demand[i][j]表示第i个时间节点，第j个客户节点的流量需求

//边缘节点存储的数据
unordered_map<string,int> serverName_to_ID;
vector<pair<string,int>> serverID_to_Val;
vector<int> server_Best_Costs;

vector<vector<int>> server_list;    //边缘节点的邻接链表
vector<vector<int>> client_list;    //客户节点的邻接链表，现在可能暂时用不到，之后优化时候再考虑

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

    clientName_to_ID.clear();
    serverName_to_ID.clear();
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
    int ten_percent = demand.size()*2/3;// 0.33 0.7 0.5 0.6 0.66(效果依次增加)
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

void mysolve() {
    vector<int> sequence = sortDemands();
    int times = sequence.size();
    vector<vector<vector<pair<int,int>>>> ans(demand.size());       //最终结果
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    vector<int> serverSort;                                         //用于排序的数组
    serverSort.reserve(serverNum);
    for(int i=0;i<serverID_to_Val.size();i++){
        serverSort.push_back(i);
    }
    while(times--) {
        int day = sequence[0];
        vector<int>& curDemand = demand[day];
        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的负载
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }
        while(!curDemandOver(curDemand)){//先把在本时刻会用掉次数的节点的次数用掉
            vector<int> server_Cost(serverNum); //处理到目前，每个边缘节点的负载(负载定义为边缘节点在当前时刻与其相邻的客户节点发出的请求)
            //计算每个节点的负载
            for(int i=0;i<serverNum;i++){
                for(int& neigh:server_list[i]){
                    server_Cost[i]+=curDemand[neigh];
                }
            }
            sort(serverSort.begin(),serverSort.end(),[&](const int& a,const int &b){
                return server_Cost[a]>server_Cost[b];
            });
            int serverId = -1;
            for(int sId:serverSort){
                if(serverTimes[sId]>0&&serverLoad[sId]>0 && serverTimes[sId]>0){
                    serverId = sId;
                    break;
                }
            }
            if(serverId==-1){
                break;
            }
            int curNeed = server_Cost[serverId];
            if(serverTimes[serverId]>0){    //使用这一次的机会
                serverTimes[serverId]--;    //用掉一次次数，然后就尽量把该节点分配出去
                int usedV = 0;
                for(int& client:server_list[serverId]){
                    if(serverLoad[serverId]==0){
                        break;
                    }
                    int minV = min(curDemand[client],serverLoad[serverId]);
                    curDemand[client]-=minV;
                    serverLoad[serverId]-=minV;
                    usedV += minV;
                    curAns[client].push_back({serverId,minV});
                }
                continue;
            }
        }
     
    }
}
void solve(vector<int>& sequence){
    vector<vector<vector<pair<int,int>>>> ans(demand.size());       //最终结果
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    vector<int> serverSort;                                         //用于排序的数组
    serverSort.reserve(serverNum);
    for(int i=0;i<serverID_to_Val.size();i++){
        serverSort.push_back(i);
    }
    for(const int& day:sequence){
        vector<int>& curDemand = demand[day];
        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的负载
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }
        while(!curDemandOver(curDemand)){//先把在本时刻会用掉次数的节点的次数用掉
            vector<int> server_Cost(serverNum); //处理到目前，每个边缘节点的负载(负载定义为边缘节点在当前时刻与其相邻的客户节点发出的请求)
            //计算每个节点的负载
            for(int i=0;i<serverNum;i++){
                for(int& neigh:server_list[i]){
                    server_Cost[i]+=curDemand[neigh];
                }
            }
            sort(serverSort.begin(),serverSort.end(),[&](const int& a,const int &b){
                return server_Cost[a]>server_Cost[b];
            });
            int serverId = -1;
            for(int sId:serverSort){
                if(serverTimes[sId]>0&&serverLoad[sId]>0&&server_Cost[sId]>=server_Best_Costs[sId] && serverTimes[sId]>0){
                    serverId = sId;
                    break;
                }
            }
            if(serverId==-1){
                break;
            }
            int curNeed = server_Cost[serverId];
            if(curNeed>=server_Best_Costs[serverId] && serverTimes[serverId]>0){    //使用这一次的机会
                serverTimes[serverId]--;    //用掉一次次数，然后就尽量把该节点分配出去
                int usedV = 0;
                for(int& client:server_list[serverId]){
                    if(serverLoad[serverId]==0){
                        break;
                    }
                    int minV = min(curDemand[client],serverLoad[serverId]);
                    curDemand[client]-=minV;
                    serverLoad[serverId]-=minV;
                    usedV += minV;
                    curAns[client].push_back({serverId,minV});
                }
                continue;
            }
        }
        
        while(!curDemandOver(curDemand)){
            vector<int> server_Cost(serverNum); //处理到目前，每个边缘节点的负载(负载定义为边缘节点在当前时刻与其相邻的客户节点发出的请求)
            //计算每个节点的负载

            for(int i=0;i<serverNum;i++){
                for(int& neigh:server_list[i]){
                    server_Cost[i]+=curDemand[neigh];
                }
            }
            sort(serverSort.begin(),serverSort.end(),[&](const int& a,const int &b){
                return server_Cost[a]>server_Cost[b];
            });
            int serverId = -1;
            for(int sId:serverSort){
                if(serverTimes[sId]>0&&serverLoad[sId]>0){
                    serverId = sId;
                    break;
                }
            }
            int curNeed = server_Cost[serverId];
            if(serverId==-1||server_Cost[serverId]==0){
                //调试时输出
            //    cout<<"log1"<<endl;
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
                if(curNeed>=server_Best_Costs[serverId]){    //使用这一次的机会
                    serverTimes[serverId]--;    //用掉一次次数，然后就尽量把该节点分配出去
                    int usedV = 0;
                    for(int& client:server_list[serverId]){
                        if(serverLoad[serverId]==0){
                            break;
                        }
                        int minV = min(curDemand[client],serverLoad[serverId]);
                        curDemand[client]-=minV;
                        serverLoad[serverId]-=minV;
                        usedV += minV;
                        curAns[client].push_back({serverId,minV});
                    }
                    //调试输出-用掉次数的情况下，节点分配了多少流量出去
                    // cout<<"usedV:"<<usedV<<endl;
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
#ifdef Debug
        for(int i=0;i<serverNum;i++){
            serverTotal[i][day] = serverID_to_Val[i].second-serverLoad[i];
        }
#endif
    }

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
            for(int j = 0;j<v1.size();j++){
                if(v1[j].second==0){
                    continue;
                }
                if(j>0){
                    outfile<<",";
                }
                const auto& v = v1[j];
                outfile<<"<"<<serverID_to_Val[v.first].first<<","<<v.second<<">";
            }
            outfile<<endl;
        }
    }
    outfile.close();

    //计算最终结果
#ifdef Debug
    int last = 0,totalUse = 0;
    for(int i=0;i<serverNum;i++){
        cout<<serverID_to_Val[i].first<<":";
        for(int j : serverTotal[i]){
            cout<<j<<" ";
        }
        sort(serverTotal[i].begin(),serverTotal[i].end());
        cout<<"final Cost:"<<serverTotal[i][demand.size()-1-five_percent]<<endl;
        totalUse+=five_percent-serverTimes[i];
        last+=serverTotal[i][demand.size()-1-five_percent];
    }
    cout<<"Total Cost = "<<last<<endl;
    cout<<"Total Use = "<<totalUse<<endl;
#endif

#ifdef Debug
    map<int,int> leftTimes;
    map<int,int> serverDegree;
    for(int i=0;i<serverNum;++i){
        leftTimes[serverTimes[i]]++;
        serverDegree[server_list[i].size()]++;
    }
    for(auto& l:leftTimes){
        cout<<"剩余次数为"<<l.first<<"的服务器有"<<l.second<<"个"<<endl;
    }
    cout<<"---------------------------"<<endl;
    for(auto&l:serverDegree){
        cout<<"度数为"<<l.first<<"的服务器有"<<l.second<<"个"<<endl;
    }



#endif
}



int main() {
//    clock_t startTime,endTime;
//    startTime = clock();

    ProcessInput(); //数据的输入处理
    vector<int> sequence = sortDemands();   //将每一天按照当天的请求总和排序，形成结果sequence
    solve(sequence);        //真正的处理函数

//    endTime = clock();
//    cout << "The run time is: " <<(double)(endTime - startTime)*1000 / CLOCKS_PER_SEC << "ms" << endl;
    return 0;
}
