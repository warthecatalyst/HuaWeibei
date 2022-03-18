#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <memory>

using namespace std;

//客户节点存储的数据
unordered_map<string,int> clientName_to_ID;
vector<string> clientID_to_Name;
vector<vector<int>> demand; //demand[i][j]表示第i个时间节点，第j个客户节点的流量需求

//边缘节点存储的数据
unordered_map<string,int> serverName_to_ID;
vector<pair<string,int>> serverID_to_Val;

//用于判断客户节点到边缘节点是否能够进行通讯
vector<vector<bool>> client_server_connect;


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
    ifstream infile("data\\demand.csv",ios::in);
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

//    for(auto& vec:demand){
//        for(auto &i:vec){
//            cout<<i<<" ";
//        }
//        cout<<endl;
//    }

    infile.close();
    infile.open("data\\site_bandwidth.csv");
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

//    for(auto& vec:serverID_to_Val){
//        cout<<vec.first<<" "<<vec.second<<endl;
//    }

    infile.close();
    infile.open("data\\config.ini");
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
    infile.open("data\\qos.csv");
    if(!infile){
        cout<<"Can't open file"<<endl;
        exit(1);
    }
    getline(infile,line);
    client_server_connect = vector<vector<bool>>(clientID_to_Name.size(),vector<bool>(serverID_to_Val.size()));
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
            client_server_connect[j][serverId] = stoi(qos) < QOS;
            j++;
        }
    }
    for(auto& vec:client_server_connect){
        for(auto val:vec){
            cout<<val<<" ";
        }
        cout<<endl;
    }
    clientName_to_ID.clear();
    serverName_to_ID.clear();
}

//将每个时间点按照所有客户的请求之和排序
vector<int> sortDemands(){
    vector<pair<int,int>> besorted;
    for(int i=0;i<demand.size();i++){
        int curNeed = 0;
        for(int& j:demand[i]){
            curNeed+=j;
        }
        besorted.emplace_back(i,curNeed);
    }

    sort(besorted.begin(),besorted.end(),)
}

int main() {
    ProcessInput(); //数据的输入处理

    return 0;
}
