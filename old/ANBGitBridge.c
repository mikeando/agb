/*
 * ANBGritBridge.c
 *
 *  Created on: Feb 27, 2014
 *      Author: michaelanderson
 */

/**
 * What can we do with the ANBGitBridge?
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

#include "ANBGitBridge.h"

//TODO: Mostly this is just to get NULL, maybe we can use a "smaller" include here instead.
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//Struct Forward declarations;

#include "git2.h"
typedef git_oid ANBGitBridgeID;

struct ANBGitBridgeTree;
typedef struct ANBGitBridgeTree ANBGitBridgeTree;

struct ANBGitBridgeTreeEntry;
typedef struct ANBGitBridgeTreeEntry ANBGitBridgeTreeEntry;




//Not sure these should be callbacks, maybe structs instead?
typedef void(*ANBGitBridgeTreeComparer)(char* name, int nEntries, ANBGitBridgeTreeEntry** entries, void* userdata);

//Function declarations
void ANBGitBridge_fetchRemote(ANBGitBridge* config, ANBGitBridgeFetchCallback callback);
void ANBGitBridge_compare(ANBGitBridge* config, ANBGitBridgeTreeComparer * comparer, void * user_data);
ANBGitBridgeTree * ANBGitBridge_getTree( ANBGitBridge* config, ANBGitBridgeID *treeID);
void ANBGitBridge_releaseTree(ANBGitBridge* config, ANBGitBridgeTree* tree);


void ANBGitBridge_tree_compare(
		ANBGitBridgeID ** treeids,
		int nTrees,
		ANBGitBridge* config,
		ANBGitBridgeTreeComparer callback,
		void * user_data);

//Struct Definiations

struct ANBGitBridgeTree {
	int nEntries;
	ANBGitBridgeTreeEntry * entries;
};

struct ANBGitBridgeTreeEntry {
	char * name;
	ANBGitBridgeID *id;
};





void anb_git_bridge_init() {
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

char * getMinName(ANBGitBridgeTreeEntry ** entries, int nEntries) {
	if(nEntries==0) return NULL;
	char * minName = entries[0]->name;
	for(int i=1; i<nEntries; ++i) {
		if(strcmp(entries[i]->name,minName)<0) {
			minName = entries[i]->name;
		}
	}
	return minName;
}

void getEntriesNamed(ANBGitBridgeTreeEntry ** entries, int nEntries, char *name, ANBGitBridgeTreeEntry ** out_entries) {
	for(int i=0; i<nEntries; ++i) {
		if(strcmp(entries[i]->name,name)==0) {
			out_entries[i]=entries[i];
		} else {
			out_entries[i] = NULL;
		}
	}
}

//TODO: This is probably an OK spot to start using libgit2 internals
void ANBGitBridge_tree_compare(
		ANBGitBridgeID ** treeids,
		int nTrees,
		ANBGitBridge* config,
		ANBGitBridgeTreeComparer callback,
		void * user_data) {

	ANBGitBridgeTree ** trees = (ANBGitBridgeTree**)malloc(sizeof(ANBGitBridgeTree*)*nTrees);
	int* treeIndices = (int*)malloc(sizeof(int)*nTrees);
	int* nTreeElements = (int*)malloc(sizeof(int)*nTrees);

	//These are trees[i].entries[ treeIndices[i] ] or NULL;
	ANBGitBridgeTreeEntry ** curTreeEntries = (ANBGitBridgeTreeEntry**)malloc(sizeof(ANBGitBridgeTreeEntry*)*nTrees);

	//These are the tree entries in curTreeEntries with the minimum name.
	ANBGitBridgeTreeEntry ** treeEntries = (ANBGitBridgeTreeEntry**)malloc(sizeof(ANBGitBridgeTreeEntry*)*nTrees);

	//Initialize the curTreeEntries
	int i;
	for(i=0; i<nTrees; ++i) {
		trees[i] = ANBGitBridge_getTree(config,treeids[i]);
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


	//The  ANBGitBridgeTreeEntry values are owned by the trees. So we only need to release the trees
	for(i=0;i<nTrees;++i) {
		ANBGitBridge_releaseTree(config,trees[i]);
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
void keep(char * name, ANBGitBridgeTreeEntry* entry) { }
void conflict(char * name) { }

int are_same(ANBGitBridgeTreeEntry* e1, ANBGitBridgeTreeEntry * e2) {

	if(e1==NULL && e2==NULL) {
		return 1;
	}

	if( ( (e1==NULL) && (e2!=NULL) ) || ( (e1!=NULL) && (e2==NULL) ) ) {
		return 0;
	}

	return git_oid_equal(e1->id, e2->id);
}

void my_3_merge_callback(char * name, int nEntries, ANBGitBridgeTreeEntry** entries, void * user_data) {
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
ANBGitBridgeID* ANBGitBridge_findCommonRoot(ANBGitBridgeID * branch1, ANBGitBridgeID * branch2);

void MyMerge(ANBGitBridge * config, ANBGitBridgeID * mine, ANBGitBridgeID * theirs) {
	ANBGitBridgeID * rootID = ANBGitBridge_findCommonRoot(mine,theirs);
	if(rootID==NULL) {
		//Can't merge no common ancestor?
		//Or fall back to a two head merge
		return;
	}

	ANBGitBridgeID* ids[3] = {mine, theirs, rootID};

	my_3_merge_callback_data data;

	ANBGitBridge_tree_compare(ids, 3, config, my_3_merge_callback, &data);
}

//TODO: Implement me
ANBGitBridgeTree * ANBGitBridge_getTree( ANBGitBridge* config, ANBGitBridgeID *treeID){
	return NULL;
}

//TODO: Implement me
void ANBGitBridge_releaseTree(ANBGitBridge* config, ANBGitBridgeTree* tree) {
}

//TODO: Implement me
ANBGitBridgeID* ANBGitBridge_findCommonRoot(ANBGitBridgeID * branch1, ANBGitBridgeID * branch2) {
	return NULL;
}

void anb_git_bridge_error_init(ANBGitBridgeError * e) {
	e->error_code = 0;
	e->message = NULL;
}

void anb_git_bridge_error_free(ANBGitBridgeError * e) {
	free((void*)e->message);
}

