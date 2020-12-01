// Cache Simulator with 3 different replacement policies:
// Least Recently Used, Not Most Recently Used, and Random Replacement
// Tests the hit rate of a policy.

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime> 
using namespace std;

#define LENGTH 1000000
#define ITERAT 1000000
#define SLINESIZE 64
#define MAX_ASSOC 16
#define MAX_SET_NUM 8192
#define BANK_BIT_0 6
#define BANK_OFFSET 1
#define HIT_MISS_NUM 2


typedef unsigned long long LARGEINT;

typedef struct element {
	char Ncc;
	LARGEINT Address;
} element_t;

typedef enum {
	ACCESS_TYPE_LOAD,
	ACCESS_TYPE_STORE,
	ACCESS_TYPE_WRITEBACK
} ACCESS_TYPE;

typedef struct CACHE_SET_LRU_PO {
	LARGEINT _tags[MAX_ASSOC];
	int _repl[MAX_ASSOC];
	LARGEINT  _addr[MAX_ASSOC];
	bool _clean[MAX_ASSOC];
} SET_t;

typedef struct CACHE {
	int _setNum;
	CACHE_SET_LRU_PO _sets[MAX_SET_NUM];
	CACHE* below;
	bool conflict;
	int _cacheId;
	int _cacheSize;
	int _lineSize;
	int _associativity;
	int _lineShift;
	int _setIndexMask;
	LARGEINT _access[HIT_MISS_NUM];
} CACHE_t;

bool IsPower2(int n) {
	return ((n & (n - 1)) == 0);
}

int FloorLog2(int n) {
	int p = 0;
	if (n == 0) return -1;
	if (n & 0xffff0000) {
		p += 16;
		n >>= 16;
	}
	if (n & 0x0000ff00) {
		p +=  8;
		n >>=  8;
	}
	if (n & 0x000000f0) {
		p +=  4;
		n >>=  4;
	}
	if (n & 0x0000000c) {
		p +=  2;
		n >>=  2;
	}
	if (n & 0x00000002) {
		p +=  1;
	}
	return p;
}

int CeilLog2(int n) {
	return FloorLog2(n - 1) + 1;
}

void SplitAddress(CACHE_t* cc, LARGEINT addr, LARGEINT* tag, int* setIndex) {
	*tag = addr >> cc->_lineShift;
	*setIndex = *tag & cc->_setIndexMask;
}

void d2b(char* st,  char tar) {
	for (int j = 0; j < 8; j++) {
		st[j] = 0;
	}
	int i = 0;
	unsigned char mtar = (unsigned char)tar;
	while (mtar > 0) {
		st[i] = mtar % 2;
		mtar = mtar / 2;
		i++;
	}
}

int Find(SET_t * tar, LARGEINT tag, int assoc) {
	for (int way = 0; way < assoc; way++) {
		if(tar->_tags[way] == tag) {
			return way;
		}
	}
	return -1;
}

void Update(SET_t * tar, int way, int assoc) {
	assert(way < assoc);
	int currentp = tar->_repl[way];
	for(int i=0; i < assoc; i++) {
		if( tar->_repl[i] < currentp ) {
			tar->_repl[i] ++;
		}
	}
	tar->_repl[way] = 0;
}

int Replace(SET_t* tar, LARGEINT tag, int assoc) {
	int lruWay = 0;
	for (int way = 0; way < assoc; way++) {
		if(tar->_repl[way] == (assoc-1)) {
			lruWay = way;
			tar->_tags[lruWay] = tag;
			break;
		}
	}
	return lruWay;
}

int Replace_NMRU(SET_t* tar, LARGEINT tag, int assoc) {
	int lruWay = 0;
	for (int way = 0; way < assoc; way++) {
		if(tar->_repl[way] != 0) {  
			lruWay = way;
			tar->_tags[lruWay] = tag;
			break;
		}
	}
	return lruWay;
}

int Replace_Random(SET_t* tar, LARGEINT tag, int assoc) {
    srand((unsigned) time(0));
    int random_index = (rand % assoc) 
    tar->_tags[random_index] = tag;
    return random_index;
}

void AccessSingleLine(CACHE_t* cc, LARGEINT addr, ACCESS_TYPE accessType, bool dirty) {
	LARGEINT tag;
	int setIndex;
	SplitAddress(cc, addr, &tag, &setIndex);
	assert(setIndex < cc->_setNum);
	assert(setIndex >= 0);
	CACHE_SET_LRU_PO * set = &(cc->_sets[setIndex]);
	assert(set != NULL);
	long index = Find(set, tag, cc->_associativity);

	bool hit = true;
	if(index == -1) hit = false;

	if (!hit) {
		int way = Replace(set, tag, cc->_associativity);
		bool clean = set->_clean[way];
		LARGEINT bAddr = set->_addr[way];

		if(cc->below != NULL) {
			AccessSingleLine(cc->below, bAddr, ACCESS_TYPE_WRITEBACK, clean);
		} 

		if(cc->below != NULL && accessType != ACCESS_TYPE_WRITEBACK) {
			AccessSingleLine(cc->below, addr, ACCESS_TYPE_LOAD, true);
		} 

		Update(set, way, cc->_associativity);
		set->_addr[way]= addr;

		if(accessType == ACCESS_TYPE_STORE) set->_clean[way] = false;
		if(accessType == ACCESS_TYPE_LOAD) set->_clean[way] = true;
		if(accessType == ACCESS_TYPE_WRITEBACK) set->_clean[way] = dirty;
	} else {
		Update(set, index, cc->_associativity);

		if(accessType == ACCESS_TYPE_STORE) set->_clean[index] = false;
		if(accessType == ACCESS_TYPE_WRITEBACK && dirty == false) set->_clean[index] = dirty;
	}
	cc->_access[hit] ++;
}

void initialCache(CACHE_t *mm, int cid, int cs, int ls, int so, CACHE_t* be) {
	mm->conflict = true;
	mm->below = be;
	mm->_cacheId = cid;
	mm->_associativity = so;
	mm->_cacheSize = cs;
	mm->_lineSize = ls;
	mm->_lineShift = FloorLog2(mm->_lineSize);
	mm->_setIndexMask = (mm->_cacheSize / (mm->_associativity * mm->_lineSize)) - 1;
	mm->_setNum = mm->_setIndexMask + 1;

	assert(IsPower2(mm->_lineSize));
	assert(IsPower2(mm->_setIndexMask + 1));

	for(int i = 0; i < MAX_SET_NUM; i++) {
		for(int j = 0; j < MAX_ASSOC; j++) {
			mm->_sets[i]._tags[j] = 0;
			mm->_sets[i]._repl[j] = j;
			mm->_sets[i]._addr[j] = 0;
			mm->_sets[i]._clean[j] = false;
		}
	}

	for(int i =0; i < HIT_MISS_NUM; i++) {
		mm->_access[i] = 0;
	}
}

void restoreCache(CACHE_t *l1, CACHE_t *l2, CACHE_t *l3, ifstream* inputFile) {
	inputFile->seekg(0, ios::beg);
	inputFile->read((char*)l1->_sets, sizeof(SET_t) * l1->_setNum);
	streamoff offset = sizeof(SET_t) * l1->_setNum;
	inputFile->seekg(offset, ios::beg);
	inputFile->read((char*)l2->_sets, sizeof(SET_t) * l2->_setNum);
	offset = sizeof(SET_t) * (l1->_setNum + l2->_setNum);
	inputFile->seekg(offset, ios::beg);
	inputFile->read((char*)l3->_sets, sizeof(SET_t) * l3->_setNum);
	inputFile->close();
}


int main(int argc, char *argv[]) {
	ifstream filein0 (argv[1], ios::in|ios::binary);

	CACHE_t* L3 = (CACHE_t *)malloc(sizeof(CACHE_t));
	initialCache(L3, 2, 8388608, 64, 16, NULL);
	CACHE_t* L2 = (CACHE_t *)malloc(sizeof(CACHE_t));
	initialCache(L2, 1, 262144, 64, 8, L3);
	CACHE_t* DL = (CACHE_t *)malloc(sizeof(CACHE_t));
	initialCache(DL, 0, 32768, 64, 4, L2);
	restoreCache(DL, L2, L3, &filein0);

	ifstream filein1 (argv[2], ios::in|ios::binary);

	streamoff offset = 0;
	if (filein1.is_open()) {
		element_t* sample = (element_t *) malloc(sizeof(element_t) * LENGTH);
		filein1.seekg(offset, ios::beg);
		filein1.read((char*)sample, sizeof(element_t) * LENGTH);

		for(int j = 0; j < LENGTH; j++) {
			if(sample[j].Ncc) {
				AccessSingleLine(DL, sample[j].Address, ACCESS_TYPE_LOAD, true);
			} else {
				AccessSingleLine(DL, sample[j].Address, ACCESS_TYPE_STORE, false);
			}
		}

		cout << "L1 Hit: " << DL->_access[true] / (double) (DL->_access[true] + DL->_access[false]) 
			<< " " << (DL->_access[true] + DL->_access[false]) << endl;
		cout << "L2 Hit: " << L2->_access[true] / (double) (L2->_access[true] + L2->_access[false]) 
			<< " " << (L2->_access[true] + L2->_access[false]) << endl;
		cout << "L3 Hit: " << L3->_access[true] / (double) (L3->_access[true] + L3->_access[false]) 
			<< " " << (L3->_access[true] + L3->_access[false]) << endl;

		DL->_access[true] = 0;
		DL->_access[false] = 0;
		L2->_access[true] = 0;
		L2->_access[false] = 0;
		L3->_access[true] = 0;
		L3->_access[false] = 0;

		filein1.close();
		free(sample);
	}

	free(DL);
	free(L2);
	free(L3);
	return 0;
}