#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <inttypes.h>
#include <math.h>
using namespace std;

struct Edge{
    uint64_t startnode;
    uint64_t endnode;
    float value;
    bool operator < (const Edge& e)const{
        return this->startnode < e.startnode;
    }
    Edge(){
        this->startnode = 0;
        this->endnode = 0;
        this->value = 0;
    }
};

vector<Edge> edge_list;
vector<Edge> P_list;
int Triangle_num = 0;
int stream_num = 75000;
int window_size = 1500000;
map<int, set<int>> Adj;
set<pair<int,int>> distinct_edges;
set<int> no_empty_stream;


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



int preoperation() {        //预处理，将边的信息存入列表中
    string filename;
    int number = 1;
    cout << "input the filename:";
    getline(cin,filename);
    filename = filename + ".txt";
    P_list.resize(stream_num);
    string line;
    ifstream _csvinput(filename, ios::in);
    if (!_csvinput) {
        cout << "no such file!" << endl;
        return 0;
    }
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
                e.startnode = stoul(t);
                index++;
            }
            else {                   //目标结点，并将边存入列表
                e.endnode = stoul(t);
                edge_list.push_back(e);
                break;
            }
        }  
    }
    return 1;
}


void IncreaseTriangle(Edge e){
    int startnode = e.startnode;
    int endnode = e.endnode;
    set<int> combine;
    set_intersection(Adj[startnode].begin(),Adj[startnode].end(),Adj[endnode].begin(),Adj[endnode].end(),inserter(combine,combine.begin()));
    Triangle_num += combine.size();
    Adj[startnode].insert(endnode);
    Adj[endnode].insert(startnode);
}

void DecreaseTriangle(Edge e){
    int startnode = e.startnode;
    int endnode = e.endnode;
    set<int> combine;
    set_intersection(Adj[startnode].begin(),Adj[startnode].end(),Adj[endnode].begin(),Adj[endnode].end(),inserter(combine,combine.begin()));
    Triangle_num -= combine.size();
    Adj[startnode].erase(endnode);
    Adj[endnode].erase(startnode);
}


void UpdateTriangle(int flag,Edge e){
    if (flag == 0){
        DecreaseTriangle(e);
    }
    else{
        IncreaseTriangle(e);
    }
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


void PartitionCT(vector<Edge> edge_list){
    int edge_num = edge_list.size();
    int current_time = 1;
    double n = 0;
    double q = 1;
    cout << "size:" << edge_num << endl;
    while (current_time < edge_num){
        int g = 0;
        Edge current_edge = edge_list[current_time];
        distinct_edges.insert({current_edge.startnode,current_edge.endnode});
        int position = allocate_position(current_edge.startnode, current_edge.endnode);
        no_empty_stream.insert(position);
        current_edge.value = allocate_value(current_edge.startnode, current_edge.endnode);        //使用标准的hash函数
        if (compare_egde(current_edge, P_list[position]) != 1){
            if (current_edge.value < P_list[position].value || P_list[position].value == 0){
                if (P_list[position].value != 0){
                    UpdateTriangle(0,P_list[position]);
                    g = ceil(-((log(P_list[position].value))/log(2)));
                }
                int y = ceil(-((log(current_edge.value))/log(2)));
                // cout << " time: " << current_time << " y: " << y << " g: " << g ; 
                if (y > g){
                    n = n + 1/q;
                    q = q + ((double)1/stream_num)*(pow(2,-y)-pow(2,-g));
                    // cout << " q: " << q << " n: " << n << endl; 
                }
                P_list[position] = current_edge;
                UpdateTriangle(1,current_edge);
            }
        }
        if ((current_time % (window_size / 10) == 0 && current_time > 0) || current_time == edge_num-1) {
            int starttime = ((current_time-window_size)<0)?0:(current_time-window_size);
            int true_n = distinct_edges.size();
            cout << "time: " << current_time;
            cout << " n: " << n  << " true distinct num: " << true_n;
            int m = no_empty_stream.size();
            double p = (double(m)/n)*(double(m-1)/(n-1))*(double(m-2)/(n-2));
            int count = Triangle_num/p;
            cout << " Graph size: " << no_empty_stream.size() << " Triangle num: " << Triangle_num << " True num: " << count << endl;

        }
        current_time++;
    }
}



int main(){
    srand(time(NULL));
    if(!preoperation()){
        return 0;
    }
    // vector<Edge> list(edge_list.begin()+8000000, edge_list.begin()+12000001);
    PartitionCT(edge_list);

}