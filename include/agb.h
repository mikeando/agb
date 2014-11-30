#pragma once
#include <stdint.h>
#include <stddef.h>

//TODO: remove this? (Need to opaquify AGBMergeEntry to do so...)
#include "git2.h"

typedef struct AGBMergeEntry {
	const char * name;
	const git_tree_entry * treeentries[3];
} AGBMergeEntry;

typedef struct AGBCore AGBCore;
typedef struct AGBError AGBError;
typedef struct AGBMergeIterator AGBMergeIterator;
/* TODO: This is probably too low level to expose here? */
typedef struct AGBBranch AGBBranch;
/* TODO: This is probably too low level to expose here? */
struct AGBBranchCompare {
	size_t extra_commits_on_a;
	size_t extra_commits_on_b;
};
typedef struct AGBBranchCompare AGBBranchCompare;

typedef void(*AGBCallback)(void*);

/**
 * This is really a commit
 */
int agb_sync_files(AGBCore* anbGitBridge, AGBError * error);

/**
 * For now we only support fetching from origin.
 */
int agb_fetch(AGBCore * anbGitBridge, AGBError * error, size_t * ahead, size_t * behind );
int agb_set_fetch_callback(AGBCore * anbGitBridge, AGBCallback fetch_callback, void * userdata, AGBError * error);

/**
 * Basic set up and tear down.
 */
int agb_error_new( AGBError ** error );
int agb_bridge_new( AGBCore ** core );
int agb_core_create( AGBCore ** core, const char * path);
int agb_error_delete( AGBError * error );
int agb_bridge_delete( AGBCore * error );
const char * agb_error_message( const AGBError * error);

//TODO: Move/rename me
#include "git2.h"

//TODO: This could do with a better name.
// NOTE: Don't change these values - they need to coincide with some values from withing libgit2.
enum AGBMergeIndex {
    AGB_MERGE_LOCAL = 0,
    AGB_MERGE_REMOTE = 1,
    AGB_MERGE_BASE = 2,
};

typedef struct AGBMerger_vtable AGBMerger_vtable;
typedef struct AGBMerger AGBMerger;

struct AGBMerger {
    AGBMerger_vtable * vtable;
    AGBCore * core;
    void * user_data;
};

struct AGBMerger_vtable {
	// Called only if we can't resolve changes. (i.e. its not a simple add/change/remove)
	int (*onConflict)(AGBMerger* context, AGBMergeEntry * entry);

    // Called for every entry even if its not a conflict.
    int (*onEveryEntry)(AGBMerger* context, AGBMergeEntry * entry);

    // Called for every changed entry.
    int (*onEveryChange)(AGBMerger* context, AGBMergeEntry * entry);
    int (*onRemove)(AGBMerger* context, AGBMergeEntry * entry, enum AGBMergeIndex index);
    int (*onAdd)(AGBMerger* context, AGBMergeEntry * entry, enum AGBMergeIndex index);
    int (*onModify)(AGBMerger* context, AGBMergeEntry * entry, enum AGBMergeIndex index);

	int (*done)(AGBMerger* context);
	int (*failed)(AGBMerger * context);
};


int agb_merge( AGBMerger * merger, AGBError * error);

/**
 * Lower level merge API.
 */

static const uint32_t agb_merge_iterator_options_NONE = 0;
static const uint32_t agb_merge_iterator_options_ALL_ENTRIES  = 1;

//TODO: This should be internal.
AGBMergeIterator * 
agb_merge__create_iterator(
		const git_tree * head_tree, 
		const git_tree * branch_tree, 
		const git_tree * base_tree, 
		uint32_t merge_iterator_options);
int agb_merge_iterator_free( AGBMergeIterator * it );

int agb_merge_iterator_next( AGBMergeIterator * it);
int agb_merge_iterator_is_valid( const AGBMergeIterator * it);
AGBMergeEntry * agb_merge_entry_from_iterator(const AGBMergeIterator * it);



const git_oid * agb_merge_iterator_tree_id( const AGBMergeIterator * it, enum AGBMergeIndex i);
const char * agb_merge_entry_name( const AGBMergeEntry * it);
const git_oid * agb_merge_entry_id( const AGBMergeEntry * entry, enum AGBMergeIndex i);
git_filemode_t agb_merge_entry_filemode( const AGBMergeEntry * it, enum AGBMergeIndex i);


/**
 * Branch finding and comparing functions
 */

int agb_branch_delete( AGBBranch * branch );
int agb_branch_find( AGBCore * core , const char * name, AGBBranch ** branch, AGBError * error); 
int agb_branch_compare( const AGBBranch * branch_a, const AGBBranch * branch_b, AGBBranchCompare * result, AGBError * error);

/**
 * Utils wrapping libgit2 functions.
 */

//A NULL safe git_oid_equal
int agb_git_oid_equal(const git_oid * oid_a, const git_oid * oid_b );
