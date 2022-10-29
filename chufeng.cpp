#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <windows.h>
using namespace std;

struct Node {
    bool flag = true;
    vector<int> edges;
    vector<int> path;
    vector<Node*> children;
    Node(vector<int> _edges) :edges(_edges) {}
    Node(){}
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
            Node* node = new Node({});
            TreeList.push_back(Qtree(node));
        }
        else {
            TreeList.push_back(Qtree(nullptr));
        }
    }
}


void growth() {
    for (int i = 0; i < 10; i++) {
        bool flag = false;
        for (int j : adjList[i]) {
            try {
                if (TreeList[j].root == nullptr) {
                    continue;
                }
                else {
                    flag = true;
                    vector<int> temp(TreeList[j].root->edges);
                    temp.push_back(j);
                    Node* node = new Node(temp);
                    TreeList2.push_back(Qtree(node));
                    break;
                }
            }
            catch (exception & e) {
                cout << e.what() << endl;
            }
        }
        if (!flag) {
            TreeList2.push_back(Qtree(nullptr));
        }
        else {
        }
    }
}


void growth1(int start, int p, int q, Qtree t) {
    Qtree ans;
    queue<Node*> QtNode;
    Node* root;
    QtNode.push(root);
    int floor = 1;
    while (!QtNode.empty() && floor <= q) {
        int count = QtNode.size();
        //记录当前层次
        for (int i = 0; i < count; i++) {
            Node* node = QtNode.front();
            QtNode.pop();
            //为结点生成p个孩子
            if (!node->edges.empty()) {
                for (int j = 0; j < p; j++) {
                    Node* child = new Node();
                    child->path = node->path;
                    child->path.push_back(node->edges[j]);
                    for (int k : adjList[start]) {
                        if (search(child->path, k)){
                            continue;
                        }
                        else {
                            vector<int> ans = Find(TreeList[k].root, child->path);
                            if (!ans.empty()) {
                                ans.push_back(k);
                                child->edges = ans;
                                QtNode.push(child);
                                break;
                            }
                        }
                    }
                    node->children[i] = child;
                }
            }
            
        }
        floor++;
    }    
}


int main() {
    TreeList.reserve(1000);
    preparetion();
    growth();
    cout << "over!" << endl;
    for (int i = 0; i < 10; i++) {
        if (TreeList2[i].root == nullptr) {
        }
        else {
            cout << "第" << i << "棵树根节点为:" << TreeList2[i].root->edges[0] << endl;
        }
    }
    return 0;
}