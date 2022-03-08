#include <iostream>
#include <utility>
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
#include <cmath>

using namespace std;
int CPUMax = -1,MemMax = -1;       //使用最多CPU核以及使用最多内存

struct Server {
	friend ostream& operator<<(ostream& os, const Server& server);
    const string name;
	const int CPU, Mem, Cost, Loss;	//Cost是购买成本，Loss是能耗成本
	int ACPU, AMem, BCPU, BMem; //A节点还剩多少CPU和Mem,以及B节点还剩下多少CPU和Mem
	Server() = delete;
	Server(string Name,int CPU_, int Mem_, int Cost_, int Loss_) :
		name(std::move(Name)),CPU(CPU_), Mem(Mem_), Cost(Cost_), Loss(Loss_) {
		ACPU = BCPU = CPU/2;
        AMem = BMem = Mem/2;
	}

    Server(const Server& server):name(server.name),CPU(server.CPU),Mem(server.Mem),Cost(server.Cost),Loss(server.Loss){
        ACPU = server.ACPU;
        BCPU = server.BCPU;
        AMem = server.AMem;
        BMem = server.BMem;
    }

    bool operator<(const Server& server) const{
        return Cost<server.Cost;
    }

    //单双节点判断是否能够部署
    bool canDeploy(int CPUNeed,int MemNeed, bool flag) const {//flag代表是否双节点插入
        if(flag){   //如果双节点插入
            return ACPU>=CPUNeed/2&&AMem>=MemNeed/2&&BCPU>=CPUNeed/2&&BMem>=MemNeed/2;
        }else{      //单节点插入
            return ACPU>=CPUNeed&&AMem>=MemNeed||BCPU>=CPUNeed&&BMem>=MemNeed;
        }
    }

    //单节点判断部署在哪个节点并执行部署操作,双节点就直接部署并返回'C'
    char deploy(int CPUNeed,int MemNeed,bool flag){
        if(flag){
            ACPU-=CPUNeed/2;
            BCPU-=CPUNeed/2;
            AMem-=MemNeed/2;
            BMem-=MemNeed/2;
            return 'C';
        }
        if(ACPU>=BCPU&&AMem>=BMem){
            ACPU-=CPUNeed;
            AMem-=MemNeed;
            return 'A';
        }
        if(ACPU<BCPU&&AMem<BMem){
            BCPU-=CPUNeed;
            BMem-=MemNeed;
            return 'B';
        }
        if(ACPU<CPUNeed||AMem<MemNeed){
            BCPU-=CPUNeed;
            BMem-=MemNeed;
            return 'B';
        }
        if(BCPU<CPUNeed||BMem<MemNeed){
            ACPU-=CPUNeed;
            AMem-=MemNeed;
            return 'A';
        }
        double RatNeed = (double)CPUNeed/(double )MemNeed;
        double ARat = (double)ACPU/(double)AMem;
        double BRat = (double)BCPU/(double)BMem;
        if(abs(ARat-RatNeed)<=abs(BRat-RatNeed)){
            ACPU-=CPUNeed;
            AMem-=MemNeed;
            return 'A';
        }else{
            BCPU-=CPUNeed;
            BMem-=MemNeed;
            return 'B';
        }
    }



    //在迁移的时候以及删除虚拟机都会释放服务器的资源
    void Release(int CPUNeed,int MemNeed,char flag){
        //flag == 0代表双节点部署, 1代表部署于A节点，2代表部署于B节点
        if(flag=='C'){
            ACPU+=CPUNeed/2;AMem+=MemNeed/2;
            BCPU+=CPUNeed/2;BMem+=MemNeed/2;
        }else if(flag=='A'){
            ACPU+=CPUNeed;
            AMem+=MemNeed;
        }else if(flag=='B'){
            BCPU+=CPUNeed;
            BMem+=MemNeed;
        }
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

    VirtualM(const VirtualM& vm)= default;

    VirtualM& operator=(const VirtualM& vm) = default;
};


struct Operation{
    bool isAdd; //是否为添加虚拟机
    string name;    //需要部署的虚拟机的类型
    int id;     //部署或者释放虚拟机的id
    Operation():isAdd(false),id(0){}
};

ostream& operator<<(ostream& os,const VirtualM& virtualM){
    os<< virtualM.name <<" [CPU:" << virtualM.CPU << ", Mem:" << virtualM.Mem << ", flag: " << virtualM.flag<<"]";
    return os;
}

vector<Server> servers; //服务器的种类


vector<vector<Operation>> total_operations;    //所有天的所有操作，每一项是一天的所有操作vector
unordered_map<int,VirtualM> CPUMaXDay;
unordered_map<int,VirtualM> MemMaxDay;

multiset<Server> find_best_servers(){   //找出能够同时满足CPU占用和内存占用的性价比最高的服务器
    //同时满足并且总成本最低，感觉有点像个多重背包问题的逆问题，目前还没有很好的思路解决该问题
    multiset<Server> ans;
    return ans;
}

int main()
{
    int N,M,T,R;
    unordered_map<string,VirtualM> virtualMap;
	cin >> N;
	for (int i = 0; i < N; i++) {
        string name,cpu,mem,cost,loss;
		cin>>name>>cpu>>mem>>cost>>loss;
        Server server(name.substr(1,name.size()-2),stoi(cpu.substr(0,cpu.size()-1)),stoi(mem.substr(0,mem.size()-1))
        ,stoi(cost.substr(0,cost.size()-1)),stoi(loss.substr(0,loss.size()-1)));
        servers.push_back(server);
	}
    //将服务器根据购买的
    sort(servers.begin(),servers.end());

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
    unordered_map<int,VirtualM> id_to_virtualM;
    for(int i=0;i<T;i++){
        int dayCPU = 0,dayMem = 0;
        cin>>R;
        while(R--){
            string tmp;
            cin>>tmp;
            Operation oper;
            if(tmp=="(add, "){
                oper.isAdd = true;
                string name,ids;
                cin>>name>>ids;
                oper.name = name.substr(0,name.size()-1);
                oper.id = stoi(ids.substr(0,ids.size()-1));
                dayCPU += virtualMap[oper.name].CPU;
                dayMem += virtualMap[oper.name].Mem;
                id_to_virtualM[oper.id] = virtualMap[oper.name];
                if(dayCPU>CPUMax){
                    CPUMax = dayCPU;
                    CPUMaXDay = id_to_virtualM;
                }
                if(dayMem>MemMax){
                    MemMax = dayMem;
                    MemMaxDay = id_to_virtualM;
                }
            }else{
                oper.isAdd = false;
                string id;
                cin>>id;
                oper.id = stoi(id.substr(0,id.size()-1));
                id_to_virtualM.erase(oper.id);
            }
        }
        //一天结束是否还要进行处理？可以暂时先不考虑
    }

    //找出同时满足CPU和内存的服务器购买数量
    vector<Server> currentServers;  //当前已经购买的服务器，每个服务器都有一个对应的id，因为编号从0开始，其实本质上就是这个vector的对应下标
    unordered_map<int,pair<int,char>> total_deployment; //已经部署过的全部虚拟机
    //上面等会儿再写，直接处理每一天的请求
    //每一天是购买、迁移、最后部署
    //购买的
    for(vector<Operation>& dayOper:total_operations){
        vector<Server> purchase;    //当天购买的服务器
        unordered_map<int,pair<int,char>> migration;  //当天的迁移操作，把某个id的虚拟机迁移到某个id的服务器的A或者B节点，暂时先不考虑
        unordered_map<int,pair<int,char>> deployment;   //当天的部署操作，把某个id的虚拟机部署到某个id的服务器的A或者B节点，如果双节点部署，第二个填C

        for(Operation& oper:dayOper){
            if(oper.isAdd){
                //查看已经购买的服务器是否能够部署，尽量部署在能耗最低的节点上
                VirtualM vm = virtualMap[oper.name];
                id_to_virtualM[oper.id] = vm;
                sort(currentServers.begin(),currentServers.end(),[](const Server& a,const Server& b){
                    return a.Loss<b.Loss;
                }); //根据能耗排序
                bool beDeployed = false;    //是否能够被部署
                pair<int,char> deployAt;
                for(int i=0;i<currentServers.size();i++){
                    if(currentServers[i].canDeploy(vm.CPU,vm.Mem,vm.flag)){
                        deployAt = {i,currentServers[i].deploy(vm.CPU,vm.Mem,vm.flag)};
                        beDeployed = true;
                        total_deployment[oper.id] = deployAt;
                    }
                }
                if(!beDeployed){
                    //没有被部署，购买最优的服务器，此处采用动态策略

                }
            }else{
                //从已经部署的服务器中释放该服务器
                pair<int, char> Server_Deployed = total_deployment[oper.id];
                VirtualM vm = id_to_virtualM[oper.id];
                currentServers[Server_Deployed.first].Release(vm.CPU,vm.Mem,Server_Deployed.second);
                id_to_virtualM.erase(oper.id);
            }
        }

    }

	return 0;
}
