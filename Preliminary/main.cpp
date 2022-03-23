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
//#define Debug1 1

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
vector<vector<double>> server_link;

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
    server_list = vector<vector<int>>(serverNum);
    client_list = vector<vector<int>>(clientNum);
    server_link = vector<vector<double>>(serverNum,vector<double>(serverNum));

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

    //找到两个边缘节点之间的连接性
    for(int s = 0;s<serverNum;s++){
        vector<int>& slist = server_list[s];    //A
        for(int t = s+1;t<serverNum;t++){
            vector<int>& tlist = server_list[t];    //B
            int ans = 0;
            for(int& client:tlist){
                if(find(slist.begin(), slist.end(), client)!=slist.end()){
                    ans++;
                }
            }
            if(tlist.size()==0){
                server_link[s][t] = 0;
            }else{
                server_link[s][t] = (double)ans/(double)tlist.size();
            }
            if(slist.size()==0){
                server_link[t][s] = 0;
            }else{
                server_link[t][s] = (double)ans/(double)slist.size();
            }
        }
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

//将所有的时间节点按照当时的消耗进行排序，得到处理的顺序
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

void printCurDemand(vector<int>& curDemand){
    cout<<"current day demand:{";
    for(int i=0;i<clientNum;i++){
        if(curDemand[i]!=0){
            cout<<clientID_to_Name[i]<<":"<<curDemand[i]<<", ";
        }
    }
    cout<<"}"<<endl;
}

void printServerStatus(int serverId, vector<int>& serverLoad,vector<int>& curDemand){
    cout<<serverID_to_Val[serverId].first<<" : "<<serverLoad[serverId]<<endl;
    for(int& client:server_list[serverId]){
        cout<<clientID_to_Name[client]<<":"<<curDemand[client]<<", ";
    }
    cout<<endl;
}

//record用于记录边缘节点在随机分配中所使用的带宽排序
//record[i]为第i个边缘节点的整体排序情况,record[i][day]为第day天的消耗在当日所排的百分比（从小到大）
void solve(const vector<int>& sequence,vector<vector<int>>& serverTotal,vector<vector<double>>& record){   //用于求解第一轮的结果
    vector<vector<vector<pair<int,int>>>> ans(demand.size());              //最终记录的结果，目前可能不需要，但先暂时保存着
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    vector<int> serverSort;                                         //用于排序的数组
    serverSort.reserve(serverNum);
    for(int i=0;i<serverID_to_Val.size();i++){
        serverSort.push_back(i);
    }

    for(const int& day:sequence){
#ifdef Debug1
        cout<<"current day is "<<day<<endl;
#endif
        vector<int> curDemand = demand[day];
        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的负载
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }

        while(!curDemandOver(curDemand)){
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
        }
        ans[day] = curAns;
        for(int i=0;i<serverNum;i++){
            serverTotal[i][day] = serverID_to_Val[i].second-serverLoad[i];
        }
#ifdef Debug1
        //system("pause");
#endif
    }

    //计算最终结果
    int last = 0,totalUse = 0;
    for(int i=0;i<serverNum;i++){
#ifdef Debug
        cout<<serverID_to_Val[i].first<<":";
#endif
        vector<pair<int,int>> seri;
        for(int j = 0;j<serverTotal[i].size();j++){
            seri.emplace_back(serverTotal[i][j],j);
        }
        sort(seri.begin(),seri.end(),[](const pair<int,int>& a,const pair<int,int>& b){
            return a.first<b.first;
        });  //从小到大排序

        for(int j = 0;j<seri.size();j++){
            record[i][seri[j].second] = (double)(j+1)/(double)demand.size();
        }

#ifdef Debug
//        for(double j : record[i]){
//            cout<<j<<" ";
//        }
//        cout<<endl;
        for(auto& j : seri){
            cout<<j.first<<" ";
        }
        cout<<"final Cost:"<<seri[demand.size()-1-five_percent].first<<"\t";
        totalUse+=five_percent-serverTimes[i];
        cout<<"times Used:"<<five_percent-serverTimes[i]<<endl;
        last+=seri[demand.size()-1-five_percent].first;
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

void furtherImprovement(const vector<int>& sequence,vector<vector<int>>& serverTotal,vector<vector<double>>& record){    //之后的进步轮次
    vector<vector<vector<pair<int,int>>>> ans(demand.size());              //最终记录的结果
    vector<int> serverTimes(serverNum,five_percent);       //表示边缘节点还剩下多少次机会
    vector<int> serverSort;
    serverSort.reserve(serverNum);
    for(int i=0;i<serverNum;i++){
        serverSort.push_back(i);
    }

    for(const int& day:sequence){   //同样按照天数进行处理

        vector<int> curDemand = demand[day];
#ifdef Debug1
        cout<<"current day is "<<day<<endl;
#endif
        vector<vector<pair<int,int>>> curAns(clientNum);  //当前这轮的结果
        vector<int> serverLoad(serverNum);     //这轮所剩的负载
        for(int i=0;i<serverNum;i++){
            serverLoad[i] = serverID_to_Val[i].second;
        }
        unordered_set<int> rec;
        while(!curDemandOver(curDemand)){
#ifdef Debug1
            printCurDemand(curDemand);
#endif
            vector<double> serverCosts(serverNum,0.0);
            for(int i=0;i<serverNum;i++){
                if(rec.count(i)){
                    continue;
                }
                for(int j = 0;j<serverNum;j++){
                    if(rec.count(j)){
                        continue;
                    }
                    serverCosts[i]+=server_link[i][j]*record[j][day];
                }
            }

            sort(serverSort.begin(),serverSort.end(),[&](const int& a,const int& b){
                return serverCosts[a]>serverCosts[b];
            });
            int serverId = -1;
            for(int sId:serverSort){
                if(serverTimes[sId]>0&&serverLoad[sId]>0){
                    if(serverTimes[sId]>0&&serverLoad[sId]>0){
                        serverId = sId;
                        break;
                    }
                }
            }
            if(serverId!=-1&&serverCosts[serverId]>20){
                serverTimes[serverId]--;
                rec.insert(serverId);
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
                //调试时输出
//                cout<<"log1"<<endl;
                //说明已经没有节点可以使用这次次数了，直接进行平均分配法
                int clientId = 0;
                for (int i = 0; i < clientNum; i++) {
                    if (curDemand[i] > 0) {
                        clientId = i;
                        break;
                    }
                }
//                cout<<"clientID :"<<clientID_to_Name[clientId]<<" ,its demands: "<<curDemand[clientId] << " ,and it's neighbours: "<<endl;
                int neighLoad = 0;
                for (int &neigh: client_list[clientId]) {
                    neighLoad += serverLoad[neigh];
//                    cout<<serverID_to_Val[neigh].first<<" : "<<serverLoad[neigh]<<endl;
                }
//                cout<<endl;

                double curN = curDemand[clientId];
                for (int i = 0; i < client_list[clientId].size() && curDemand[clientId] > 0; i++) {
                    int neigh = client_list[clientId][i];
                    if (serverLoad[neigh] == 0) {
                        continue;
                    }
                    double cL = (double) serverLoad[neigh] / (double) neighLoad;
                    int cuL = ceil(cL * (double) curN);    //向上取整
                    if (cuL > curDemand[clientId]) {
                        cuL = curDemand[clientId];
                    }
                    if (cuL > serverLoad[neigh]) {
                        cuL = serverLoad[neigh];
                    }
                    curDemand[clientId] -= cuL;
                    serverLoad[neigh] -= cuL;
                    curAns[clientId].push_back({neigh, cuL});
                }
                continue;
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
#ifdef Debug
        for(int j : seri){
            cout<<j<<" ";
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
//    if(isLastRound){
//        output(ans);
//    }
}




int main() {
//    clock_t startTime,endTime;
//    startTime = clock();
    const int MAXRound = 30;
    ProcessInput(); //数据的输入处理
    vector<int> sequence = sortDemands();   //将每一天按照当天的请求总和排序，形成结果sequence
    vector<vector<double>> record(serverNum,vector<double>(demand.size()));
    vector<vector<int>> serverTotal(serverNum,vector<int>(demand.size()));
    int round = 0;
#ifdef Debug
    cout<<"current round is "<<round<<endl;
#endif
    solve(sequence,serverTotal,record);
    round++;
    furtherImprovement(sequence,serverTotal,record);
//    for(;round<MAXRound;round++){
//#ifdef Debug
//        cout<<"current round is "<<round<<endl;
//#endif
//        if(round==MAXRound-1){
//            furtherImprovement(sequence,serverTotal,record, round%2==1,true);
//        }else{
//            furtherImprovement(sequence,serverTotal,record,round%2==1);
//        }
//    }

//    endTime = clock();
//    cout << "The run time is: " <<(double)(endTime - startTime)*1000 / CLOCKS_PER_SEC << "ms" << endl;
    return 0;
}