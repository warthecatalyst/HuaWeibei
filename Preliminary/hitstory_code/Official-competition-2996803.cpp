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
#include <stack>

//#define Debug 1

using namespace std;
const string prefix = "/";
int clientNum, serverNum, Times;
//客户节点存储的数据
unordered_map<string,int> clientName_to_ID;
vector<string> clientID_to_Name;
vector<vector<int>> demand; //demand[i][j]表示第i个时间节点，第j个客户节点的流量需求

//边缘节点存储的数据
unordered_map<string,int> serverName_to_ID;
vector<pair<string,int>> serverID_to_Val;
vector<int> server_95per;  //上一轮迭代得到的95百分位带宽

vector<vector<int>> server_list;    //边缘节点的邻接链表
vector<vector<int>> client_list;    //客户节点的邻接链表，现在可能暂时用不到，之后优化时候再考虑

int five_percent;

//网络流类代码
struct Network_Flow{
    const int INF = 0x3f3f3f3f;
    struct Edge{
        int u,v;    //from,to
        int cap,flow;
        Edge(int x,int y,int z,int w){
            u = x;v=y;
            cap = z;flow = w;
        }
    };
    int start,end;
    vector<Edge> edges;
    vector<vector<int>> G;
    vector<bool> vis;
    vector<int> d,cur;
    vector<vector<int>> ans;
    vector<Edge> edges_result;

    void init(int s = 0,int t = clientNum+serverNum+1){
        start = s;end = t;
        G = vector<vector<int>>(end+1);
        vis = vector<bool>(end+1, false);
        d = vector<int>(end+1);
        cur = vector<int>(end+1);
        ans = vector<vector<int>>(clientNum,vector<int>(serverNum,0));
        edges.clear();
        edges_result.clear();
    }

    void addEdge(int x,int y,int z){
        edges.emplace_back(x,y,z,0);
        edges.emplace_back(y,x,0,0);
        int m = edges.size();
        G[x].push_back(m-2);    //链式前向星
        G[y].push_back(m-1);
    }

    inline bool isClient(int nodeId){
        return nodeId>=1&&nodeId<=clientNum;
    }

    inline bool isServer(int nodeId){
        return nodeId>clientNum&&nodeId<=serverNum+clientNum;
    }

    pair<int,int> contains(Edge& e){
        for(int i=0;i<edges_result.size();i++){
            if(edges_result[i].u==e.u&&edges_result[i].v == e.v){
                return {i,0};
            }else if(edges_result[i].v==e.u&&edges_result[i].u == e.v){
                return {i,1};
            }
        }
        return {-1,0};
    }

    bool BFS(){
        vis = vector<bool>(end+1, false);
        queue<int> que;
        que.push(start);
        d[start]=0;
        vis[start]= true;
        while(!que.empty()){
            int x=que.front();que.pop();
            for(int i=0;i<G[x].size();++i){
                Edge& e=edges[G[x][i]];
                if(!vis[e.v] && e.cap>e.flow){
                    vis[e.v]= true;
                    d[e.v]=d[x]+1;
                    que.push(e.v);
                }
            }
        }
        return vis[end];
    }

    int DFS(int x,int a){
        if(x==end || a==0) return a;
        int flow=0,f;
        for(int& i=cur[x];i<G[x].size();++i){
            Edge& e=edges[G[x][i]];
            if(d[x]+1==d[e.v] && (f=DFS(e.v,min(a,e.cap-e.flow)))>0){
                e.flow+=f;
                //如果是正向边，那么在答案中+f，如果是反向边，那么在答案中-f
                if(isClient(e.u)&& isServer(e.v)){  //如果e.u是client节点,e.v是server节点
                    ans[e.u-1][e.v-clientNum-1]+=f;
                }else if(isServer(e.u)&& isClient(e.v)){    //如果e.u是server节点,e.v是client节点
                    ans[e.v-1][e.u-clientNum-1]-=f;
                }
                edges[G[x][i]^1].flow-=f;
                flow+=f;
                a-=f;
                if(a==0) break;
            }
        }
        return flow;
    }

    int Dinic(){
        int flow=0;
        while(BFS()){
            cur = vector<int>(end+1);
            flow+=DFS(start,INF);
        }
        return flow;
    }

    void printResult(bool flag){
        if(flag){
            for(auto& edge:edges_result){
                cout<<edge.u<<"->"<<edge.v<<" : "<<edge.flow<<endl;
            }
        }else{
            for(auto& edge:edges){
                cout<<edge.u<<"->"<<edge.v<<" : "<<edge.flow<<endl;
            }
        }
    }
};

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
    Times = demand.size();
    server_list = vector<vector<int>>(serverNum);
    client_list = vector<vector<int>>(clientNum);
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
int sortDemands(vector<int>& dailyDemands,vector<int>& sequence){
    sort(sequence.begin(),sequence.end(),[&](const int& a,const int& b){
        return dailyDemands[a]>dailyDemands[b];
    });
    return sequence[0];
}

bool curDemandOver(vector<int>& curDemand){
    for(const int& d:curDemand){
        if(d>0){
            return false;
        }
    }
    return true;
}


void solve(vector<vector<int>>& record,vector<vector<int>>& serverTotal){   //用于求解第一轮的结果
    vector<vector<vector<pair<int,int>>>> ans(demand.size(),vector<vector<pair<int,int>>>(clientNum));  //最终记录的结果
    vector<int> serverTimes(serverNum,five_percent);
    auto demands_to_use = demand;
    vector<int> serverLoad_a(serverNum);
    for(int i=0;i<serverNum;i++){
        serverLoad_a[i] = serverID_to_Val[i].second;
    }
    vector<vector<int>> serverLoads(Times,serverLoad_a);
    serverLoad_a.clear();
    vector<int> serverSort;
    serverSort.reserve(serverNum);
    for(int i = 0;i<serverNum;i++){
        serverSort.push_back(i);
    }

    //先将边缘节点按照其带宽排序
    sort(serverSort.begin(),serverSort.end(),[&](const int& a,const int &b){
        return (double)serverID_to_Val[a].second/(double)server_list[a].size()>
        (double)serverID_to_Val[b].second/(double)server_list[b].size();
    });
    bool flag = false;
    for(int sId:serverSort){
        //对该节点每天的需求排序
        vector<pair<int,int>> dailyDemands; //每一天以及那一天其周围节点的带宽需求
        for(int day = 0;day<Times;day++){
            int curNeed = 0;
            for(int& client:server_list[sId]){
                curNeed+=demands_to_use[day][client];
            }
            dailyDemands.emplace_back(day,curNeed);
        }
        sort(dailyDemands.begin(),dailyDemands.end(),[](const pair<int,int>& a,const pair<int,int>& b){
            return a.second>b.second;
        });

        for(int t = 0;t<five_percent;t++){
            auto& p = dailyDemands[t];
            if(p.second == 0){
                flag = true;
                break;
            }
            vector<int>& curDemand = demands_to_use[p.first];
            vector<int>& serverLoad = serverLoads[p.first];
            auto& curAns = ans[p.first];

            serverTimes[sId]--;
            record[p.first].push_back(sId);
            int serGive = serverLoad[sId];
            for(int& client:server_list[sId]){
                if(serverLoad[sId]==0||curDemand[client]==0){
                    break;
                }
                double cL = (double)curDemand[client]/(double)p.second;
                int minV = ceil(cL*serGive);
                minV = min(curDemand[client],minV);
                minV = min(serverLoad[sId],minV);
                curDemand[client]-=minV;
                serverLoad[sId]-=minV;
                curAns[client].push_back({sId,minV});
            }
        }
    }

    for(int day = 0;day<Times;day++){
        vector<int>& curDemand = demands_to_use[day];
        vector<int>& serverLoad = serverLoads[day];
        auto& curAns = ans[day];
        while(!curDemandOver(curDemand)){
            //平均分
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
        }
    }

    for(int ser = 0;ser<serverNum;ser++){
        for(int day = 0;day<Times;day++){
            serverTotal[ser][day] = serverID_to_Val[ser].second-serverLoads[day][ser];
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
    // output(ans);
}

int lastRoundPosition(vector<vector<int>>& serverTotal,int serverId,int day){
    vector<int> seri = serverTotal[serverId];
    sort(seri.begin(),seri.end());
    int pos = std::find(seri.begin(), seri.end(), serverTotal[serverId][day])-seri.begin()+1;
    return pos;
    // double cL = 1/ sqrt((double)pos/(double)seri.size());
    // return cL;
}
/*
void furtherImprovement(vector<vector<int>>& serverTotal,vector<vector<int>>& record,bool isLastRound = false){    //之后的进步轮次
    vector<vector<vector<pair<int,int>>>> ans(demand.size());              //最终记录的结果
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    for(int day=0;day<demand.size();++day){
        for(int sId:record[day]){
            serverTimes[sId]--;
        }
    }
    // vector<int> server_max_use(server_95per);
    vector<int> server_pre_use(serverNum);  //每个边缘节点预先分配出去的额度
    int pre_use_times = 10; //预先分配的额度分十次使用
    for(int i=0;i<serverNum;i++){
        auto seri = serverTotal[i];
        sort(seri.begin(),seri.end());
        // server_pre_use[i] = seri[seri.size()*0.5-1];
        server_pre_use[i] = server_95per[i]*0.9;

    }
    for(int day=0;day<demand.size();++day){   //同样按照天数进行处理
        vector<int> server_max_use_today(serverNum, 0); //今天每个边缘节点用出去多少流量
        vector<int> curDemand = demand[day];            //今天每个客户节点的需求
        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的流量
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
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
        map<int, int> pre_use_round;
        int round = pre_use_times;
        for(int i=0;i<serverNum;++i){
            pre_use_round[i] = pre_use_times;
        }
        for(auto r:record[day]){
            pre_use_round[r] = 0;
        }
        //再查看是否有上一轮没有使用但是大于P95的节点，全部存入prepare，之后使用
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
            });
            if(!prepare.empty()&&serverTimes[prepare.back()]>0&&server_Cost[prepare.back()]>server_95per[prepare.back()]){   //在还能使用次数的情况下优先使用次数
                int serverId = prepare.back();
                serverTimes[serverId]--;
                record[day].push_back(serverId);
                rec.insert(serverId);
                pre_use_round[serverId] = 0;    //用过次数的节点不会再进行预分配
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
                int serverId = -1;
                //找到一个还可以预分配出流量的边缘节点
                for(int i = 0;i<serverNum;i++){
                    int index = i;
                    if(pre_use_round[index]!=round){
                        continue;
                    }
                    for(int client:server_list[index]){
                        if(curDemand[client]>0){
                            serverId = index;
                            break;
                        }
                    }
                    if(serverId==index){
                        break;
                    }
                }

                if(serverId==-1){
                    if(round>1){    //预分配未结束，就进行下一轮预分配
                        round --;
                        continue;
                    }
                    //预分配已经结束，处理还没被满足的客户节点
                    int clientId = 0;
                    for(int i=0;i<clientNum;i++){
                        if(curDemand[i]>0){
                            clientId = i;
                            break;
                        }
                    }
                    //先把未被满足的客户分给没把预分配配额用完的节点。
                    for(int ser:client_list[clientId]){
                        if(server_max_use_today[ser]<server_pre_use[ser]){
                            int val = min(curDemand[clientId], server_pre_use[ser]-server_max_use_today[ser]);
                            curDemand[clientId] -= val;
                            serverLoad[ser] -= val;
                            server_max_use_today[ser] += val;
                            curAns[clientId].push_back({ser,val});
                        }
                    }
                    if(curDemand[clientId]==0){
                        continue;
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
                    pre_use_round[serverId]--;
                    int MaxGive = server_pre_use[serverId]/pre_use_times;     
                    //将Maxgive按照需求平均分配给他的客户节点
                    int clientNeed = 0;
                    for(int& client:server_list[serverId]){
                        clientNeed += curDemand[client];
                    }
                    int serLeft = MaxGive;
                    for(int& client:server_list[serverId]){
                        double cL = (double)curDemand[client]/(double)clientNeed;
                        int cuL = ceil(cL*(double)MaxGive);
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
            if(server_pre_use[i]< server_max_use_today[i]){
                server_pre_use[i] = server_max_use_today[i];
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
}
*/

bool notEndLoop(vector<int>& leftvec,vector<int>& rightvec){
    for(int i=0;i<serverNum;i++){
        if(rightvec[i]-leftvec[i]>32){
            return true;
        }
    }
    return false;
}


void furtherImprovement(vector<vector<int>>& serverTotal,vector<vector<int>>& record,bool isLastRound = false){    //之后的进步轮次
    vector<vector<vector<pair<int,int>>>> ans(demand.size());              //最终记录的结果
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    for(int day=0;day<demand.size();++day){
        for(int sId:record[day]){
            serverTimes[sId]--;
        }
    }
    // vector<int> server_max_use(server_95per);
    vector<int> server_pre_use(serverNum);  //每个边缘节点预先分配出去的额度
    int pre_use_times = 30; //预先分配的额度分十次使用
    for(int i=0;i<serverNum;i++){
        auto seri = serverTotal[i];
        sort(seri.begin(),seri.end());
        // server_pre_use[i] = seri[seri.size()*0.5-1];
        server_pre_use[i] = server_95per[i]*0.96;//(0.9: 3008258 0.93:3007814   0.96：2996803 0.965：2997570 0.99: 3127202)

    }
    for(int day=0;day<demand.size();++day){   //同样按照天数进行处理
        vector<int> server_max_use_today(serverNum, 0); //今天每个边缘节点用出去多少流量
        vector<int> curDemand = demand[day];            //今天每个客户节点的需求
        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的流量
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }
        //把上一轮已经用过的先用了
        for(int serverId:record[day]){
            
            int serGive = serverLoad[serverId];
            int t = 0;
            for(int& client:server_list[serverId]){
                t += curDemand[client];
            }

            for(int& client:server_list[serverId]){
                if(serverLoad[serverId]==0||curDemand[client]==0){
                    break;
                }

                double cL = (double)curDemand[client]/(double)t;
                int minV = ceil(cL*serGive);
                minV = min(curDemand[client],minV);
                minV = min(serverLoad[serverId],minV);
                curDemand[client]-=minV;
                serverLoad[serverId]-=minV;
                curAns[client].push_back({serverId,minV});
            }
        
        }
        unordered_set<int> rec(record[day].begin(),record[day].end());  //在该日已经使用过次数的服务器
        map<int, int> pre_use_round;
        int round = pre_use_times;
        for(int i=0;i<serverNum;++i){
            pre_use_round[i] = pre_use_times;
        }
        for(auto r:record[day]){
            pre_use_round[r] = 0;
        }
        //再查看是否有上一轮没有使用但是大于P95的节点，全部存入prepare，之后使用
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
            });
            if(!prepare.empty()&&serverTimes[prepare.back()]>0&&server_Cost[prepare.back()]>server_95per[prepare.back()]){   //在还能使用次数的情况下优先使用次数
                int serverId = prepare.back();
                serverTimes[serverId]--;
                record[day].push_back(serverId);
                rec.insert(serverId);
                pre_use_round[serverId] = 0;    //用过次数的节点不会再进行预分配
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
                int serverId = -1;
                //找到一个还可以预分配出流量的边缘节点
                for(int i = 0;i<serverNum;i++){
                    int index = i;
                    if(pre_use_round[index]!=round){
                        continue;
                    }
                    for(int client:server_list[index]){
                        if(curDemand[client]>0){
                            serverId = index;
                            break;
                        }
                    }
                    if(serverId==index){
                        break;
                    }
                }

                if(serverId==-1){
                    if(round>1){    //预分配未结束，就进行下一轮预分配
                        round --;
                        continue;
                    }
                    //预分配已经结束，处理还没被满足的客户节点
                    int clientId = 0;
                    for(int i=0;i<clientNum;i++){
                        if(curDemand[i]>0){
                            clientId = i;
                            break;
                        }
                    }
                    //先把未被满足的客户分给没把预分配配额用完的节点。
                    
                    for(int ser:client_list[clientId]){
                        if(server_max_use_today[ser]<server_pre_use[ser]){
                            int val = min(curDemand[clientId], server_pre_use[ser]-server_max_use_today[ser]);
                            val = min(val, serverLoad[ser]);
                            curDemand[clientId] -= val;
                            serverLoad[ser] -= val;
                            server_max_use_today[ser] += val;
                            curAns[clientId].push_back({ser,val});
                        }
                    }
                    
                    if(curDemand[clientId]==0){
                        continue;
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
                    pre_use_round[serverId]--;
                    int MaxGive = server_pre_use[serverId]/pre_use_times;     
                    //将Maxgive按照需求平均分配给他的客户节点
                    int clientNeed = 0;
                    for(int& client:server_list[serverId]){
                        clientNeed += curDemand[client];
                    }
                    int serLeft = MaxGive;
                    for(int& client:server_list[serverId]){
                        double cL = (double)curDemand[client]/(double)clientNeed;
                        int cuL = ceil(cL*(double)MaxGive);
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
            if(server_pre_use[i]< server_max_use_today[i]){
                server_pre_use[i] = server_max_use_today[i];
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
    if(isLastRound)
        output(ans);
#ifdef Debug
    cout<<"Total Cost = "<<last<<endl;
    cout<<"Total Use = "<<totalUse<<endl;
#endif
}


inline int network_flow_solve(Network_Flow& networkFlow,vector<int>& curDemand,vector<int>& serverLoad,vector<int>& mid){
    networkFlow.init();
    //添加边，首先是从源点到客户节点，以及从客户节点到边缘节点
    for(int i=0;i<clientNum;i++){
        networkFlow.addEdge(0,i+1,curDemand[i]);
        for(int& ser:client_list[i]){
            networkFlow.addEdge(i+1,ser+clientNum+1,networkFlow.INF);   //该边设置为正无穷
        }
    }
    //边缘节点到目标节点
    for(int i=0;i<serverNum;i++){
        networkFlow.addEdge(i+clientNum+1,clientNum+serverNum+1,mid[i]);
    }
    return networkFlow.Dinic();
}

void networkflow_improvement(vector<vector<int>>& record){
    Network_Flow networkFlow;
    vector<vector<vector<pair<int,int>>>> ans(demand.size(),vector<vector<pair<int,int>>>(clientNum));              //最终记录的结果
    vector<vector<int>> serverTotal(serverNum,vector<int>(Times));
    vector<int> serverTimes(serverNum,five_percent);
    for(int day = 0;day<Times;day++) {
        auto& curAns = ans[day];
        vector<int> serverLoad(serverNum);     //这轮所剩的负载
        vector<int>& curDemand = demand[day];
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }

        //把上一轮已经用过的先用了
        for(int serverId:record[day]){
            serverTimes[serverId]--;
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

        //上一轮的还没用完这一轮再用
        if(!curDemandOver(curDemand)){
            int sum = 0;
            unordered_set<int> rec(record[day].begin(),record[day].end());
            for(int& i:curDemand){
                sum+=i;
            }
            unordered_set<int> le;
            vector<int> leftvec(serverNum),rightvec(serverNum);
            for(int i=0;i<clientNum;i++){
                if(curDemand[i]>0){
                    for(int &j:client_list[i]){
                        if(rec.count(j)){
                            continue;
                        }
                        le.insert(j);
                    }
                }
            }
            for(int i=0;i<serverNum;i++){
                if(le.count(i)){
                    leftvec[i] = 0;
                    rightvec[i] = server_95per[i];
                }else{
                    leftvec[i] = rightvec[i] = 0;
                }
            }
            bool flag = false;
            while(notEndLoop(leftvec,rightvec)){
                vector<int> mid(serverNum);
                for(int i=0;i<serverNum;i++){
                    mid[i] = (leftvec[i]+rightvec[i])/2;
                }

                int res = network_flow_solve(networkFlow,curDemand,serverLoad,mid);
                if(res==sum){   //mid可以满足
                    rightvec.assign(mid.begin(),mid.end());
                    flag = true;
                }else{  //mid不能满足
                    for(int i=0;i<serverNum;i++){
                        leftvec[i] = mid[i]+1;
                    }
                    flag = false;
                }
            }
            if(!flag){
                int res = network_flow_solve(networkFlow,curDemand,serverLoad,rightvec);
            }

            for(int i=0;i<clientNum;i++){
                for(int j=0;j<serverNum;j++){
                    if(networkFlow.ans[i][j]==0){
                        continue;
                    }
                    curAns[i].push_back({j,networkFlow.ans[i][j]});
                    serverLoad[j]-=networkFlow.ans[i][j];
                }
            }
        }
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
        vector<int>& seri = serverTotal[i];
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
    // output(ans);
}

int main() {
//    clock_t startTime,endTime;
//    startTime = clock();
    const int MAXRound = 30;
    ProcessInput(); //数据的输入处理
    vector<vector<int>> record(demand.size());
    vector<vector<int>> serverTotal(serverNum,vector<int>(demand.size()));
    int round = 0;
#ifdef Debug
    cout<<"current round is "<<round<<endl;
#endif
    solve(record,serverTotal);
    int roundTime = 25;
    for(int i=0;i<roundTime;++i){
        furtherImprovement(serverTotal, record);
    }
    furtherImprovement(serverTotal, record, true);
//    round++;
//    for(;round<MAXRound;round++){
//#ifdef Debug
//        cout<<"current round is "<<round<<endl;
//#endif
////        if(round==MAXRound-1){
////            furtherImprovement(sequence,serverTotal,record, round%2==1, false);
////        }else{
////            furtherImprovement(sequence,serverTotal,record,round%2==1);
////        }
//    }
//
//    networkflow_improvement(record);


//    endTime = clock();
//    cout << "The run time is: " <<(double)(endTime - startTime)*1000 / CLOCKS_PER_SEC << "ms" << endl;
    return 0;
}