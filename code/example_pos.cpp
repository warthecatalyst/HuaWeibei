#include<iostream>
#include<stdlib.h>   
#include<stdio.h>
#include<fstream>
#include <string>
#include<ctime>
#include <algorithm>
#include <vector>
#include<math.h>
#include <map>
#include<float.h>
using namespace std;

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
vector<Server> servers; //服务器的种类
map<Server, int> res;
const int person_number = 20;//种群个体数目
int N = 4;//搜索空间维度->变量数
const double LDW_ini = 0.9;//线性递减权值策略（LDW）
const double LDW_end = 0.4;
const double c1=2, c2=2;//学习因子
const int maxgen = 50;//迭代次数
const int const_num = 2;//约束数量

class POS_compute{
public:
//PSO参数
    double weight = 0;//惯性权重
    vector<vector<double>> X_pos;//粒子位置变量
    vector<vector<double>> V_speed;//粒子速度变量
    double pbest[person_number];//粒子适应值
    double max_pbest;//临时个体最优值
    double gbest;//粒子最优值
    double gbest_temp;//临时全局最优值--->更新历史值时使用
    double gbest_temp_2;//临时全局最优值--->约束判断中使用
    int gbest_index=0;//迭代过程中全局最优时的粒子位置
    vector<int> gbest_X;//记录相应的X取值
    //数据记录
    double iter_arr[maxgen];//记录迭代次数
    double gbest_arr[maxgen];//记录迭代过程中的最优值
    //目标约束
    vector<double> C;
    vector<vector<double>> A;
    vector<double> B;

    //目标函数
    double Fitness(int pers, int dim) {//输入参数->粒子数，维数
        double fit = 0;
        for (int j = 0; j < dim; j++) {
            fit = fit + C[j] * X_pos[pers][j];
        }
        return fit;
    }
    //约束条件
    double Constraint(int dim,int index ) {//维数，max_pbest所在粒子位置
        float constraint[const_num] = { 0 };
        gbest_temp_2 = INT_MAX;
        for (int i = 0; i < const_num; i++) {
            for (int j = 0; j < dim; j++) {
                constraint[i] = constraint[i] + A[i][j] * X_pos[index][j];
            }
        }
        if (constraint[0] >= B[0] && constraint[1] >= B[1]) {
            if (max_pbest < gbest_temp_2) {//更新历史最优值
                gbest_temp_2 = max_pbest;
            }
            else {
                gbest_temp_2 = gbest_temp_2;
            }
        }
        return gbest_temp_2;
    }

    //权重设置
    double Weight(int ITER) {
        double pso_weight  = 0;
        //惯性权重
        //weight = 0.8;
        //权重计算-2
        pso_weight  = ((LDW_ini - LDW_end)*(maxgen - ITER) / maxgen) + LDW_end;
        return pso_weight ;
    }

    //初始化
    void initial() {	
        for (int i = 0; i < person_number;i++) {
            vector<double> temp1;
            vector<double> temp2;
            for (int j = 0; j < N; j++) {
                temp1.push_back(0);
                temp2.push_back(rand() % (9 + 1) / (float)(9 + 1));//0-1
            }
            X_pos.push_back(temp1);
            V_speed.push_back(temp2);
        }
        for (int j = 0; j < N; j++) {
                gbest_X.push_back(0);
        }
        //初始化个体极值
        for (int i = 0; i < person_number; i++) {
            pbest[i] = Fitness(i,N);
        }
        //初始化全局极值---->判断是否满足约束，否则gbest=0;
        max_pbest = DBL_MAX;
        for (int i = 0; i < person_number; i++) {
            if (pbest[i] < max_pbest) {
                max_pbest = pbest[i];
                gbest_index = i;
            }
            else {
                max_pbest = max_pbest;
                gbest_index = gbest_index;
            }
        }
        gbest = Constraint(N, gbest_index);
    }

    void run_pso() {
        int iter = 0;
        srand(time(NULL));
        while (iter<maxgen) {
            //惯性权重
            weight = Weight(iter);
            //更新速度和位置
            for (int i = 0; i < person_number;i++) {
                for (int j = 0; j < N;j++) {
                    float r_rand[2];
                    for (int k = 0; k < 2;k++) {
                        r_rand[k]= rand() % (9 + 1) / (float)(9 + 1);
                    }
                    V_speed[i][j] = weight * V_speed[i][j] + c1 * r_rand[0]*(pbest[i]-X_pos[i][j]) + c2 * r_rand[0] * (gbest - X_pos[i][j]);	
                    if(V_speed[i][j] > 1) V_speed[i][j] = rand() % (9 + 1) / (float)(9 + 1);
                    X_pos[i][j] = round(X_pos[i][j] + (V_speed[i][j]));
                    if (X_pos[i][j] < 0)
                        X_pos[i][j] = 0; 
                    cout<<X_pos[i][j]<<endl;
                }	
            }
            //更新个体最优值
            for (int i = 0; i < person_number; i++) {	
                pbest[i] = Fitness(i,N);
            }
            max_pbest = DBL_MAX;//临时个体最优值
            for (int i = 0; i < person_number; i++) {
                if (pbest[i] < max_pbest) {
                    max_pbest = pbest[i];
                    gbest_index= i;
                }
                else {
                    max_pbest = max_pbest;
                    gbest_index = gbest_index;
                }
            }
            //约束条件判断
            gbest_temp = Constraint(N, gbest_index);

            //更新历史最优值
            if (gbest_temp < gbest) {
                gbest = gbest_temp;
                for (int j = 0; j < N; j++) {
                    gbest_X[j] = X_pos[gbest_index][j];
                }
            }
            else {
                gbest = gbest;
                for (int j = 0; j < N; j++) {
                    gbest_X[j] = gbest_X[j];
                }
            }
            iter_arr[iter] = iter;
            gbest_arr[iter] = gbest;
            iter++;
        }
    }

};

void findBestSet(int CPUMax, int MemMax) {
    POS_compute POS;
    for (int i = 0; i < N; i++) {
        POS.C.push_back(servers[i].Cost);
    }
    vector<double> temp1;
    for (int i = 0; i < N; i++) {
        temp1.push_back(servers[i].CPU);
    }
    POS.A.push_back(temp1);
    vector<double> temp2;
    for (int i = 0; i < N; i++) {
        temp2.push_back(servers[i].Mem);
    }
    POS.A.push_back(temp2);
    POS.B.push_back(CPUMax);
    POS.B.push_back(MemMax);
	POS.initial();
	POS.run_pso();
	cout << "min_Cost = " << POS.gbest << endl;
	cout << "X" << endl;
	for (int j = 0; j < N; j++) {
        res[servers[j]] = POS.gbest_X[j];
		cout << POS.gbest_X[j] << " "; 
	}
}
int main() {
    cin >> N;
	for (int i = 0; i < N; i++) {
        string name,cpu,mem,cost,loss;
		cin>>name>>cpu>>mem>>cost>>loss;
        Server server(name.substr(1,name.size()-2),stoi(cpu.substr(0,cpu.size()-1)),stoi(mem.substr(0,mem.size()-1))
        ,stoi(cost.substr(0,cost.size()-1)),stoi(loss.substr(0,loss.size()-1)));
        servers.push_back(server);
	}
    findBestSet(100,100);
	return 0;
}
