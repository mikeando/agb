#pragma once
#include <stdint.h>
#include <stddef.h>
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

/**
 * High level merge API.
 * Not yet implemented.
 */
struct AGBMergeEntryFile {
	const git_oid * fileid;
	const git_oid * treeid;
	git_filemode_t mode;
};
typedef struct AGBMergeEntryFile AGBMergeEntryFile;

struct AGBMergeEntry{
	AGBMergeEntryFile * original;
	AGBMergeEntryFile * ours;
	AGBMergeEntryFile * theirs;
	const char * filename;
}
typedef AGBMergeEntry;

struct AGBMergeContext {
}
typedef AGBMergeContext;

struct AGBMergeCallbacks {
	int (*conflict)(AGBMergeContext* context, AGBMergeEntry * entry);
	int (*done)(AGBMergeContext* context);
	int (*failed)(AGBMergeContext * context);
};

typedef struct AGBMergeCallbacks AGBMergeCallbacks;

int agb_merge( AGBCore * core, AGBMergeCallbacks callbacks, AGBError * error);

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

const git_oid * agb_merge_iterator_tree_id( const AGBMergeIterator * it, int i);
const char * agb_merge_iterator_entry_name( const AGBMergeIterator * it);
const git_oid * agb_merge_iterator_entry_id( const AGBMergeIterator * it, int i);
git_filemode_t agb_merge_iterator_entry_filemode( const AGBMergeIterator * it, int i);

int agb_merge_iterator_next( AGBMergeIterator * it);
int agb_merge_iterator_is_valid( const AGBMergeIterator * it);

/**
 * Branch finding and comparing functions
 */

int agb_branch_delete( AGBBranch * branch );
int agb_branch_find( AGBCore * core , const char * name, AGBBranch ** branch, AGBError * error); 
int agb_branch_compare( const AGBBranch * branch_a, const AGBBranch * branch_b, AGBBranchCompare * result, AGBError * error);

