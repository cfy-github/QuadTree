//
//  main.cpp
//  NEQuadTree
//
//  Created by fychen on 9/27/14.
//  Copyright (c) 2014 fychen. All rights reserved.
//

#include <iostream>
#include <vector>

using namespace std;

class Point{
public:
    int x;
    int y;
    friend ostream& operator<<(ostream &o, const Point &rhs);
    Point(int a, int b): x(a), y(b) {}
};

ostream& operator<<(ostream &o, const Point &rhs) {
    o<<"("<<rhs.x<<","<<rhs.y<<")";
    return o;
}

class Rect{
public:
    Rect(const Point &ld, const Point &rt);
    bool contains(const Point &p) const;
    bool intersect(const Rect &rt) const;
    Point leftDown;
    Point rightTop;
};

Rect::Rect(const Point &ld, const Point &rt): leftDown(ld), rightTop(rt) {}

bool Rect::contains(const Point &p) const {
    return (p.x <= rightTop.x) && (p.x >= leftDown.x) && (p.y >= leftDown.y) && (p.y <= rightTop.y);
}

bool Rect::intersect(const Rect &rt) const {
    Point rUp(min(rt.rightTop.x, rightTop.x), min(rt.rightTop.y, rightTop.y));
    Point lDown(max(rt.leftDown.x, leftDown.x), max(rt.leftDown.y, leftDown.y));
    return (lDown.x<=rUp.x) && (lDown.y<=rUp.y);
}

class QuadNode {
public:
    QuadNode(const Rect& rt): rect(rt) {m_pParent=m_pLeftDown=m_pLeftUp=m_pRightDown=m_pRightUp=nullptr;}
    Rect rect;
    vector<Point> objs;
    QuadNode *m_pParent;
    QuadNode *m_pLeftDown;
    QuadNode *m_pLeftUp;
    QuadNode *m_pRightDown;
    QuadNode *m_pRightUp;
};

class QuadTree{
public:
    int maxNum;
    QuadNode *root;
    QuadTree() {root=nullptr;}
    void createQuadTreeWrapper(vector<Point> &objs, const Rect &rt);
    void findObjByRect(const Rect &rt, vector<Point> &objs);
    void setMaxNum(int N);
    void printTree();
private:
    void splitSubtreeObjs(vector<Point> &objs, const Rect& rt, vector<vector<Point> > &vvObjs, vector<Rect> &vRects);
    void filterObjsByRect(vector<Point> &objs, const Rect &rt);
    void createQuadTree(QuadNode *parent, QuadNode* &root, vector<Point> &objs, const Rect &rt);
    void printTree(const QuadNode *root, int depth);
};

void QuadTree::printTree(const QuadNode *root, int depth) {
    if(root==nullptr) return;
    cout<<"<"<<depth<<">";
    if(root->objs.size() != 0) { //Leaf
        for(int i=0;i<depth;++i) cout<<"-";
        for(auto &e: root->objs) cout<<e<<" ";
        cout<<endl;
    } else {
        cout<<endl;
        printTree(root->m_pLeftDown,depth+1);
        printTree(root->m_pLeftUp, depth+1);
        printTree(root->m_pRightDown, depth+1);
        printTree(root->m_pRightUp, depth+1);
    }
}

void QuadTree::printTree() {
    if(root!=nullptr) {
        printTree(root, 1);
    }
}

void QuadTree::setMaxNum(int N) { maxNum=N; }

void QuadTree::filterObjsByRect(vector<Point> &objs, const Rect &rt) {
    for(auto it=objs.begin(); it!=objs.end(); it++) {
        if (!rt.contains(*it)) {
            it=objs.erase(it);
        }
    }
}

void QuadTree::createQuadTreeWrapper(vector<Point> &objs, const Rect &rt) {
    filterObjsByRect(objs, rt);
    createQuadTree(nullptr, root, objs, rt);
}

void QuadTree::splitSubtreeObjs(vector<Point> &objs, const Rect& rt, vector<vector<Point> > &vvObjs, vector<Rect> &vRects){
    //Split Rectangles
    int midX=(rt.leftDown.x+rt.rightTop.x)/2;
    int midY=(rt.leftDown.y+rt.rightTop.y)/2;
    vRects.push_back(Rect(rt.leftDown, Point(midX, midY))); //LeftDown
    vRects.push_back(Rect(Point(rt.leftDown.x, midY), Point(midX, rt.rightTop.y)));//LeftUp
    vRects.push_back(Rect(Point(midX, rt.leftDown.y), Point(rt.rightTop.x, midY)));//RightDown
    vRects.push_back(Rect(Point(midX, midY), rt.rightTop));//RightTop
    
    //Split Objects
    vector<Point> vLeftDown;
    vector<Point> vLeftUp;
    vector<Point> vRightDown;
    vector<Point> vRightUp;
    for(const auto &pnt: objs) {
        if(vRects[0].contains(pnt)) {
            vLeftDown.push_back(pnt);
        } else if(vRects[1].contains(pnt)) {
            vLeftUp.push_back(pnt);
        } else if(vRects[2].contains(pnt)) {
            vRightDown.push_back(pnt);
        } else if(vRects[3].contains(pnt)) {
            vRightUp.push_back(pnt);
        } else {
            //Do Nothing
        }
    }
    vvObjs.push_back(vLeftDown);
    vvObjs.push_back(vLeftUp);
    vvObjs.push_back(vRightDown);
    vvObjs.push_back(vRightUp);
}

void QuadTree::createQuadTree(QuadNode *parent, QuadNode* &root, vector<Point> &objs, const Rect &rt) {
    root=new QuadNode(rt);
    root->m_pParent=parent;
    if(objs.size()<=maxNum) {//Form a leaf
        root->objs=objs;
        return;
    } else { //Split and construct sub-QuadTrees
        vector<vector<Point> > vecSubtreeObjs;
        vector<Rect> vecSubtreeRects;
        splitSubtreeObjs(objs, rt, vecSubtreeObjs, vecSubtreeRects);
        createQuadTree(root, root->m_pLeftDown, vecSubtreeObjs[0], vecSubtreeRects[0]);
        createQuadTree(root, root->m_pLeftUp, vecSubtreeObjs[1], vecSubtreeRects[1]);
        createQuadTree(root, root->m_pRightDown, vecSubtreeObjs[2], vecSubtreeRects[2]);
        createQuadTree(root, root->m_pRightUp, vecSubtreeObjs[3], vecSubtreeRects[3]);
    }
}

void QuadTree::findObjByRect(const Rect &rt, vector<Point> &objs) {
    if(rt.intersect(root->rect)) {
        if(root->objs.size()!=0) {
            for(const auto &obj: root->objs) objs.push_back(obj);
            return;
        }
        findObjByRect(root->m_pLeftDown->rect, objs);
        findObjByRect(root->m_pLeftUp->rect, objs);
        findObjByRect(root->m_pRightDown->rect, objs);
        findObjByRect(root->m_pRightUp->rect, objs);
    }
}



int main(int argc, const char * argv[])
{
    Rect rt(Point(0,0), Point(16,16));
    vector<Point> vPnt={Point(2,10), Point(6,14), Point(4,4), Point(9,1), Point(11,3), Point(10,6), Point(14,6),Point(13,3), Point(15,1)};
    // insert code here...
    QuadTree qt;
    qt.setMaxNum(2);
    qt.createQuadTreeWrapper(vPnt, rt);
    
    qt.printTree();
    
    std::cout << "Hello, World!\n";
    return 0;
}

