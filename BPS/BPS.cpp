#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;

struct Edge{
    int startnode;
    int endnode;
    int value;
    int time;
};

vector<vector<Edge>> edge_list;
vector<vector<int>> graph;
map<int, float> sample_num_bps;
map<int, float> sample_num_swtc;
map<int,int> m1;
map<int,int> m2;
int stream_num = 200;
int window_size = 10000;


unsigned int RSHash(const std::string& str)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   for(std::size_t i = 0; i < str.length(); i++)
   {
      hash = hash * a + str[i];
      a    = a * b;
   }
   return hash;
}



void preoperation() {        //Ԥ�������ļ��ж����㡢�ڽӽ�����Լ����նȽ�����
    int graphsize;
    int number = 1;
    cout << "input the number:";
    cin >> graphsize;
    graph.resize(graphsize);
    edge_list.resize(stream_num);
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
        if (number % 1000 == 0) {
            cout << number << " ";
        }
        number++;
        istringstream sin(line);
        string t;
        int index = 0;
        int startnode, endnode;
        Edge e;
        while (sin >> t) {
            if (index == 0) {        //��???��??
                e.startnode = stoi(t);
                index++;
            }
            else {                   //??���㣬��������??�����ڽ���??
                e.endnode = stoi(t);
                //e.time = edge_num;
                //num = rand() % stream_num;
                edge_list[num].push_back(e);
                num = (num+1)%stream_num;
                break;
            }
        }
    }
}


void add_edge(Edge* e){
    // graph[e->startnode].push_back(e->endnode);
    // graph[e->endnode].push_back(e->startnode);
    cout << "����������Ϊ<" << e->startnode << "," << e->endnode << "," << e->value <<  ">" << endl;
}

void delete_edge(Edge* e){
    if (e==nullptr || e->value == 0){
        cout << "ԭ����Ϊ�գ�����ɾ��" << endl;
        return;
    }
    // vector<int>::iterator it = find(graph[e->startnode].begin(), graph[e->startnode].end(), e->endnode);
    // vector<int>::iterator it1 = find(graph[e->endnode].begin(), graph[e->endnode].end(), e->startnode);
    // if (it != graph[e->startnode].end()){
    //     graph[e->startnode].erase(it);
    //     graph[e->endnode].erase(it1);
    // }
    cout << "��ͼɾ��������Ϊ<" << e->startnode << "," << e->endnode << "," << e->value <<  ">" << endl;
}


/*
����ͼ���ݽṹ�����
[0,1,2,3,4]
0->[1,3,]
[[1,3],[2,3]]
SWTC������ʵ��*/


int BPS(vector<Edge> edge_list){
    int edge_num = edge_list.size();
    int current_time = 0;
    Edge* test_edge = nullptr;
    Edge* sample_edge = nullptr;
    Edge current_edge;
    Edge temp1,temp2;
    int number = 0;
    while (current_time < edge_num){
        if (sample_edge != nullptr && sample_edge->time <= current_time-window_size){
            temp2 = *sample_edge;
            test_edge = &temp2;
            sample_edge = nullptr;
            // cout << "�����߹���!" << endl;
            // cout << "���Ա�Ϊ" << test_edge->startnode << "," << test_edge->endnode  <<  ">" << "���ȼ�Ϊ" << test_edge->value << endl;

        }
        if (test_edge!=nullptr && test_edge->time <= current_time-2*window_size+1){
            test_edge = nullptr;
        }
        current_edge = edge_list[current_time];
        current_edge.time = current_time;
        current_edge.value = rand()%30000;        //ʹ�ñ�׼��hash����
        // cout << current_time << "ʱ�̣������ı�Ϊ<" << current_edge.startnode << "," << current_edge.endnode  <<  ">" << "���ȼ�Ϊ" << current_edge.value << endl;
        // if (sample_edge != nullptr){
        //     // cout << current_time << "ʱ�̣�ԭ������Ϊ<" << sample_edge->startnode << "," << sample_edge->endnode  <<  ">" << "���ȼ�Ϊ" << sample_edge->value << endl;
        // }
        if (sample_edge == nullptr || current_edge.value >= sample_edge->value){
            // delete_edge(sample_edge);
            temp1 = current_edge;
            sample_edge = &temp1;
            if (test_edge == nullptr || sample_edge->value >= test_edge->value){
                // cout << current_time << "ʱ�̣���Ч������: <" << sample_edge->startnode << "," << sample_edge->endnode << ">" << endl;
                // add_edge(sample_edge);                
            }
        }
        // cout << current_time << "ʱ�̣�������Ϊ<" << sample_edge->startnode << "," << sample_edge->endnode  <<  ">" << "���ȼ�Ϊ" << sample_edge->value << endl;
        if (test_edge != nullptr){
            // cout << "���Ա�Ϊ" << test_edge->startnode << "," << test_edge->endnode  <<  ">" << "���ȼ�Ϊ" << test_edge->value << endl;
        }
        if (test_edge == nullptr || sample_edge->value >= test_edge->value){
            if (current_time >= 2*window_size){
                number++;
            }
            if (current_time % (window_size / 10) == 0) {
                sample_num_bps[current_time]++;
            }
            // cout << "��Ч����!" << number << endl;
        }
        else{
            // cout << "��Ч����!" << endl;
        }
        current_time++;
    }
    return number;
}


int SWTC(vector<Edge> edge_list){
    int edge_num = edge_list.size();
    Edge old_slice;
    Edge new_slice;
    old_slice.value = 0;
    new_slice.value = 0;
    int current_time = 0;
    Edge current_edge;
    // srand(time(NULL));
    int number = 0;
    while (current_time < edge_num){
        if (current_time % window_size == 0 && current_time > 0){   //��ʼǰ�ж�һ�Σ�������������Ƭ�غ�ʱ���迼�Ǿ���Ƭ
            old_slice.value = 0;
        }
        current_edge = edge_list[current_time];
        current_edge.time = current_time;
        current_edge.value = rand()%30000;        //ʹ�ñ�׼��hash����
        // cout << current_time << "ʱ�̣������ı�Ϊ<" << current_edge.startnode << "," << current_edge.endnode  <<  ">" << "���ȼ�Ϊ" << current_edge.value << endl;
        if (current_edge.value >= new_slice.value){
            // cout << "�滻����Ƭ ";
            // delete_edge(&new_slice);
            new_slice = current_edge;
            if (new_slice.value > old_slice.value){
                // add_edge(&new_slice);
                // delete_edge(&old_slice);
            }
        }
        else{
            // cout << "��ʱ����Ƭ����Ϊ<" << new_slice.startnode << "," << new_slice.endnode << ">" << "���ȼ�Ϊ" << new_slice.value << ",�������滻" << endl;
        }
        if (old_slice.value > new_slice.value && old_slice.time <= current_time-window_size){
        }
        else{
            if (current_time % (window_size / 10) == 0) {
                sample_num_swtc[current_time]++;
            }
            if (current_time >= 2*window_size){
                number++;
            }
        }
        if (current_time % window_size == 0 && current_time > 0){   //����±ߵıȽϺ�ԭ��������Ƭ��ɾ���Ƭ����������Ƭ�ÿ�
            // cout << "�¾���Ƭ���£�" << endl;
            old_slice = new_slice;
            new_slice.value = 0;
        }
        current_time++;
        // cout << endl;
    }
    return number;
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
        cout << i.second << " ";
        outfile2 << i.second/float(stream_num) << " ";
    }
    cout << endl;
    for (auto i : sample_num_bps) {
        outfile1 << i.first << " ";
    }
    outfile1 << endl;
    for (auto i : sample_num_bps) {
        cout << i.second << " ";
        outfile1 << i.second / float(stream_num) << " ";
    }
    /*for (int i = 0; i < sample_num_bps.size(); i++){
        outfile1 << sample_num_bps[i]/float(stream_num) << " " ;
        outfile2 << sample_num_swtc[i]/float(stream_num) << " ";
    }*/
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
    /*for (int i = 0; i < 200; i++){
        int number1 = BPS(edge_list[0]);
        int number2 = SWTC(edge_list[0]);
        m1[number1]++;
        m2[number2]++;
    }
    save();*/
    // for (auto i : m1){
    //     cout << i.first << ":" << i.second << "; ";
    // }
    // cout << endl;
    // for (auto i : m2){
    //     cout << i.first << ":" << i.second << "; ";
    // }
    // int number1 = BPS(edge_list[0]);
    // int number2 = SWTC(edge_list[0]);
    // // cout << number << endl;
    // cout << number1 << " " << number2 << endl;
    //cout << "start" << endl;
    for(int i = 0; i < stream_num; i++){
        int number1 = BPS(edge_list[i]);
        int number2 = SWTC(edge_list[i]);
        //cout << number1 << " " << number2 << endl;
    }
    save_result();
}