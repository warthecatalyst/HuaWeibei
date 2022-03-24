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
#include <queue>
#include <algorithm>
#include <stdlib.h>
#include <cstring>

// #include "InternetStream.cpp"
using namespace std;
// #define Debug 1


const int msize = 2000;


int N, M;   // N--路径数, M--结点数
int r[msize][msize];  //
int pre[msize];  // 记录结点i的前向结点为pre[i]
bool vis[msize]; // 记录结点i是否已访问

// 用BFS来判断从结点s到t的路径上是否还有delta
// 即判断s,t之间是否还有增广路径，若有，返回1
bool BFS(int s, int t)
{
    queue<int> que;
    memset(pre, -1, sizeof(pre));
    memset(vis, false, sizeof(vis));
    vector<int> route;
    pre[s] = s;
    vis[s] = true;
    que.push(s);
    route.push_back(s);
    int p;
    while(!que.empty())
    {
        p = que.front();
        que.pop();
        for(int i=1; i<=M; ++i)
        {
            if(r[p][i]>0 && !vis[i])
            {
                pre[i] = p;
                vis[i] = true;
                if(i == t){  // 存在增广路径

                    return true;
                }
                que.push(i);
                route.push_back(i);
            }
        }
    }
    return false;
}

int EK(int s, int t)
{
    // cout<<"EK in"<<endl;
    int maxflow = 0, d;
    while(BFS(s, t))
    {
        d= INT32_MAX;
        // 若有增广路径，则找出最小的delta
        for(int i=t; i!=s; i=pre[i]){
            d = min(d, r[pre[i]][i]);
            
        }

        // 这里是反向边，看讲解
        for(int i=t; i!=s; i=pre[i])
        {
            r[pre[i]][i] -= d;
            r[i][pre[i]] += d;
        }
        maxflow += d;
    }
    // cout<<maxflow<<endl;
    // cout<<"EK out"<<endl;
    return maxflow;
}

int EK(int s, int t, vector<vector<vector<int>>> &route_list)
{
    int maxflow = 0, d;
    while(BFS(s, t))
    {
        d= INT32_MAX;
        // 若有增广路径，则找出最小的delta
        for(int i=t; i!=s; i=pre[i]){
            d = min(d, r[pre[i]][i]);
            
        }

        vector<vector<int>> one_route;
        // 这里是反向边，看讲解
        for(int i=t; i!=s; i=pre[i])
        {
            r[pre[i]][i] -= d;
            r[i][pre[i]] += d;
            vector<int> one_edge;
            one_edge.push_back(pre[i]);
            one_edge.push_back(i);
            one_edge.push_back(d);
            one_route.push_back(one_edge);
        }
        maxflow += d;
        route_list.push_back(one_route);
    }
    
    return maxflow;
}

int GetMaxStream(int n, int m, vector<vector<int>> edges){
    N = n;
    M = m;
    memset(r, 0, sizeof(r));
        int s, e, c;
        for(int i=0; i<N; ++i)
        {
            vector<int> edge = edges[i];
            s = edge[0];
            e = edge[1];
            c = edge[2];
            r[s][e] += c;   // 有重边时则加上c
        }
        return EK(0, M);
}

vector<vector<vector<int>>> FindRoute(int n, int m, vector<vector<int>> edges){
    N = n;
    M = m;
    memset(r, 0, sizeof(r));
    int s, e, c;
    for(int i=0; i<N; ++i)
    {
        vector<int> edge = edges[i];
        s = edge[0];
        e = edge[1];
        c = edge[2];
        r[s][e] += c;   // 有重边时则加上c
    }
    vector<vector<vector<int>>> ret;
    EK(0, M, ret);
    return ret;
}




const string prefix = "";
int clientNum, serverNum;
//客户节点存储的数据
unordered_map<string,int> clientName_to_ID;
vector<string> clientID_to_Name;
vector<vector<int>> demand; //demand[i][j]表示第i个时间节点，第j个客户节点的流量需求
vector<int> server_avg;
vector<vector<vector<int>>> graph;
vector<vector<int>> server_load;
vector<vector<int>> server_load_sorted;
vector<int> server_load_avg;
vector<int> SERVER_COST_RATE;
vector<vector<int>> SERVER_LOAD_SORTED;
//边缘节点存储的数据
unordered_map<string,int> serverName_to_ID;
vector<pair<string,int>> serverID_to_Val;
vector<int> server_Best_Costs;
vector<int> server_95per;  //上一轮迭代得到的95百分位带宽

vector<vector<int>> server_list;    //边缘节点的邻接链表
vector<vector<int>> client_list;    //客户节点的邻接链表，现在可能暂时用不到，之后优化时候再考虑
vector<unordered_set<int>> FULL_SERVER_ID;

int five_percent;
int BASE_RATE=1000;
int RATE = BASE_RATE;
int STEP = 100;

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
}


bool server_to_client(vector<vector<int>> &graph, int serverId, int clientId, int val){
    if(graph[serverId][clientId]+val>serverID_to_Val[serverId].second){
        return false;
    }
    graph[serverId][clientId] += val;
    return true;
}

bool client_to_server(vector<vector<int>> &graph, int serverId, int clientId, int val){
    if(graph[serverId][clientId]<val){
        return false;
    }
    graph[serverId][clientId] -= val;
    return true;
}

void init(){
    ProcessInput();
    for(int day=0;day<demand.size();++day){
        vector<vector<int>> graph_daily;
        for(int serverid=0;serverid<serverNum;++serverid){
            vector<int> v;
            for(int clientid=0;clientid<clientNum;++clientid){
                v.push_back(0);
            }
            graph_daily.push_back(v);
        }
        graph.push_back(graph_daily);
    }

    SERVER_COST_RATE = vector<int>(serverNum, BASE_RATE);
    
}
int getFullServerIndex(int day){
    int a = day/(demand.size()*0.05);
    int b = serverNum * 0.05;
    return a*b;
}

void AvgDistribute(){
    for(int day = 0;day<demand.size();++day){
        auto demand_today = demand[day];
        for(int clientId=0;clientId<client_list.size();++clientId){
            int cur_demand = demand_today[clientId];
            int demand_left = cur_demand;
            
            for(int serverId:client_list[clientId]){
                int min_val = ceil(cur_demand/client_list[clientId].size());
                if(min_val>demand_left){
                    min_val = demand_left;
                }
                demand_left -= min_val;
                graph[day][serverId][clientId] += min_val;
            }
        }
    }
}
void CulculateServerLoadAVG(){
    
    for(int serverId=0;serverId<serverNum;++serverId){
        vector<int> v(demand.size(), 0);
        for(int day=0;day<demand.size();++day){
            for(int clientId=0;clientId<clientNum;++clientId){
                v[day] += graph[day][serverId][clientId];
            }
        }
        server_load.push_back(v);
    }
    int total = 0;
    server_load_sorted = server_load;
    for(int serverId=0;serverId<serverNum;++serverId){
        sort(server_load_sorted[serverId].begin(),server_load_sorted[serverId].end());
        int p95 = server_load_sorted[serverId][int(demand.size()*0.95)];
        // cout<<p95<<" " ;
        total += p95;
        int avg = 0;
        for(int day=0;day<demand.size();++day){
            avg += server_load[serverId][day];
        }
        avg = avg/demand.size();

        server_load_avg.push_back(avg);
        // cout<<avg<<" ";
    }
    // cout<<"------------------"<<endl;
    // cout<<"total: "<<total<<endl;
}

bool IsEnough(int day){
    int s = 0, t = clientNum + serverNum + 1;
    int m = t;
    int n=0;
    vector<vector<int>> edges;
    for(int i=0;i<clientNum;i++){
        int node_id = i+1;
        vector<int> edge;
        edge.push_back(0);
        edge.push_back(node_id);
        edge.push_back(demand[day][i]);
        edges.push_back(edge);
    }
    for(int clientId=0;clientId<clientNum;++clientId){
        for(int serverId:client_list[clientId]){
            vector<int> edge;
            edge.push_back(clientId+1);
            edge.push_back(serverId+1+clientNum);
            edge.push_back(10000000);
            edges.push_back(edge);
        }
    }
    int begin = getFullServerIndex(day);
    int end = begin + int(serverNum*0.05);
    for(int serverId=0;serverId<serverNum;++serverId){
        vector<int> edge;
        // int val = (serverID_to_Val[serverId].second*SERVER_COST_RATE[serverId]);
        int val = SERVER_COST_RATE[serverId];
        if(FULL_SERVER_ID[day].count(serverId)){
            val = (serverID_to_Val[serverId].second);
        }
        edge.push_back(serverId+1+clientNum);
        edge.push_back(t);
        edge.push_back(val);
        edges.push_back(edge);
    }
    int max_stream = GetMaxStream(edges.size(), t, edges);
    int total_demand = 0;
    for(int clientId=0;clientId<clientNum;++clientId){
        total_demand += demand[day][clientId];
    }
    // cout<<max_stream<<","<<total_demand<<endl;
    return max_stream == total_demand;
    
}



void AdjustRate(int day){

    //同时增长所有节点的rate，直到满足需求
    bool isContinue=true;
    int count = 0;
    while(isContinue){
        count++;
        for(int i=0;i<serverNum;++i){
            SERVER_COST_RATE[i] += STEP;
        }
        if(IsEnough(day)){
            isContinue=false;
        }
    }
    //逐个降低每个节点的rate，直到不能再降低了
    for(int i=0;i<count;++i){
        for(int serverId=0;serverId<serverNum;++serverId){
            SERVER_COST_RATE[serverId] -= STEP;
            if(!IsEnough(day)){
                SERVER_COST_RATE[serverId] += STEP;
            }
            
        }
    }
    
}

vector<vector<vector<int>>>  GetAns(){
    vector<vector<vector<int>>> ans;
    for(int day=0;day<demand.size();++day){
        vector<vector<int>> ans_daily;
        for(int clientId=0;clientId<clientNum;++clientId){
            vector<int> v;
            for(int serverId=0;serverId<serverNum;++serverId){
                v.push_back(0);
            }
            ans_daily.push_back(v);
        }
        ans.push_back(ans_daily);
    }
    for(int day=0;day<demand.size();++day){
        int s = 0, t = clientNum + serverNum + 1;
        int m = t;
        int n=0;
        vector<vector<int>> edges;
        for(int i=0;i<clientNum;i++){
            int node_id = i+1;
            vector<int> edge;
            edge.push_back(0);
            edge.push_back(node_id);
            edge.push_back(demand[day][i]);
            edges.push_back(edge);
        }
        for(int clientId=0;clientId<clientNum;++clientId){
            for(int serverId:client_list[clientId]){
                vector<int> edge;
                edge.push_back(clientId+1);
                edge.push_back(serverId+1+clientNum);
                edge.push_back(10000000);
                edges.push_back(edge);
            }
        }
        int begin = getFullServerIndex(day);
        int end = begin + int(serverNum*0.05);
        for(int serverId=0;serverId<serverNum;++serverId){
            vector<int> edge;
            // int val = (serverID_to_Val[serverId].second*SERVER_COST_RATE[serverId]);
            int val =SERVER_COST_RATE[serverId];
            if(FULL_SERVER_ID[day].count(serverId)){
                val = (serverID_to_Val[serverId].second);
            }
            edge.push_back(serverId+1+clientNum);
            edge.push_back(t);
            edge.push_back(val);
            edges.push_back(edge);
        }
        auto route_today = FindRoute(edges.size(), t, edges);
        // cout<<"find route ok"<<endl;
        // cout<<route_today.size()<<endl;
        for(auto one_route:route_today){
            for(auto edge:one_route){
                int from = edge[0];
                int to = edge[1];
                int val = edge[2];
                if(from>0&&from<=clientNum&&to>clientNum&&to<=clientNum+serverNum){
                    ans[day][from-1][to-clientNum-1] += val;
                }
                if(to>0&&to<=clientNum&&from>clientNum&&from<=clientNum+serverNum){
                    ans[day][to-1][from-clientNum-1] -= val;
                }
            }
        }
    }
    return ans;
}

int CulServerLoad(int day, int serverId){
    int ret = 0;
    for(int clientId:server_list[serverId]){
        ret += demand[day][clientId];
    }
    return ret;
}

vector<int> CulServerLoadThreshold(){
    vector<vector<int>> server_load;
    vector<int> ret(serverNum, 0);
    for(int serverId=0;serverId<serverNum;++serverId){
        vector<int> one_server_load(demand.size(), 0);
        for(int day=0;day<demand.size();++day){
            int load = CulServerLoad(day, serverId);
            one_server_load[day] = load;
        }
        sort(one_server_load.begin(), one_server_load.end());
        int thre = one_server_load[ceil(demand.size()*0.95)-1];
        server_load.push_back(one_server_load);
        ret[serverId] = thre;
    }
    return ret;

}
void GetFullServerId(){
    /*
    auto thr = CulServerLoadThreshold();
    for(int day=0;day<demand.size();++day){
        unordered_set<int> s;
        FULL_SERVER_ID.push_back(s);
        for(int serverId=0;serverId<serverNum;++serverId){
            int load = CulServerLoad(day, serverId);
            if(load> thr[serverId]){
                FULL_SERVER_ID[day].insert(serverId);
            }
        }
    }
    */
   for(int day=0;day<demand.size();++day){
       unordered_set<int> s;
        FULL_SERVER_ID.push_back(s);
       int b = getFullServerIndex(day);
       int e = b + int(serverNum*0.05);
       for(int i=b;i<e;++i){
           FULL_SERVER_ID[day].insert(i);
       }
   }
}
void output(vector<vector<vector<int>>> ans){
    ofstream outfile(prefix+"output/solution.txt",ios::out);

    if(!outfile){
        cout<<"Can't open output file"<<endl;
        exit(1);
    }


    for(int day=0;day<demand.size();++day){
        for(int clientId=0;clientId<clientNum;++clientId){
            string clientName = clientID_to_Name[clientId];
            clientName.erase(std::remove(clientName.begin(), clientName.end(), '\r'), clientName.end());
            outfile<<clientName<<":";
            int cc = 0;
            for(int serverId=0;serverId<serverNum;++serverId){
                int val = ans[day][clientId][serverId];
                if(val !=0){
                    if(cc!=0){
                        outfile<<",";
                    }
                    cc++;
                    string serverName = serverID_to_Val[serverId].first;
                    outfile<<"<"<<serverName<<","<<val<<">";
                }
            }
            outfile<<endl;
        }
    }
    outfile.close();
}

int main() {
    init();
    AvgDistribute();
    CulculateServerLoadAVG();
    GetFullServerId();
    /*
    for(int day=0;day<demand.size();++day){
        for(auto it:FULL_SERVER_ID[day]){
            cout<<it<<" ";
        }
        cout<<endl;
    }
    */
    
    bool isContinue = true;
    
    for(int day=0;day<demand.size();++day){
        bool a = IsEnough(day); 
        if(!a){
            // RATE += STEP;
            AdjustRate(day);
            #ifdef Debug
                cout<<"day "<<day<<" fail"<<endl;
            #endif
            day--;
        }
    }
    auto ans = GetAns();
    output(ans);
    #ifdef Debug
    int total = 0;
        for(int serverId=0;serverId<serverNum;++serverId){
            int a = (SERVER_COST_RATE[serverId]);
            total += a;
        }
        cout<<"total cost is:"<<total<<endl;
    #endif
    
}