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
    vector<pair<int,int>> vec;
    for(int day = 0;day<Times;day++){
        int sum = 0;
        for(int client = 0;client<clientNum;client++){
            for(auto& it:demand_remain[day][client]){
                sum+=it.second;
            }
        }
        vec.emplace_back(day,sum);
    }
    sort(vec.begin(),vec.end(),[&](const pair<int,int>& a,const pair<int,int>& b){
        return a.second>b.second;
    });
    for(auto &v:vec){
        cout<<"day"<<v.first<<":"<<v.second<<endl;
    }
    try2average(demand_remain,bandwith_remain,ans);
    output(ans);
}


int main() {

    ProcessInput();
    solve();


    return 0;
}
