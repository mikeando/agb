/*
 * ANBGritBridge.c
 *
 *  Created on: Feb 27, 2014
 *      Author: michaelanderson
 */

/**
 * What can we do with the AGBCore?
 *
 * Fetch a remote branch
 *
 * Compare the current branch with the remote
 *
 * Fast forward merge remote into current
 *
 * Fast forward merge/push current into remote
 *
 * Merge differences.
 */

#include "AGBCore.h"

//TODO: Mostly this is just to get NULL, maybe we can use a "smaller" include here instead.
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//Struct Forward declarations;

#include "git2.h"
typedef git_oid AGBID;

struct AGBTree;
typedef struct AGBTree AGBTree;

struct AGBTreeEntry;
typedef struct AGBTreeEntry AGBTreeEntry;




//Not sure these should be callbacks, maybe structs instead?
typedef void(*AGBTreeComparer)(char* name, int nEntries, AGBTreeEntry** entries, void* userdata);

//Function declarations
void AGB_fetchRemote(AGBCore* config, AGBFetchCallback callback);
void AGB_compare(AGBCore* config, AGBTreeComparer * comparer, void * user_data);
AGBTree * AGB_getTree( AGBCore* config, AGBID *treeID);
void AGB_releaseTree(AGBCore* config, AGBTree* tree);


void AGB_tree_compare(
		AGBID ** treeids,
		int nTrees,
		AGBCore* config,
		AGBTreeComparer callback,
		void * user_data);

//Struct Definiations

struct AGBTree {
	int nEntries;
	AGBTreeEntry * entries;
};

struct AGBTreeEntry {
	char * name;
	AGBID *id;
};





void agb_init() {
	git_threads_init();
}

//TODO: Fix this up
#define TRUE 1
#define FALSE 0

int atleastOneNotDone(int * treeIndices, int * nTreeElements, int nTrees) {
	for(int i=0; i<nTrees; ++i) {
		if(treeIndices[i]<nTreeElements[i]) return TRUE;
	}
	return FALSE;
}

char * getMinName(AGBTreeEntry ** entries, int nEntries) {
	if(nEntries==0) return NULL;
	char * minName = entries[0]->name;
	for(int i=1; i<nEntries; ++i) {
		if(strcmp(entries[i]->name,minName)<0) {
			minName = entries[i]->name;
		}
	}
	return minName;
}

void getEntriesNamed(AGBTreeEntry ** entries, int nEntries, char *name, AGBTreeEntry ** out_entries) {
	for(int i=0; i<nEntries; ++i) {
		if(strcmp(entries[i]->name,name)==0) {
			out_entries[i]=entries[i];
		} else {
			out_entries[i] = NULL;
		}
	}
}

//TODO: This is probably an OK spot to start using libgit2 internals
void AGB_tree_compare(
		AGBID ** treeids,
		int nTrees,
		AGBCore* config,
		AGBTreeComparer callback,
		void * user_data) {

	AGBTree ** trees = (AGBTree**)malloc(sizeof(AGBTree*)*nTrees);
	int* treeIndices = (int*)malloc(sizeof(int)*nTrees);
	int* nTreeElements = (int*)malloc(sizeof(int)*nTrees);

	//These are trees[i].entries[ treeIndices[i] ] or NULL;
	AGBTreeEntry ** curTreeEntries = (AGBTreeEntry**)malloc(sizeof(AGBTreeEntry*)*nTrees);

	//These are the tree entries in curTreeEntries with the minimum name.
	AGBTreeEntry ** treeEntries = (AGBTreeEntry**)malloc(sizeof(AGBTreeEntry*)*nTrees);

	//Initialize the curTreeEntries
	int i;
	for(i=0; i<nTrees; ++i) {
		trees[i] = AGB_getTree(config,treeids[i]);
		treeIndices[i] = 0;
		nTreeElements[i] = trees[i]->nEntries;
		if(nTreeElements[i]>0) {
			curTreeEntries[i] = trees[i]->entries+0;
		} else {
			curTreeEntries[i] = NULL;
		}
	}


	while( atleastOneNotDone(treeIndices,nTreeElements,nTrees)) {
		char* minName = getMinName(curTreeEntries,nTrees);
		getEntriesNamed(treeEntries, nTrees, minName, curTreeEntries);
		(*callback)(minName, nTrees, curTreeEntries, user_data);
	}


	//The  AGBTreeEntry values are owned by the trees. So we only need to release the trees
	for(i=0;i<nTrees;++i) {
		AGB_releaseTree(config,trees[i]);
	}

	free(trees);
	free(treeIndices);
	free(nTreeElements);
	free(curTreeEntries);
	free(treeEntries);
}

//So now we can implement a merge based on compare

typedef struct my_3_merge_callback_data {

} my_3_merge_callback_data;


//TODO: Implement these
void keep(char * name, AGBTreeEntry* entry) { }
void conflict(char * name) { }

int are_same(AGBTreeEntry* e1, AGBTreeEntry * e2) {

	if(e1==NULL && e2==NULL) {
		return 1;
	}

	if( ( (e1==NULL) && (e2!=NULL) ) || ( (e1!=NULL) && (e2==NULL) ) ) {
		return 0;
	}

	return git_oid_equal(e1->id, e2->id);
}

void my_3_merge_callback(char * name, int nEntries, AGBTreeEntry** entries, void * user_data) {
	assert(nEntries=3);

	if( !are_same(entries[0],entries[2] ) ) {
		//There are changes on our branch (and maybe theirs)
		if(!are_same(entries[1],entries[2])) {
			// There are changes on their branch too...
			// ... this will usually be a conflict, unless we've
			// both made the same change
			if(are_same(entries[0],entries[1])) {
				// Both made the same change, lets keep ours.
				// (Only really matters for timestamps etc.)
				keep(name,entries[0]);

				return;
			}

			//...CONFLICT...
			// TODO: If they're TREE entries we can just recurse into them and repeat
			//       otherwise its a real conflict.

			conflict(name);
			return;
		}

		//Change is only on our branch
		keep(name,entries[0]);
		return;
	}


	if(!are_same(entries[1],entries[2])) {
		//There are changes on their branch (but not ours)
		keep(name, entries[1]);
		return;
	}

	// No changes on our branch, and no changes on theirs.
	// Better keep ours.

	keep(name,entries[0]);
}


//TODO: Move & implement this
AGBID* AGB_findCommonRoot(AGBID * branch1, AGBID * branch2);

void MyMerge(AGBCore * config, AGBID * mine, AGBID * theirs) {
	AGBID * rootID = AGB_findCommonRoot(mine,theirs);
	if(rootID==NULL) {
		//Can't merge no common ancestor?
		//Or fall back to a two head merge
		return;
	}

	AGBID* ids[3] = {mine, theirs, rootID};

	my_3_merge_callback_data data;

	AGB_tree_compare(ids, 3, config, my_3_merge_callback, &data);
}

//TODO: Implement me
AGBTree * AGB_getTree( AGBCore* config, AGBID *treeID){
	return NULL;
}

//TODO: Implement me
void AGB_releaseTree(AGBCore* config, AGBTree* tree) {
}

//TODO: Implement me
AGBID* AGB_findCommonRoot(AGBID * branch1, AGBID * branch2) {
	return NULL;
}

void agb_error_init(AGBError * e) {
	e->error_code = 0;
	e->message = NULL;
}

void agb_error_free(AGBError * e) {
	free((void*)e->message);
}

