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
#include <queue>

using namespace std;
using pii = pair<int,int>;

long long width[150];
long long demand[40][9000];       //需求序列
string client_names[40];
string site_names[150];
int client_num, site_num;
int Max_time,QoS;
bool tunnel[150][40];       //tunnel[边缘节点][客户]
int site_out[150], client_in[40], overload[150];//出度入度
priority_queue<long long> load[150];

//分割函数
vector<string> split(string& s, char sep) {
    vector<string> res;
    string tmp;
    for (auto&each:s) {
        if(each==sep)
            res.emplace_back(tmp), tmp.clear();
        else
            tmp+=each;
    }
    if (!tmp.empty())
        res.emplace_back(tmp);
    return move(res);
}
//初始化和读入
void init() {

    memset(tunnel, false, sizeof(tunnel));

    freopen("../data/demand.csv", "r", stdin);
    string tmp;
    cin>>tmp;
    vector<string> line = split(tmp, ',');
    for (int i = 1; i < line.size(); i++)
        client_names[i] = line[i];
    client_num = (int)line.size() - 1;
    while(cin>>tmp) {
        Max_time++;
        line.clear();
        line = split(tmp, ',');
        for (int i = 1; i < line.size(); i++)
            demand[i][Max_time] = (long long)stoi(line[i]);
    }
    cin.clear();


    freopen("../data/site_bandwidth.csv", "r", stdin);
    cin>>tmp;
    while(cin>>tmp) {
        site_num++;
        line.clear();
        line = split(tmp, ',');
        site_names[site_num] = line[0];
        width[site_num] = (long long)stoi(line[1]);
    }
    cin.clear();

    freopen("../data/config.ini", "r", stdin);
    cin>>tmp;cin>>tmp;
    line.clear();
    line = split(tmp, '=');
    QoS = stoi(line[1]);
    cin.clear();

    freopen("../data/qos.csv", "r", stdin);
    cin>>tmp;
    int cnt = 0;
    while(cin>>tmp) {
        cnt++;
        line.clear();
        line = split(tmp, ',');
        for (int i = 1; i < line.size(); i++) {
            int val = stoi(line[i]);
            if(val < QoS)
                tunnel[cnt][i] = true, site_out[cnt]++, client_in[i]++;
        }
    }
    cin.clear();

    for(int i=1;i<=site_num;++i)overload[i]=0.05*Max_time;
}

struct Edge {
    int u, v;
    long long c, f;
    Edge(int x,int y,long long z,long long k){u=x;v=y;c=z;f=k;}
};
vector<Edge> edge;
vector<int> G[200];
int s, t, S, T;
long long INF=999999999999999;
long long dist[200],cur[200];
bool vist[200];

void Addedge(int x,int y,long long z)
{
    edge.push_back(Edge(x,y,z,0));
    edge.push_back(Edge(y,x,0,0));
    int m=edge.size();
    G[x].push_back(m-2);
    G[y].push_back(m-1);
}

bool bfs()
{
    memset(vist,0,sizeof(vist));
    queue<int> q;
    dist[S]=0;
    vist[S]=1;
    q.push(S);
    while(!q.empty())
    {
        int u=q.front();q.pop();
        for(int i=0;i<G[u].size();i++)
        {
            int v=edge[G[u][i]].v;
            long long c=edge[G[u][i]].c,f=edge[G[u][i]].f;
            if(!vist[v] && c>f)
            {
                dist[v]=dist[u]+1;
                vist[v]=1;
                q.push(v);
            }
        }
    }
    return vist[T];
}

long long dfs(int u,long long a)
{
    if(u==T || !a) return a;
    long long flow=0,tmp;
    for(int i=cur[u];i<G[u].size();i++)
    {
        cur[u]=i;
        int v=edge[G[u][i]].v;
        long long c=edge[G[u][i]].c,f=edge[G[u][i]].f;
        if(dist[v]==dist[u]+1 && (tmp=(dfs(v,min(a,c-f))))>0)
        {
            edge[G[u][i]].f+=tmp;
            edge[G[u][i]^1].f-=tmp;
            a-=tmp;
            flow+=tmp;
            if(!a) break;
        }
    }
    return flow;
}

long long maxflow()
{
    long long flow=0;
    while(bfs())
    {
        memset(cur,0,sizeof(cur));
        flow+=dfs(S,INF);
    }
    return flow;
}

long long imo[200];

bool build_judge(int Time, long long maxx, int x) { //x为过载结点，maxx为过载结点最大流量，Time是时刻
    memset(imo, 0, sizeof(imo));
    edge.clear();
    for (int i = 0; i < client_num + site_num + 5; i++)
        G[i].clear();

    s = 0;                              //原图源点
    t = client_num + site_num + 1;      //原图汇点
    S = t + 1;                          //虚拟图源点
    T = S + 1;                          //虚拟图源点
    for (int i = 1; i <= client_num; i++) {
        imo[i] -= demand[i][Time];
        imo[t] += demand[i][Time];
    }

    for (int i = client_num + 1; i <= client_num + site_num; i++) {
        if(i - client_num != x) Addedge(s, i, width[i - client_num]);
        else {
            Addedge(s, i, width[i - client_num] - maxx);
            imo[s] -= maxx;
            imo[i] += maxx;
        }
    }

    for (int i = 1; i <= site_num; i++)
        for (int j = 1; j <= client_num; j++)
            if (tunnel[i][j])
                Addedge(i + client_num, j, INF);

    long long sum = 0;
    for (int i = 0; i <= client_num + site_num + 1; i++) {
        if (imo[i] > 0) {
            Addedge(S, i, imo[i]);
            sum += imo[i];
        }
        else if (imo[i] < 0) {
            Addedge(i ,T, -imo[i]);
        }
    }

    Addedge(t, s, INF);

    long long tmp = maxflow();
    if(tmp != sum) return false;
    return true;
}

void calc(int tim) {
    vector<pair<string, long long>> out[40];
    long long site_sum[150];
    long long test[150];
    memset(site_sum, 0, sizeof(site_sum));
    memset(test, 0, sizeof(test));
    for(int i = 0; i < edge.size(); i+=2) {
        int u = edge[i].u, v = edge[i].v;
        long long f = edge[i].f;
        if(!f) continue;
        if(u > client_num && u <= client_num + site_num && v >= 1 && v <= client_num) {
            out[v].emplace_back(site_names[u-client_num], f);
            test[v]+=f;
            site_sum[u-client_num] += f;
        }
    }
    for(int i = 1; i <= client_num; i++) {
        cout<<client_names[i];
        printf(":");
        for(int j = 0; j < out[i].size(); j++) {
            printf("<");
            cout<<out[i][j].first;
            printf(",");
            printf("%lld",out[i][j].second);
            printf(">");
            if(j != out[i].size() - 1) printf(",");
        }
        printf("\n");
    }
    for(int i = 1; i <= site_num; i++) {
        load[i].push(site_sum[i]);
    }
    for(int i = 1; i <= client_num; i++) {
        if(test[i] != demand[i][tim]) {
            printf("day: %d\n", tim);
            cout<< client_names[i];
            printf("\n");
        }
    }
}

int heavyCalc(int t) {
    vector<pii> above95(site_num + 1);
    for (int i = 1; i <= site_num; ++i) {
        for (int j = 1; j <= client_num; ++j)
            if (tunnel[i][j])
                above95[i].first += demand[j][t]/client_in[j];
        above95[i].second = i;
    }
    sort(above95.begin(), above95.end(), greater<>());
    for(auto&each:above95)if(overload[each.second]){
        overload[each.second]--;
        return each.second;
    }
    return 0;
}

int main() {
    init();
    //heavyCalc();
    cin.clear();
    random_device rd;
    mt19937 mt(rd());
    freopen("../output/solution.txt", "w", stdout);
    for (int k = 1; k <= Max_time; k++) {
        int x = heavyCalc(k);
        //if(!timeSeq[k].empty()) x = timeSeq[k].front();
        long long l = 0, r = width[x], mid, pre=0;
        bool flag;
        while(l<r-1) {
            mid = (l+r) / 2;
            if(build_judge(k, mid, x))
                l = mid, pre = mid, flag = true;
            else r = mid - 1, flag = false;
        }
        if(!flag)
            build_judge(k, pre, x);
        calc(k);
    }
    long long ans = 0;
    for(int i = 1; i <= site_num; i++) {
        int cnt = (int)Max_time - ceil(Max_time * 0.95);
        for(int j = 1; j < cnt; j++)
            load[i].pop();
        ans += load[i].top();
    }
    printf("%lld",ans);
    return 0;
}