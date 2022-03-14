# HuaWeibei
华中科技大学计算机学院硕士3班参赛组
参赛队员：朱寒冰、刘衡、周鑫宜

## 初赛
### 第一天 处理输入以及初步整理思路
初读题目之后，针对输入的数据如何存储，定义如下的数据结构：
1、首先剔除无用的信息：因为输出是跟输入文件的时刻顺序相同的，因此时刻具体指什么时间无关紧要，我们仅仅用一个索引来体现他是对应时刻即可
2、所有的客户节点都有一个名称（题目中的ID）和编号（数字），名称和编号是一一对应的。在实际计算的过程中，我们只需要他的编号即可，只有在最后输出的时候才需要对应名称（ID）。为了保险起见，定义一个unordered_map<string,int>实现名称到编号的转换，以及一个vector\<string>实现编号到名称的转换。
3、每个客户节点在一个时刻都有一个带宽的需求（需求可能为0）。定义一个vector<vector\<int>> demand，demand[i][j]表示的是第i个时间第j个客户节点的需求。
4、对于边缘节点，同样赋予其名称和编号。此处使用的数据结构为unordered_map<string,int>表示名称到编号的转化。以及定义一个vector\<pair<string,int>>，表示第i个节点的名称（ID）以及最大带宽BandWidth
5、客户到边缘节点存在着时延，用vector\<vector\<int>>表示第i个客户节点到第j个边缘节点的时延
6、QOS直接从config.ini进行读取即可
7、输入处理结束之后，实现名称到编号转换的两个map都可以删除


初步的思路整理：
由于单位带宽的成本就是1，因此我们需要最小化的就是所有边缘节点的95百分位带宽之和。因此我们希望的是95百分位带宽越小越好。95百分位带宽意味着存在5%的时刻，当前节点的带宽需求可以非常大(只要不超过带宽限制即可)。因此个人初步的思路就是贪心：对于每个节点，我们都尽量找5%（此处是向下取整，因为最后的结果是向上取证）的时间，他的带宽越高越好，然后剩下的时间，他的带宽越低越好（就是剩下时间的最大值要尽可能小）。

初步思路有如下的限制：
限制：对于每个边缘节点，有一部分的客户节点是无法被他服务的。
