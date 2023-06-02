#pragma once
#include <iostream>
#include <vector>

using namespace std;

class Node
{
public:
	Node(const string& label) : label(label), childCnt(0) {}

	Node(const string& label, const string& value) : label(label), value(value), childCnt(0) {}

	bool addChild(Node* child) 
	{
		if (child != nullptr)
		{
			children.push_back(child);
			this->childCnt += child->getChildCnt() + 1;
			return true;
		}
		return false;
	}

	const string& getLabel() const {
		return label;
	}

	const string& getValue() const {
		return value;
	}

	const vector<Node*>& getChildren() const {
		return children;
	}

	const int getChildCnt() const {
		return childCnt;
	}

private:
	string label;            // 节点标签，如 "程序"、"分程序" 等
	string value;            // 可选的节点值，如标识符的名称、数字的值等
	vector<Node*> children;  // 子节点的列表
	int childCnt;
};
