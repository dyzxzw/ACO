#pragma once
#include<iostream>
#include<vector>
using namespace std;

static const int SKIPLIST_P_VAL = RAND_MAX / 4;
static const int MAX_LEVEL = 16;

struct SkipListNode {
	int val;
	vector<SkipListNode*>nexts;
	SkipListNode(int v = -1, int s = MAX_LEVEL) :val(v), nexts(s, nullptr) {};
};

class SkipList {
private:

	SkipListNode* head;
	//SkipListNode* tail;
	int level;//Á´±í×Ü²ãÊý                                            

public:
	SkipList() {
		level = 0;
		head = new SkipListNode();
	}

	int randomLevel() {
		int lv = 1;
		while (lv < MAX_LEVEL && (rand()) < SKIPLIST_P_VAL) ++lv;
		return lv;
	}


	bool search(int target) {
		SkipListNode* node = head;
		for (int i = level - 1; i >= 0; --i) {
			while (node->nexts[i] != nullptr && target > node->nexts[i]->val) {
				node = node->nexts[i];
			}
			if (node->nexts[i] != nullptr && target == node->nexts[i]->val)
				return true;
		}
		return false;
	}


	void add(int val) {
		int rLevel = randomLevel();
		level = max(level, rLevel);
		SkipListNode* cur = new SkipListNode(val, rLevel);
		SkipListNode* node = head;
		for (int i = level - 1; i >= 0; --i) {
			while (node->nexts[i] != nullptr && val > node->nexts[i]->val) {
				node = node->nexts[i];
			}
			if (i < rLevel) {
				/*SkipListNode* next = node->nexts[i];
				node->nexts[i] = cur;
				cur->nexts[i] = next;*/

				cur->nexts[i] = node->nexts[i];
				node->nexts[i] = cur;
			}
		}
	}

	bool erase(int num) {
		SkipListNode* node = head;
		for (int i = level - 1; i >= 0; --i) {
			while (node->nexts[i] != nullptr && num > node->nexts[i]->val) {
				node = node->nexts[i];
			}
			if (node->nexts[i] != nullptr && num == node->nexts[i]->val) {
				SkipListNode* del = node->nexts[i];
				for (; i >= 0; --i) {
					while (node->nexts[i] != del)
						node = node->nexts[i];
					node->nexts[i] = node->nexts[i]->nexts[i];
					if (!head->nexts[i])
						level = i;
				}

				delete del;
				return true;
			}
		}
		return false;
	}
};