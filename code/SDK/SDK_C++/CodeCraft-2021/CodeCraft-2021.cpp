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
	int CPU, Mem, Cost, Loss;	//Cost是购买成本，Loss是能耗成本
	int ACPU, AMem, BCPU, BMem;
	Server() = default;
	Server(int CPU_, int Mem_, int Cost_, int Loss_) :
		CPU(CPU_), Mem(Mem_), Cost(Cost_), Loss(Loss_) {
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
	os << "CPU:" << server.CPU << "Mem:" << server.Mem << "Cost" << server.Cost << "Loss:" << server.Loss;
	return os;
}

struct VirtualM {
	int CPU, Mem;
	bool flag;	//是否双节点部署
	VirtualM() = default;
 	VirtualM(int CPU_, int Mem_, int flag_) :
		CPU(CPU_), Mem(Mem_), flag(flag_ == 1) {
	}
};

//字符串分割函数
vector<string> splitString(string& s, const char* delim) {
	char* strc = new char[s.size() + 1];
	strcpy(strc, s.c_str());
	std::vector<std::string> resultVec;
	char* tmpStr = strtok(strc, delim);
	while (tmpStr != NULL)
	{
		resultVec.push_back(std::string(tmpStr));
		tmpStr = strtok(NULL, delim);
	}

	delete[] strc;
	return resultVec;
}

int main()
{
	cin >> N;
	string line;
	unordered_map<string, Server> servers;
	cout << N << endl;
	for (int i = 0; i < N; i++) {
		cout << "log1" << endl;
		getline(cin, line);
		vector<string> current = splitString(line,", ");
		Server server(stoi(current[1]), stoi(current[2]), stoi(current[3]), stoi(current[4].substr(0,current[4].size()-1)));
		servers[current[0].substr(1)] = server;
	}

	for (auto it = servers.begin(); it != servers.end(); it++) {
		cout << it->first << " " << it->second << endl;
	}

	return 0;
}
