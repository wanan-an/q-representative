#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
using namespace std;

struct Triangle {
    int node1;
    int node2;
    int node3;
};


vector<pair<int,int>> position(2050);                 //用于排序后记录结点的顺序，数值越小表示该节点度越大，即越靠前
map<vector<int>,int> position_egde;                  //用于记录每一条边上的三角形对数
vector<pair<int, vector<int>>> nodes(2050);     //保存结点编号以及邻接结点信息

int grapgsize;


bool cmp(pair<int, vector<int>> p1, pair<int, vector<int>> p2) {
    return p1.second.size() > p2.second.size();
}



vector<int> find(vector<int> adj1, vector<int> adj2) {       //寻找两个数组中的共同元素
    vector<int> result;
    int i = 0, j = 0;
    sort(adj1.begin(), adj1.end());
    sort(adj2.begin(), adj2.end());
    while (i < adj1.size() && j < adj2.size()) {
        if (adj1[i] == adj2[j]) {
            result.push_back(adj1[i]);
            i++;
            j++;
            continue;
        }
        if (adj1[i] < adj2[j]) {
            i++;
        }
        else {
            j++;
        }
    }
    return result;
}




void preoperation() {        //预处理，包括从文件中读入结点、邻接结点信息以及按照度进行排序
    for (int i = 0; i < 2050; i++) {
        nodes[i].first = i;
        nodes[i].second = {};
    }
    cout << "input the number:";
    cin >> grapgsize;
    string filename = "PA_" + to_string(grapgsize) + "_edge.txt";
    string line;
    ifstream _csvinput(filename, ios::in);
    if (!_csvinput) {
        cout << "no such file!" << endl;
        return;
    }
    getline(_csvinput, line);
    while (getline(_csvinput, line)) {
        istringstream sin(line);
        string t;
        int index = 0;
        int startnode, endnode;
        while (sin >> t) {
            if (index == 0) {        //初始结点
                startnode = stoi(t);
                index++;
            }
            else {                   //目标结点，并更新两个结点的邻接列表
                endnode = stoi(t);
                nodes[startnode].second.push_back(endnode);
                nodes[endnode].second.push_back(startnode);
                break;
            }
        }
    }
    vector<pair<int, vector<int>>> temp = nodes;
    sort(temp.begin(), temp.end(), cmp);
    for (int i = 0; i < 2050; i++) {
        position[temp[i].first].first = i;
        position[temp[i].first].second = temp[i].second.size();
    }
}





vector<Triangle> forward(vector<pair<int, vector<int>>> nodes_info) {
    vector<vector<int>> Anode(2050);
    vector<pair<int,int>> temp_position(2050);
    sort(nodes_info.begin(), nodes_info.end(), cmp);
    for (int i = 0; i < nodes_info.size(); i++) {
        temp_position[nodes_info[i].first].first = i;
        temp_position[nodes_info[i].first].second = nodes_info[i].second.size();
    }
    vector<Triangle> triangles;
    for (int i = 0; i < nodes_info.size(); i++) {
        for (auto j : nodes_info[i].second) {
            if (temp_position[nodes_info[i].first] <= temp_position[j]) {
                Triangle triangle;
                if (find(Anode[nodes_info[i].first], Anode[j]).empty()) {
                }
                else {
                    triangle.node2 = nodes_info[i].first;
                    triangle.node3 = j;
                    for (auto u : find(Anode[nodes_info[i].first], Anode[j])) {
                    //cout << "find:" << nodes_info[i].first << "-" << j << "-" << u << endl;
                        triangle.node1 = u;
                        triangles.push_back(triangle);
                    }
                }
                Anode[j].push_back(nodes_info[i].first);
                //cout << nodes_info[i].first << " added to:" << j << endl;
            }
        }
    }
    return triangles;
}

vector<Triangle> coin(vector<pair<int, vector<int>>> nodes_info, int p) {
    vector<pair<int, vector<int>>> nodes_result = nodes_info;
    srand(time(NULL));
    for (int i = 0; i < nodes_result.size(); i++) { 
        for (auto j : nodes_result[i].second) {
            if (position[nodes_result[i].first].first < position[j].first) {
                if (rand() % 10 >= p) {
                   /* cout << "remove edge:" << nodes_result[i].first << "->" << j << endl;
                    cout << "remove edge:" << nodes_result[j].first << "->" << i << endl;*/
                    vector<int>::iterator p_j = find(nodes_result[i].second.begin(), nodes_result[i].second.end(), j);
                    vector<int>::iterator p_i = find(nodes_result[j].second.begin(), nodes_result[j].second.end(), i);
                    if (p_j != nodes_result[i].second.end()) {
                        nodes_result[i].second.erase(p_j);
                    }
                    if (p_i != nodes_result[j].second.end()) {
                        nodes_result[j].second.erase(p_i);
                    }
                }
            }  
        }
    }
    /*for (int i = 0; i < nodes_result.size(); i++) {
        cout << i << ":";
        for (auto j : nodes_result[i].second) {
            cout << j << " ";
        }
        cout << endl;
    }*/
    return forward(nodes_result);
}




//生成每一条边对应的三角形数量
void generatek(vector<Triangle> triangles) {
    for (Triangle t : triangles) {
        vector<int> edge1, edge2, edge3;
        edge1 = { t.node1, t.node2 };
        edge2 = { t.node1, t.node3 };
        edge3 = { t.node2, t.node3 };
        if (position_egde.find(edge1) == position_egde.end()) {
            position_egde[edge1] = 1;
        }
        else {
            position_egde[edge1]++;
        }
        if (position_egde.find(edge2) == position_egde.end()) {
            position_egde[edge2] = 1;
        }
        else {
            position_egde[edge2]++;
        }
        if (position_egde.find(edge3) == position_egde.end()) {
            position_egde[edge3] = 1;
        }
        else {
            position_egde[edge3]++;
        }
    }
}

//计算图中关联的三角形组数
int countk(map<vector<int>, int> position_egde) {
    int k = 0;
    for (map<vector<int>, int>::iterator iter = position_egde.begin(); iter != position_egde.end(); iter++) {
        k += iter->second * (iter->second - 1) / 2;
    }
    return k;
}

void printTriangle(vector<Triangle> T) {
    for (Triangle t : T) {
        cout << "triangle is:" << t.node1 << "-" << t.node2 << "-" << t.node3 << endl;
    }
}


void transfrom() {

}




int main(){
    preoperation();
    /*for (int i = 0; i < nodes.size(); i++) {
        cout << i << ":";
        for (auto j : nodes[i].second) {
            cout << j << " ";
        }
        cout << endl;
    }*/
    vector<Triangle> triangles_info = forward(nodes);
    cout << "原三角形数量：" << triangles_info.size() << endl;
    int p = 7;
    vector<Triangle> triangles = coin(nodes, p);
    cout << "稀疏图中三角形数量为:" << triangles.size() << endl;
    printTriangle(triangles_info);

    cout << endl;
    printTriangle(triangles);

    generatek(triangles_info);
    for (map<vector<int>, int>::iterator iter = position_egde.begin(); iter != position_egde.end(); iter++) {
        cout << iter->first[0] << "->" << iter->first[1] << ":" << iter->second << endl;
    }
    int k = countk(position_egde);
    cout << "k :" << k << endl;
    cout << "得到的近似三角形数量为:" << int(triangles.size()*pow(10.0/p,3)) << endl;
}
