#include "ProcessInput.h"
#include "Process_p5.h"
#include "try2average.h"

void solve(){
    auto demand_remain = demand;
    vector<vector<unordered_map<string,int>>> ans;
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
        vector<unordered_map<string,int>> ans_item;
        for(int clientId=0;clientId<clientNum;++clientId){
            unordered_map<string, int> m;
            ans_item.push_back(m);
        }
        ans.push_back(ans_item);
    }
    Process_p5(demand_remain, bandwith_remain, ans);
    try2average(demand_remain,bandwith_remain,ans);
    output(ans);
}


int main() {

    ProcessInput();
    solve();


    return 0;
}

