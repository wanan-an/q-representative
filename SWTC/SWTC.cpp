#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <random>
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
int stream_num = 400000;
int window_size = 8000000;
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


int count_distinct_edges(int starttime, int endtime){
    set<pair<int,int>> distinct_edges;
    for (int i = starttime+1; i <= endtime; i++){
        distinct_edges.insert({edge_list[i].startnode,edge_list[i].endnode});
    }
    return distinct_edges.size();
}

void save_list(string filename){
    ofstream outfile;
    filename.erase(filename.end()-4,filename.end());
    filename = filename + "1.txt";
    outfile.open(filename,ios::out);
    for (auto i : edge_list){
        outfile << i.startnode << " " << i.endnode << endl;
    }
    outfile.close();
}


int preoperation() {        //预处理，将边的信息存入列表中
    string filename;
    int number = 1;
    cout << "input the filename:";
    getline(cin,filename);
    old_list.resize(stream_num);
    new_list.resize(stream_num);
    sample_list.resize(stream_num);
    test_list.resize(stream_num);
    P_list.resize(stream_num);
    filename = filename + ".txt";
    string line;
    ifstream _csvinput(filename, ios::in);
    if (!_csvinput) {
        cout << "no such file!" << endl;
        return 0;
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
        uint32_t index = 0;
        uint32_t startnode, endnode;
        Edge e;
        while (sin >> t) {
            if (index == 0) {        //初始结点
                e.startnode = (uint32_t)stoul(t);
                index++;
            }
            else {                   //目标结点，并将边存入列表
                e.endnode = (uint32_t)stoul (t);
                if (e.startnode != e.endnode){
                    if (e.startnode < e.endnode){
                        uint32_t temp = e.startnode;
                        e.startnode = e.endnode;
                        e.endnode = temp;
                    }
                    edge_list.push_back(e);
                }
                break;
            }
        }
    }
    mt19937 gen(random_device{}());
    shuffle(edge_list.begin(),edge_list.end(),gen);
    save_list(filename);
    return 1;
}

//当边e插入样本图时，增加对应的三角形数量
void IncreaseTriangle(Edge e){
    int startnode = e.startnode;
    int endnode = e.endnode;
    set<int> combine;
    set_intersection(Adj[startnode].begin(),Adj[startnode].end(),Adj[endnode].begin(),Adj[endnode].end(),inserter(combine,combine.begin()));
    Triangle_num += combine.size();    
}

//当边e退出样本图时，减少对应的三角形数量
void DecreaseTriangle(Edge e){
    int startnode = e.startnode;
    int endnode = e.endnode;
    set<int> combine;
    set_intersection(Adj[startnode].begin(),Adj[startnode].end(),Adj[endnode].begin(),Adj[endnode].end(),inserter(combine,combine.begin()));
    Triangle_num -= combine.size();
}

//将边e插入样本图
void add_edge(Edge e){
    Graph.insert(e);
    Adj[e.startnode].insert(e.endnode);
    Adj[e.endnode].insert(e.startnode);
    IncreaseTriangle(e);
}

//将边e从样本图删去
void delete_edge(Edge e){
    //边e为空或不在样本图中
    if (e.value == 0 || Graph.find(e) == Graph.end()){
        return;
    }
    Graph.erase(e);
    Adj[e.startnode].erase(e.endnode);
    Adj[e.endnode].erase(e.startnode);
    DecreaseTriangle(e);
}




uint32_t allocate_position(uint32_t i1, uint32_t i2){
    string m = to_string(max(i1,i2));
    string s = to_string(min(i1,i2));
    string e = m+s;
    return RSHash((const uint8_t*) e.c_str(), e.length()) % stream_num;
}

double allocate_value(uint32_t i1, uint32_t i2){
    string m = to_string(max(i1,i2));
    string s = to_string(min(i1,i2));
    string e = m+s;
    return (double)(DEKHash((const uint8_t*) e.c_str(), e.length()) % 1000000+1)/1000001;
}

int compare_egde(Edge e1, Edge e2){
    if (e1.startnode==e2.endnode){
        return (e1.endnode == e2.startnode);
    }
    return (e1.startnode == e2.startnode);
}



void BPS(vector<Edge> edge_list){
    int edge_num = edge_list.size();
    int current_time = 0;
    while (current_time < edge_num){
        if (current_time % 100000 == 0){
            cout << " time=" << current_time;
        }        
        Edge current_edge = edge_list[current_time];
        int position = allocate_position(current_edge.startnode, current_edge.endnode);
        current_edge.time = current_time;
        current_edge.value = allocate_value(current_edge.startnode, current_edge.endnode);        //使用标准的hash函数
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
        current_time++;
    }
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
    int current_time = 1;
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
        current_edge.time = current_time;
        current_edge.value = allocate_value(current_edge.startnode, current_edge.endnode);        
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
                delete_edge(new_list[position]);
                new_list[position] = current_edge;
                if (new_list[position].value >= old_list[position].value){
                    delete_edge(old_list[position]);
                    add_edge(new_list[position]);
                }
            }
        }
        //新旧切片替换
        if (current_time % window_size == 0 && current_time > 0){   
            for (int i = 0; i < stream_num; i++){
                if (new_list[i].value < old_list[i].value && new_list[i].value > 0){
                    add_edge(new_list[i]);
                }
                old_list[i] = new_list[i];
                new_list[i].value = 0;
                new_list[i] = *new Edge();
            }
        }
        //每1/10个窗口计数一次
        if (current_time % (window_size / 10) == 0 && current_time > 0) {
            int starttime = ((current_time-window_size)<0)?0:(current_time-window_size);
            int true_n = count_distinct_edges(starttime,current_time);
            cout << " time: " << current_time;
            int m = Graph.size();
            int n = (double(alpha*stream_num*m)/calculate());
            cout << " n: " << n  << " true distinct num: " << true_n;
            sample_num_swtc[current_time]=Graph.size();
            double p = (double(m)/n)*(double(m-1)/(n-1))*(double(m-2)/(n-2));
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
    outfile1.close();
    outfile2.close();
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
    outfile1.close();
    outfile2.close();

}


int main(){
    srand(time(nullptr));
    if (!preoperation()){
        return 0;
    }
    SWTC(edge_list);
    // BPS(edge_list); 
    save_result();
}