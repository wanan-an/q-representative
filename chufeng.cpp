#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
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
    int depth;
    int width;
    Node* root;
    Qtree(Node* _root) :root(_root) {}
    Qtree() {}
};

vector<Qtree> TreeList;
vector<Qtree> TreeList2;
vector<vector<int>> adjList;


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


void preparetion() {
    adjList.push_back({});
    adjList.push_back({ 2,5,6,7,9 });
    adjList.push_back({ 1,4,9 });
    adjList.push_back({ 6,8,9 });
    adjList.push_back({ 2,7,8,9,10 });
    adjList.push_back({ 1,6,10 });
    adjList.push_back({ 1,3,5,10 });
    adjList.push_back({ 1,4,10 });
    adjList.push_back({ 3,4,10 });
    adjList.push_back({ 1,2,3,4 });
    adjList.push_back({ 4,5,6,7,8 });
    for (int i = 0; i < 10; i++) {
        if (find(adjList[10].begin(), adjList[10].end(), i) != adjList[10].end()) {
            Node* node = new Node(vector<int>{});
            TreeList.push_back(Qtree(node));
        }
        else {
            TreeList.push_back(Qtree(nullptr));
        }
    }
}




//start表示初始结点，p表示路径中包含的中间结点数，q为代表集大小，trees表示上一级的树集
Qtree grow(int start, int end, int p, int q, vector<Qtree> trees) {
    Qtree result;
    int flag = false;
    queue<Node*> QtNode;
    //生成树的根节点
    for (int m : adjList[start]) {
        if (trees[m].root != nullptr) {
            flag = true;
            vector<int> temp(trees[m].root->edges);
            temp.insert(temp.begin(), m);
            Node* root = new Node(temp);
            result.root = root;
            break;
        }
    }
    if (!flag) {
        result.root = nullptr;
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
                        if (k == end || search(child->path, k) || trees[k].root == nullptr) {
                            continue;
                        }
                        else {
                            vector<int> ans = Find(trees[k].root, child->path);
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


int main() {
    //TreeList.reserve(1000);
    preparetion();
    for (int p = 1; p < 3; p++) {
        for (int i = 0; i < 10; i++) {
            Qtree tree = grow(i, 10, p, 5 - p, TreeList);
            TreeList2.push_back(tree);
        }
        TreeList.swap(TreeList2);
        vector<Qtree>().swap(TreeList2);
    }
    cout << "trees generated!" << endl;
    for (int i = 0; i < TreeList.size(); i++){
        cout << "tree" << i << ":" << endl;
        PrintTree(TreeList[i]);
    }
    return 0;
}