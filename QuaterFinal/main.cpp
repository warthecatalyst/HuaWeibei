#include "ProcessInput.h"
#include "Process_p5.cpp"
#include "try2average.h"

void solve(){
    auto demand_remain = demand;
    vector<vector<unordered_map<string,int>>> result;
    vector<vector<int>> bandwith_remain;
#ifdef Debug
    cout<<bandwith_remain.size()<<endl;
#endif
    for(int t=0;t<Times;++t){
        vector<int> v;
        for(int serverId=0;serverId<serverNum;++serverId){
            v.push_back(serverID_to_Val[serverId].second);
        }
        bandwith_remain.push_back(v);
        vector<unordered_map<string,int>> result_item;
        for(int clientId=0;clientId<clientNum;++clientId){
            unordered_map<string, int> m;
            result_item.push_back(m);
        }
        result.push_back(result_item);
    }
    Process_p5(demand_remain, bandwith_remain, result);
    try2average(demand_remain,bandwith_remain,result);
    output(result);
}


int main() {
    
    ProcessInput();
    solve();
    

    return 0;
}
