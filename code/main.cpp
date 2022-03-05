#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <deque>
#include <functional>
#include <memory>

using namespace std;
int N, M, T, R;

struct Server {
	friend ostream& operator<<(ostream& os, const Server& server);
    string name;
	int CPU, Mem, Cost, Loss;	//Cost是购买成本，Loss是能耗成本
	int ACPU, AMem, BCPU, BMem;
	Server() = default;
	Server(string Name,int CPU_, int Mem_, int Cost_, int Loss_) :
		name(Name),CPU(CPU_), Mem(Mem_), Cost(Cost_), Loss(Loss_) {
		ACPU = AMem = BCPU = BMem = 0;
	}

	bool isValid() const {
		return ACPU < CPU / 2 && BCPU < CPU / 2 && AMem < Mem / 2 && BMem < Mem / 2;
	}

	//A节点剩下多少CPU和内存
	pair<int,int> retA() const {
		return { ACPU,AMem };
	}

	pair<int, int> retB() const {
		return { BCPU,BMem };
	}

	pair<int, int> ret() const {
		return { CPU,Mem };
	}
};

ostream& operator<<(ostream& os, const Server& server) {
	os << server.name << ": [CPU:" << server.CPU << ", Mem:" << server.Mem << ", Cost:" << server.Cost << ", Loss:" << server.Loss<<"]";
	return os;
}

struct VirtualM {
    friend ostream& operator<<(ostream& os,const VirtualM& virtualM);
    string name;
	int CPU, Mem;
	bool flag;	//是否双节点部署
	VirtualM() = default;
 	VirtualM(string name_,int CPU_, int Mem_, int flag_) :
		name(std::move(name_)),CPU(CPU_), Mem(Mem_), flag(flag_ == 1) {
	}
};

ostream& operator<<(ostream& os,const VirtualM& virtualM){
    os<< virtualM.name <<" [CPU:" << virtualM.CPU << ", Mem:" << virtualM.Mem << ", flag: " << virtualM.flag<<"]";
    return os;
}

vector<Server> servers;
unordered_map<string,VirtualM> virtualMap;

int main()
{
	cin >> N;
	for (int i = 0; i < N; i++) {
        string name,cpu,mem,cost,loss;
		cin>>name>>cpu>>mem>>cost>>loss;
        Server server(name.substr(1,name.size()-2),stoi(cpu.substr(0,cpu.size()-1)),stoi(mem.substr(0,mem.size()-1))
        ,stoi(cost.substr(0,cost.size()-1)),stoi(loss.substr(0,loss.size()-1)));
        servers.push_back(server);
	}

//    for(auto &server:servers){
//        cout<<server<<endl;
//    }

    cin>>M;
    for(int i=0;i<M;i++){
        string name,cpu,mem,flag;
        cin>>name>>cpu>>mem>>flag;
        VirtualM virtualM(name.substr(1,name.size()-2),stoi(cpu.substr(0,cpu.size()-1)),stoi(mem.substr(0,mem.size()-1)),
        stoi(flag.substr(0,flag.size()-1)));
        virtualMap[name.substr(1,name.size()-2)] = virtualM;
    }

//    for(auto& virtualM:virtualMap){
//        cout<<virtualM.first<<" "<<virtualM.second<<endl;
//    }

    cin>>T;
    while(T--){
    }

	return 0;
}
