//
// Created by war on 2022/3/31.
//

#ifndef QUATERFINAL_PROCESSINPUT_H
#define QUATERFINAL_PROCESSINPUT_H
#include "global.h"

inline void ProcessClients(istringstream& is,unordered_map<string,int>& clientName_to_ID){     //主要是处理clientName_to_ID和clientID_to_Name
    string stn;
    getline(is,stn,',');
    getline(is,stn,',');
    int i = 0;
    while(getline(is,stn,',')){
        clientName_to_ID[stn] = i;
        clientID_to_Name.push_back(stn);
        i++;
    }
}

inline vector<int> ProcessNames(istringstream& is,unordered_map<string,int>& clientName_to_ID){
    vector<int> ans;
    string stn;
    getline(is,stn,',');
    while(getline(is,stn,',')){
        ans.push_back(clientName_to_ID[stn]);
    }
    return ans;
}

void ProcessInput(){
    unordered_map<string,int> clientName_to_ID;
    unordered_map<string,int> serverName_to_ID;
    ifstream infile(prefix+"data/demand.csv",ios::in);
    if(!infile){
        cout<<"Can't open file"<<endl;
        exit(1);
    }
    string line;
    getline(infile,line);
    istringstream is(line);
    //第一行先处理clientID_to_Name和clienName_to_ID
    ProcessClients(is,clientName_to_ID);
    clientNum = clientID_to_Name.size();
//    for(auto &it:clientName_to_ID){
//        cout<<it.first<<" "<<it.second<<endl;
//    }
    //处理带宽值
    vector<unordered_map<string,int>> curTimeDemand(clientNum);
    string currentTime;
    while(getline(infile,line)){
        is.clear();
        is.str(line);
        string time,streamId,stn;
        getline(is,time,',');
        if(currentTime!=time){
            if(!currentTime.empty()){
                demand.push_back(curTimeDemand);
                curTimeDemand = vector<unordered_map<string,int>>(clientNum);
            }
            currentTime = time;
        }
        getline(is,streamId,',');
        int i = 0;
        while(getline(is,stn,',')){
            int stream_mount = stoi(stn);
            Max_Stream = max(stream_mount,Max_Stream);
            curTimeDemand[i++][streamId] = stream_mount;
        }
    }
    demand.push_back(curTimeDemand);        //最后一个时刻不能忘记导入

    five_percent = demand.size()/20;

//    for(auto& vec:demand){
//        for(int i = 0;i<vec.size();i++){
//            auto mp = vec[i];
//            cout<<clientID_to_Name[i]<<":[";
//            for(auto& it:mp){
//                cout<<it.first<<":"<<it.second<<", ";
//            }
//            cout<<"]"<<endl;
//        }
//        cout<<endl;
//        system("pause");
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


    serverNum = serverID_to_Val.size();
    Times = demand.size();
    server_list = vector<vector<int>>(serverNum);
    client_list = vector<vector<int>>(clientNum);
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
    getline(infile,line);
    is.clear();
    is.str(line);
    getline(is,qo,'=');
    getline(is,qo,'=');
    V = stoi(qo);



    infile.close();
    infile.open(prefix + "data/qos.csv");
    if(!infile){
        cout<<"Can't open file"<<endl;
        exit(1);
    }
    getline(infile,line);
    is.clear();
    is.str(line);
    vector<int> clientIDs = ProcessNames(is,clientName_to_ID);
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
            return serverID_to_Val[a].second/(double)server_list[a].size() > serverID_to_Val[b].second/(double)server_list[b].size();
            //return serverID_to_Val[a].second<serverID_to_Val[b].second;
        });
    }
}

void output(vector<vector<unordered_map<string,int>>>& ans){
    ofstream outfile(prefix+"output/solution.txt",ios::out);
    if(!outfile){
        cout<<"Can't open output file"<<endl;
        exit(1);
    }

    for(const auto& vec:ans){
        //第i个时刻的输出结果
        for(int i = 0;i<vec.size();i++){
            //v1代表一个客户节点的分配策略
            const auto& v1 = vec[i];
            clientID_to_Name[i].erase(std::remove(clientID_to_Name[i].begin(), clientID_to_Name[i].end(), '\r'), clientID_to_Name[i].end());
            outfile<<clientID_to_Name[i]<<":";  //customerID
            bool flag = false;
            unordered_map<int,vector<string>> curAns;   //按照边缘节点->流的分配
            for(auto& it:v1){
                if(!curAns.count(it.second)){
                    curAns[it.second] = vector<string>(0);
                    curAns[it.second].push_back(it.first);
                }else{
                    curAns[it.second].push_back(it.first);
                }
            }
            for(auto& item:curAns){
                if(flag){
                    outfile<<',';
                }
                outfile<<'<'<<serverID_to_Val[item.first].first;    //site_id
                for(auto& str:item.second){ //streamID
                    outfile<<","<<str;
                }
                outfile<<'>';
                flag = true;
            }
            outfile<<endl;
        }
    }
    outfile.close();
}

#endif //QUATERFINAL_PROCESSINPUT_H
