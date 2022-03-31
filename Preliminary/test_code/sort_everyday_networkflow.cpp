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
#include <random>

// #define Debug 1
// #define Debug1 1

using namespace std;
const string prefix = "";
int clientNum, serverNum, Times;
//客户节点存储的数据
unordered_map<string,int> clientName_to_ID;
vector<string> clientID_to_Name;
vector<vector<int>> demand; //demand[i][j]表示第i个时间节点，第j个客户节点的流量需求

//边缘节点存储的数据
unordered_map<string,int> serverName_to_ID;
vector<pair<string,int>> serverID_to_Val;

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

//输出函数
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

bool curDemandOver(vector<int>& curDemand){
    for(int& j:curDemand){
        if(j!=0){
            return false;
        }
    }
    return true;
}

void printCurDemand(vector<int>& curDemand){
    cout<<"current day demand:{";
    for(int i=0;i<clientNum;i++){
        if(curDemand[i]!=0){
            cout<<clientID_to_Name[i]<<":"<<curDemand[i]<<", ";
        }
    }
    cout<<"}"<<endl;
}

//将每一天按照总带宽需求进行排序
int sortDemands(vector<int>& sequence,vector<int>& dailyDemands){
    sort(sequence.begin(),sequence.end(),[&](const int& a,const int& b){
        return dailyDemands[a]>dailyDemands[b];
    });
    return sequence[0];
}

//先平均分一次
void average_distribution(vector<int>& serverMax){
    vector<vector<int>> serverTotal(serverNum,vector<int>(demand.size()));
    for(int day=0;day<Times;day++){
        //平均分
        vector<int> curDemand = demand[day];
        vector<int> serverLoad(serverNum);
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }
        while(!curDemandOver(curDemand)){
            int clientId = 0;
            for(int cId = 0;cId<clientNum;cId++){
                if(curDemand[cId]>0){
                    clientId = cId;
                    break;
                }
            }

            //平均分
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
            }
        }
        for(int i=0;i<serverNum;i++){
            serverTotal[i][day] = serverID_to_Val[i].second-serverLoad[i];
        }
    }

    for(int i=0;i<serverNum;i++){
        sort(serverTotal[i].begin(),serverTotal[i].end(),greater<int>());
        serverMax[i] = serverTotal[i][0];
    }
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

bool notEndLoop(vector<int>& leftvec,vector<int>& rightvec){
    for(int i=0;i<serverNum;i++){
        if(rightvec[i]-leftvec[i]>32){
            return true;
        }
    }
    return false;
}

void solve(vector<int>& serverMax){
    auto demand_to_use = demand;
    Network_Flow networkFlow;
    vector<int> serverTimes(serverNum,five_percent);
    vector<vector<int>> serverTotal(serverNum,vector<int>(Times));
    //最终记录的结果
    vector<vector<vector<pair<int,int>>>> ans(Times,vector<vector<pair<int,int>>>(clientNum));
    vector<int> serverSort;                                         //用于排序的数组
    serverSort.reserve(serverNum);
    vector<int> serverLoad_a(serverNum);
    for(int i=0;i<serverNum;i++){
        serverLoad_a[i] = serverID_to_Val[i].second;
    }
    vector<vector<int>> serverLoads(Times,serverLoad_a);
    serverLoad_a.clear();
    for(int i=0;i<serverNum;i++){
        serverSort.push_back(i);
    }
    vector<int> dailyDemands(Times,0);
    vector<unordered_set<int>> record(Times);
    vector<int> sequence;
    sequence.reserve(Times);
    for(int day = 0;day<Times;day++){
        sequence.push_back(day);
        for(int j = 0;j<clientNum;j++){
            dailyDemands[day]+=demand_to_use[day][j];
        }
    }

    while(true){
        int curDay = sortDemands(sequence,dailyDemands);    //先对天数进行排序
//        cout<<"current day is "<<curDay<<endl;
//        cout<<"Before using an opportunity, current day demand is "<<dailyDemands[curDay]<<endl;
        vector<int> serverCost(serverNum,0);
        vector<int>& curDemand = demand_to_use[curDay];
        vector<int>& serverLoad = serverLoads[curDay];
        auto &curAns = ans[curDay];
        if(curDemandOver(curDemand)){   //光靠用次数就分完了
            break;
        }
        for(int i=0;i<serverNum;i++){
            for(int& client:server_list[i]){
                serverCost[i]+=curDemand[client];
            }
        }
        sort(serverSort.begin(),serverSort.end(),[&](const int& a,const int& b){
            return serverCost[a]>serverCost[b];
        });

        int serverId = -1;
        for(int sId:serverSort){
            if(serverTimes[sId]>0&&serverLoad[sId]>0){
                serverId = sId;
                break;
            }
        }

        if(serverId==-1||serverCost[serverId]==0){
            break;
        }
        //用掉一次次数
        serverTimes[serverId]--;
        record[curDay].insert(serverId);
        int curNeed = 0;
        for(int& client:server_list[serverId]){
            curNeed+=curDemand[client];
        }
        int serGive = serverLoad[serverId];
        for(int& client:server_list[serverId]){
            if(serverLoad[serverId]==0){
                break;
            }
            double cL = (double)curDemand[client]/(double)curNeed;
            int minV = ceil(cL*serGive);
            minV = min(curDemand[client],minV);
            minV = min(serverLoad[serverId],minV);
            curDemand[client]-=minV;
            serverLoad[serverId]-=minV;
            dailyDemands[curDay]-=minV;
            curAns[client].push_back({serverId,minV});
        }
//        cout<<"Times of server be used: "<<serverID_to_Val[serverId].first<<" ,and its bandwidth is "<<serverID_to_Val[serverId].second<<endl;
//        cout<<"After using an opportunity, current day demand is "<<dailyDemands[curDay]<<endl;
//        system("pause");
    }

    for(int day = 0;day<Times;day++){
        vector<int>& curDemand = demand_to_use[day];
        vector<int>& serverLoad = serverLoads[day];
        auto &curAns = ans[day];
        if(dailyDemands[day]>0){
            vector<int> leftVec(serverNum),rightVec(serverNum);
            unordered_set<int> le;
            for(int i=0;i<clientNum;i++){
                if(curDemand[i]>0){
                    for(int& j:client_list[i]){
                        if(record[day].count(j)){
                            continue;
                        }
                        le.insert(j);
                    }
                }
            }
            for(int i=0;i<serverNum;i++){
                if(le.count(i)){
                    leftVec[i] = 0;
                    rightVec[i] = serverMax[i]-(serverID_to_Val[i].second-serverLoads[day][i]);
                }else{
                    leftVec[i] = rightVec[i] = 0;
                }
            }
            bool flag = false;
            while(notEndLoop(leftVec,rightVec)){
                networkFlow.init();
                vector<int> mid(serverNum);
                for(int i=0;i<serverNum;i++){
                    mid[i] = (leftVec[i]+rightVec[i])/2;
                }
                int res = network_flow_solve(networkFlow,curDemand,serverLoad,mid);
                if(res==dailyDemands[day]){
                    for(int i=0;i<serverNum;i++){
                        rightVec[i] = mid[i];
                    }
                    flag = true;
                }else{
                    for(int i=0;i<serverNum;i++){
                        leftVec[i] = mid[i]+1;
                    }
                    flag = false;
                }
            }
            if(!flag){
                int res = network_flow_solve(networkFlow,curDemand,serverLoad,rightVec);
            }
            //记录网络流的结果
            for(int i=0;i<clientNum;i++){
                for(int j=0;j<serverNum;j++){
                    if(networkFlow.ans[i][j]==0){
                        continue;
                    }
                    serverLoads[day][j]-=networkFlow.ans[i][j];
                    curAns[i].push_back({j,networkFlow.ans[i][j]});
                }
            }
        }
    }

    for(int ser = 0;ser<serverNum;ser++){
        for(int day = 0;day<Times;day++){
            serverTotal[ser][day] = serverID_to_Val[ser].second-serverLoads[day][ser];
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
#ifdef Debug
        for(int j : seri){
            cout<<j<<" ";
        }
        int k = 0;
        int count = 0;
        for(int l=0;l<demand.size()-1-five_percent;++l){
            k += seri[l];
            count ++;
        }
        cout<<"avg: "<<k/count<<endl;
        cout<<"final Cost:"<<seri[demand.size()-1-five_percent]<<endl;
        totalUse+=five_percent-serverTimes[i];
        cout<<"times Used:"<<five_percent-serverTimes[i]<<endl;
#endif
        last+=seri[demand.size()-1-five_percent];
    }
#ifdef Debug
    cout<<"Total Cost = "<<last<<endl;
    cout<<"Total Use = "<<totalUse<<endl;
#endif
    output(ans);
}

int main() {
//    clock_t startTime,endTime;
//    startTime = clock();
    default_random_engine engine;// 随机数生成引擎
    engine.seed(time(NULL)); //初始化种子

    //输入处理
    ProcessInput(); //数据的输入处理
    vector<int> serverMax(serverNum);
    average_distribution(serverMax);

    //随机的循环
    solve(serverMax);
//    endTime = clock();
//    cout << "The run time is: " <<(double)(endTime - startTime)*1000 / CLOCKS_PER_SEC << "ms" << endl;
    return 0;
}