#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <fstream>
#include <sstream>
#include <stack>
#include <windows.h>
using namespace std;

struct Node {
    vector<int> edges;
    vector<int> path;
    vector<Node*> children;
    Node(vector<int> _edges) :edges(_edges) {}
    Node() = default;
};

struct  Qtree {
    Node* root;
    Qtree(Node* _root) :root(_root) {}
    Qtree() = default;
};

vector<vector<Qtree>> TreeLst1;
vector<vector<Qtree>> TreeLst2;
vector<vector<int>> adjList(2050);
int nodenum;
stack<int> node_stack;
int visit[2050] = { 0 };
vector<vector<int>> paths;


//判断两个数组是否有相同元素，若有则输出一个即可
int compare(vector<int> set1, vector<int> set2) {
    sort(set1.begin(), set1.end());
    sort(set2.begin(), set2.end());
    int i=0, j = 0;
    while (i < set1.size() && j < set2.size()) {
        if (set1[i] == set2[j]) {
            return set1[i];
        }
        else if (set1[i] > set2[j]) {
            j++;
        }
        else {
            i++;
        }
    }
    return 0;
}

//用于在数组中查找指定元素是否存在
bool search(vector<int> lst, int ans) {
    return (find(lst.begin(), lst.end(), ans) != lst.end());
}


//判断某个结点的子树中是否有和qset不冲突的结点，若有则选择一个
vector<int> Find(Node* node, vector<int> qset) {
    int ans = compare(node->edges, qset);
    if (ans == 0) {
        return node->edges;
    }
    else {
        for (int i = 0; i < node->edges.size(); i++) {
            if (node->edges[i] == ans) {
                return Find(node->children[i], qset);
            }
        }
        return {};
    }
}

//由邻边信息生成第一批树（p=0）
void base() {
    for (int i = 0; i <= nodenum; i++) {
        TreeLst1.push_back({});
        TreeLst2.push_back({});
        for (int j = 0; j <= nodenum; j++) {
            if (i!=j && (find(adjList[i].begin(), adjList[i].end(), j) != adjList[i].end())) {
                Node* node = new Node(vector<int>{});
                TreeLst1[i].push_back(Qtree(node));
            }
            else {
                TreeLst1[i].push_back(Qtree(nullptr));
            }
        }
    }
    cout << "prepare successfully" << endl;
}

void prepare(string filepath) {
    for (int i = 0; i < 2050; i++) {
        adjList[i] = {};
    }
    string line;
    ifstream _input(filepath, ios::in);
    if (!_input) {
        cout << "File not exist!" << endl;
        return;
    }
    getline(_input, line);
    while (getline(_input, line)) {
        istringstream sin(line);
        string t;
        int startnode, endnode;
        int index = 0;
        while (sin >> t) {
            if (index == 0) {
                startnode = stoi(t);
                index++;
            }
            else {
                endnode = stoi(t);
                break;
            }
        }
        adjList[startnode].push_back(endnode);
        adjList[endnode].push_back(startnode);
    }
    cout << "input edge information successfully!" << endl;
}

//由上一级的树生成下一级的树
//start表示初始结点，p表示路径中包含的中间结点数，q为代表集大小，trees表示上一级的树集
Qtree grow(int start, int end, int p, int q, vector<vector<Qtree>> trees) {
    Qtree result;
    int flag = false;
    queue<Node*> QtNode;
    //生成树的根节点
    if (start == end) {
        result.root = nullptr;
    }
    else {
        for (int m : adjList[start]) {
            if (m != end && trees[end][m].root != nullptr && !search(trees[end][m].root->edges, m) && !search(trees[end][m].root->edges,start)) {
                flag = true;
                vector<int> temp(trees[end][m].root->edges);
                temp.insert(temp.begin(), m);
                Node* root = new Node(temp);
                result.root = root;
                break;
            }
        }
        if (!flag) {
            result.root = nullptr;
        }
    }
    //树为空，则不用继续往下生长
    if (result.root != nullptr) {
        QtNode.push(result.root);
    }
    int floor = 1;
    //按层次生长
    while (!QtNode.empty() && floor <= q) {
        int count = QtNode.size();
        //记录当前层次
        for (int i = 0; i < count; i++) {
            Node* node = QtNode.front();
            QtNode.pop();
            //为结点生成p个孩子
            if (!node->edges.empty()) {
                for (int j = 0; j < p; j++) {
                    Node* child = new Node(vector<int>{});
                    child->path = node->path;
                    child->path.push_back(node->edges[j]);
                    for (int k : adjList[start]) {
                        /*不选取当前邻接结点的三种情况
                        1、邻接结点为最终结点
                        2、从邻接结点到最终结点的路径上经过初始结点
                        3、邻接结点与最终结点间不存在经过长为p的路径*/
                        if (k == end || search(child->path, k) || trees[end][k].root == nullptr) {
                            continue;
                        }
                        else {
                            vector<int> ans = Find(trees[end][k].root, child->path);
                            if (search(ans, k)) {
                                continue;
                            }
                            //此处额外在p等于1的情况下认为存在，因为p等于0的树中空集也表示一条路径
                            if ((!ans.empty() || p == 1) && !search(ans, start)) {
                                ans.insert(ans.begin(), k);
                                child->edges = ans;
                                QtNode.push(child);
                                break;
                            }
                        }
                    }
                    node->children.push_back(child);
                }
            }
        }
        floor++;
    }
    return result;
}

//打印结点数据
void PrintNode(Node* node) {
    cout << "( ";
    if (node->edges.size() == 0) {
        cout << "λ" << " ";
    }
    for (int i : node->edges) {
        cout << i << " ";
    }
    cout << ") ";
}


//按层打印树的所有结点
void PrintTree(Qtree tree) {
    if (tree.root == nullptr) {
        cout << "tree not exist!" << endl;
    }
    else {
        int floor = 0;
        queue<Node*> qe;
        qe.push(tree.root);
        while (!qe.empty()) {
            cout << "floor" << floor << ": ";
            int count = qe.size();
            for (int i = 0; i < count; i++) {
                Node* node = qe.front();
                qe.pop();
                PrintNode(node);
                for (auto child : node->children) {
                    qe.push(child);
                }
            }
            floor++;
            cout << endl;
        }            
    }
}

//查询从start到end的树，需要调换顺序
void query(int start, int end) {
    PrintTree(TreeLst1[end][start]);
}

//生成所有的代表树
void generate_all_trees(int p, int q) {
    for (int th = 1; th <= p; th++) {
        for (int i = 0; i <= nodenum; i++) {
            for (int j = 0; j <= nodenum; j++) {
                Qtree tree = grow(j, i, th, p+q-th, TreeLst1);
                TreeLst2[i].push_back(tree);
            }
            TreeLst1[i].swap(TreeLst2[i]);
            vector<Qtree>().swap(TreeLst2[i]);
        }
    }
    cout << "trees generated!" << endl;
}

void printPath(vector<int> path) {
    for (auto i : path) {
        cout << i << " ";
    }
    cout << endl;
}

void addPath(stack<int> s) {
    vector<int> path;
    stack<int> s1(s);
    while (!s1.empty()) {
        int node = s1.top();
        s1.pop();
        path.insert(path.begin(), node);
    }
    paths.push_back(path);
}

void FindAllPath(int startnode, int endnode, int length) {
    if (node_stack.size() == length+1) {
        return;
    }
    node_stack.push(startnode);
    visit[startnode] = 1;
    if (startnode == endnode) {
        if (node_stack.size() == length+1) {
            addPath(node_stack);
        }
        node_stack.pop();
        visit[startnode] = 0;
    }
    else {
        for (auto i : adjList[startnode]) {
            if (!visit[i]) {
                FindAllPath(i, endnode, length);
            }
        }
        node_stack.pop();
        visit[startnode] = 0;
    } 
}

int main() {
    int p,q,start,end;
    cout << "please input the nodenum:";
    cin >> nodenum;
    string filepath = "..\\edgelist\\PA_100_edge.csv";
    prepare(filepath);
    base();
    cout << "please input p and q:";
    cin >> p >> q;
    generate_all_trees(p, q);
    cout << "please input the startnode and endnode:";
    cin >> start >> end;
    cout << "from" << start << "to" << end << " " << endl;
    FindAllPath(start, end, p+1);
    for (int i = 0; i < paths.size(); i++) {
        cout << "path " << i+1 << ":";
        printPath(paths[i]);
    }
    query(start, end);
    
    return 0;
}