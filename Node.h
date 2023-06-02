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
	string label;            // �ڵ��ǩ���� "����"��"�ֳ���" ��
	string value;            // ��ѡ�Ľڵ�ֵ�����ʶ�������ơ����ֵ�ֵ��
	vector<Node*> children;  // �ӽڵ���б�
	int childCnt;
};
