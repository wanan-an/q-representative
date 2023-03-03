#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <math.h>
#include <inttypes.h>
using namespace std;

struct Edge{
    uint32_t startnode;
    uint32_t endnode;
    double value;
    int time;
    bool operator < (const Edge& e)const{
        return this->time < e.time;
    }
    Edge(){
        this->startnode = 0;
        this->endnode = 0;
        this->value = 0;
        this->time = 0;
    }
};

vector<Edge> edge_list;
vector<vector<int>> graph;
vector<Edge> old_list;
vector<Edge> new_list;
vector<Edge> test_list;
vector<Edge> sample_list;
vector<Edge> P_list;
map<int, float> sample_num_bps;
map<int, float> sample_num_swtc;
map<int,int> m1;
map<int,int> m2;
int Triangle_num = 0;
int Triangle_num_P = 0;
int stream_num = 160000;
int window_size = 4000000;
set<Edge> Graph;
map<int, set<int>> Adj;


uint32_t RSHash(const uint8_t* str, uint32_t len)
{
    uint32_t b = 378551;
    uint32_t a = 63689;
    uint32_t hash = 0;
	
	for(int i = 0; i < len; i++){
		hash = hash * a + (*str++);
        a *= b;
	}
    //return (hash & 0x7FFFFFFF);
	return hash;
}


uint32_t DEKHash(const uint8_t* str, uint32_t len)   
{   
    
    uint32_t hash = 1315423911;
	for(int i = 0; i < len; i++){
		hash = ((hash << 5) ^ (hash >> 27)) ^ str[i];
	}
	
    return hash;
}



void preoperation() {        //预处理，将边的信息存入列表中
    int graphsize;
    int number = 1;
    cout << "input the number:";
    cin >> graphsize;
    graph.resize(graphsize);
    old_list.resize(stream_num);
    new_list.resize(stream_num);
    sample_list.resize(stream_num);
    test_list.resize(stream_num);
    P_list.resize(stream_num);
    string filename = "PA_" + to_string(graphsize) + "_edge.txt";
    string line;
    ifstream _csvinput(filename, ios::in);
    if (!_csvinput) {
        cout << "no such file!" << endl;
        return;
    }
    getline(_csvinput, line);
    int num = 0;
    while (getline(_csvinput, line)) {
        if (number % 500000 == 0) {
            cout << number << " ";
        }
        number++;
        istringstream sin(line);
        string t;
        int index = 0;
        int startnode, endnode;
        Edge e;
        while (sin >> t) {
            if (index == 0) {        //初始结点
                e.startnode = stoi(t);
                index++;
            }
            else {                   //目标结点，并将边存入列表
                e.endnode = stoi(t);
                if (e.startnode != e.endnode){
                    edge_list.push_back(e);
                }
                break;
            }
        }
    }
}


void IncreaseTriangle(Edge e){
    int startnode = e.startnode;
    int endnode = e.endnode;
    set<int> combine;
    set_intersection(Adj[startnode].begin(),Adj[startnode].end(),Adj[endnode].begin(),Adj[endnode].end(),inserter(combine,combine.begin()));
    Triangle_num += combine.size();
    // for (auto i : combine){
    //     cout << "公共邻居为:" << i << " ";
    // }    
    // cout << "三角形数量增加" << combine.size() << endl;

    
}

void DecreaseTriangle(Edge e){
    int startnode = e.startnode;
    int endnode = e.endnode;
    set<int> combine;
    set_intersection(Adj[startnode].begin(),Adj[startnode].end(),Adj[endnode].begin(),Adj[endnode].end(),inserter(combine,combine.begin()));
    Triangle_num -= combine.size();
    // for (auto i : combine){
    //     cout << "公共邻居为:" << i << " ";
    // }
    // cout << "三角形数量减少" << combine.size() << endl;
}


void add_edge(Edge e){
    // cout << "插入边<" << e.startnode << "," << e.endnode << "," << e.time << "," << e.value << "> "<< endl;
    Graph.insert(e);
    if (Adj[e.startnode].find(e.endnode)==Adj[e.startnode].end()){
        Adj[e.startnode].insert(e.endnode);
        Adj[e.endnode].insert(e.startnode);
        IncreaseTriangle(e);
    }
    // cout << "插入样本边为<" << e.startnode << "," << e.endnode << "," << e.time << "," << e.value <<  ">" << endl;
}

void delete_edge(Edge e){
    // cout << "删除边<" << e.startnode << "," << e.endnode << "," << e.time << "," << e.value << "> " << endl;
    if (e.value == 0 || Graph.find(e) == Graph.end()){
        // cout << "原样本为空，无需删除" << endl;
        return;
    }
    Graph.erase(e);
    if (Adj[e.startnode].find(e.endnode)!=Adj[e.startnode].end()){
        Adj[e.startnode].erase(e.endnode);
        Adj[e.endnode].erase(e.startnode);
        DecreaseTriangle(e);
    }
    // cout << "删除样本边为<" << e.startnode << "," << e.endnode << "," << e.time << "," << e.value <<  ">" << endl;
}




uint32_t allocate_position(uint32_t i1, uint32_t i2){
    string m = to_string(max(i1,i2));
    string s = to_string(min(i1,i2));
    string e = m+s;
    return DEKHash((const uint8_t*) e.c_str(), e.length()) % stream_num;
}

double allocate_value(uint32_t i1, uint32_t i2){
    string m = to_string(max(i1,i2));
    string s = to_string(min(i1,i2));
    string e = m+s;
    return (double)(RSHash((const uint8_t*) e.c_str(), e.length()) % 1000000+1)/1000001;
}

int compare_egde(Edge e1, Edge e2){
    if (e1.startnode==e2.endnode){
        return (e1.endnode == e2.startnode);
    }
    return (e1.startnode == e2.startnode);
}

/*
样本图数据结构的设计
[0,1,2,3,4]
0->[1,3,]
[[1,3],[2,3]]
SWTC方法的实现*/


void BPS(vector<Edge> edge_list){
    int edge_num = edge_list.size();
    int current_time = 0;
    while (current_time < edge_num){
        if (current_time % 100000 == 0){
            cout << " time=" << current_time;
        }
        int position = ((rand()<<15)+rand())%stream_num;    //扩大散列的范围
        Edge current_edge = edge_list[current_time];
        current_edge.time = current_time;
        current_edge.value = rand()%30000+1;
        for (int i = 0; i < stream_num; i++){
            if (sample_list[i].value != 0 && sample_list[i].time <= current_time-window_size){
                test_list[i] = sample_list[i];
                sample_list[i].value = 0;
            }
            if (test_list[i].value!=0 && test_list[i].time <= current_time-2*window_size+1){
                test_list[i].value = 0;
            }
        }
        if (current_edge.value >= sample_list[position].value){
            // delete_edge(sample_edge);
            sample_list[position] = current_edge;
            if (sample_list[position].value >= test_list[position].value){
                // add_edge(sample_edge);                
            }
        }
        // 每滑动1/10个窗口计数一次
        if (current_time % (window_size / 10) == 0) {
            for (int i = 0; i < stream_num; i++){
                //有采样的情况：采样边优先级大于等于测试边且采样边不为空
                if (sample_list[i].value > 0 && sample_list[i].value >= test_list[i].value){
                    sample_num_bps[current_time]++;
                }
            }
        }
        //每秒计数一次
        // for (int i = 0; i < stream_num; i++){
        //     //有采样的情况：采样边优先级大于等于测试边且采样边不为空
        //     if (sample_list[i].value > 0 && sample_list[i].value >= test_list[i].value){
        //         sample_num_bps[current_time]++;
        //     }
        // }
        current_time++;
    }
}


void printGraph(){
    for (int i = 0; i < stream_num; i++){
        cout << "old边为<" << old_list[i].startnode << "," << old_list[i].endnode << "," << old_list[i].time << "," << old_list[i].value <<  ">" << endl;
        cout << "new边为<" << new_list[i].startnode << "," << new_list[i].endnode << "," << new_list[i].time << "," << new_list[i].value <<  ">" << endl;
    }
    cout << endl;
}

double calculate(){
    double result = 0;
    for (int i = 0; i < stream_num; i++){
        double value = max(new_list[i].value, old_list[i].value);
        int R = ceil(-(log(1-value)/log(2)));
        result += pow(2,-R);
    }
    return result;
}


void SWTC(vector<Edge> edge_list){
    int edge_num = edge_list.size();
    int current_time = 0;
    double alpha = 0.7213/(1+1.079/stream_num);
    cout << "size:" << edge_num << endl;
    while (current_time < edge_num){
        if (Graph.empty()){
        }
        else{
            if (Graph.begin()->time <= current_time-window_size){
                delete_edge(*Graph.begin());
            }
        }
        Edge current_edge = edge_list[current_time];
        int position = allocate_position(current_edge.startnode, current_edge.endnode);
        // int position =((rand()<<15)+rand())%stream_num;
        current_edge.time = current_time;
        current_edge.value = allocate_value(current_edge.startnode, current_edge.endnode);        //使用标准的hash函数
        // current_edge.value = (rand()+1)/(float)(RAND_MAX+1);        //使用标准的hash函数
        if (compare_egde(current_edge, new_list[position])){
            set<Edge>::iterator it = Graph.find(new_list[position]);
            new_list[position].time = current_time;
            if (it != Graph.end()){
                Graph.erase(it);
                Graph.insert(new_list[position]);
            }
        }
        else{
            if (current_edge.value >= new_list[position].value){
                // cout << "更新新切片" << "! ";
                delete_edge(new_list[position]);
                new_list[position] = current_edge;
                if (new_list[position].value >= old_list[position].value){
                    // if (new_list[position].value == old_list[position].value){
                    //     cout << "new边为<" << new_list[position].startnode << "," << new_list[position].endnode << "," << new_list[position].time << "," << new_list[position].value <<  ">" << endl;
                    //     cout << "old边为<" << old_list[position].startnode << "," << old_list[position].endnode << "," << old_list[position].time << "," << old_list[position].value <<  ">" << endl;
                    // }
                    // cout << "替换样本" << "! ";
                    delete_edge(old_list[position]);
                    add_edge(new_list[position]);
                }
            }
        }
        // if (current_edge.value >= new_list[position].value){
        //         // cout << "更新新切片" << "! ";
        //         delete_edge(new_list[position]);
        //         new_list[position] = current_edge;
        //         if (new_list[position].value >= old_list[position].value){
        //             // cout << "替换样本" << "! ";
        //             delete_edge(old_list[position]);
        //             add_edge(new_list[position]);
        //         }
        // }
        if (current_time % window_size == 0 && current_time > 0){   //完成新边的比较后，原来的新切片变成旧切片，并将新切片置空
            for (int i = 0; i < stream_num; i++){
                if (new_list[i].value < old_list[i].value && new_list[i].value > 0){
                    // cout << i << "号子流补充采样" << " ";
                    add_edge(new_list[i]);
                }
                old_list[i] = new_list[i];
                new_list[i].value = 0;
                new_list[i] = *new Edge();
            }
            // cout << "子图大小为：" << Graph.size() << endl;
        }
        // sample_num_swtc[current_time]=Graph.size();
        // cout << "time: " << current_time << " Graph size: " << Graph.size() << " Triangle num: " << Triangle_num << endl;
        // if (current_time % (window_size / 10) == 0) {
        //     printGraph();
        // }
        //每1/10个窗口计数一次
        if (current_time % (window_size / 10) == 0 && current_time > 0) {
            cout << "time: " << current_time;
            int m = Graph.size();
            // cout << "m" << m << endl;
            // cout << "b" << calculate() << endl;
            int n = (double(alpha*stream_num*m)/calculate());
            cout << " n: " << n ;
            sample_num_swtc[current_time]=Graph.size();
            double p = (double(m)/n)*(double(m-1)/(n-1))*(double(m-2)/(n-2));
            // cout << p << endl;
            int count = Triangle_num/p;
            cout << " Graph size: " << Graph.size() << " Triangle num: " << Triangle_num << " True num: " << count << endl;

        }
        current_time++;
    }
    return ;
}



void save_result(){
    ofstream outfile1;
    ofstream outfile2;
    outfile1.open("sample_rate_bps.txt", ios::out);
    outfile2.open("sample_rate_swtc.txt", ios::out);
    for (auto i : sample_num_swtc) {
        outfile2 << i.first << " ";
    }
    outfile2 << endl;
    for (auto i : sample_num_swtc) {
        outfile2 << i.second/float(stream_num) << " ";
    }
    for (auto i : sample_num_bps) {
        outfile1 << i.first << " ";
    }
    outfile1 << endl;
    for (auto i : sample_num_bps) {
        // cout << i.second << " ";
        outfile1 << i.second / float(stream_num) << " ";
    }
}

void save(){
    ofstream outfile1;
    ofstream outfile2;
    outfile1.open("sample_num_bps.txt", ios::out);
    outfile2.open("sample_num_swtc.txt", ios::out);
    for (auto i : m1){
        outfile1 << i.first << " " ;
    }
    outfile1 << endl;
    for (auto i : m1){
        outfile1 << i.second << " " ;
        
    }
    for (auto i : m2){
        outfile2 << i.first << " " ;
    }
    outfile2 << endl;
    for (auto i : m2){
        outfile2 << i.second << " " ;
    }
}


int main(){
    srand(time(NULL));
    preoperation();
    SWTC(edge_list);
    // BPS(edge_list); 
    save_result();
}