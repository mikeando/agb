typedef struct AGBCore AGBCore;
typedef struct AGBError AGBError;
typedef struct AGBMergeIterator AGBMergeIterator;

typedef void(*AGBCallback)(void*);

int agb_sync_files(AGBCore* anbGitBridge, AGBError * error);
/**
 * For now we only support fetching from origin.
 */
int agb_fetch(AGBCore * anbGitBridge, AGBError * error);
int agb_set_fetch_callback(AGBCore * anbGitBridge, AGBCallback fetch_callback, void * userdata, AGBError * error);

int agb_error_new( AGBError ** error );
int agb_bridge_new( AGBCore ** error );
int agb_error_delete( AGBError * error );
int agb_bridge_delete( AGBCore * error );
const char * agb_error_message( const AGBError * error);


//TODO: Move/rename me
#include "git2.h"

static const uint32_t agb_merge_iterator_options_NONE = 0;
static const uint32_t agb_merge_iterator_options_ALL_ENTRIES  = 1;

AGBMergeIterator * create_merge_iterator(const git_tree * head_tree, const git_tree * branch_tree, const git_tree * base_tree, uint32_t merge_iterator_options);
int agb_merge_iterator_free( AGBMergeIterator * it );
const git_oid * agb_merge_iterator_head_id( AGBMergeIterator * it);
const git_oid * agb_merge_iterator_branch_id( AGBMergeIterator * it);
const git_oid * agb_merge_iterator_base_id( AGBMergeIterator * it);
const char * agb_merge_iterator_entry_name( AGBMergeIterator * it);
const git_oid * agb_merge_iterator_head_entry_id( AGBMergeIterator * it);
const git_oid * agb_merge_iterator_branch_entry_id( AGBMergeIterator * it);
const git_oid * agb_merge_iterator_base_entry_id( AGBMergeIterator * it);
git_filemode_t agb_merge_iterator_head_filemode( AGBMergeIterator * it);
git_filemode_t agb_merge_iterator_branch_filemode( AGBMergeIterator * it);
git_filemode_t agb_merge_iterator_base_filemode( AGBMergeIterator * it);

int agb_merge_iterator_next( AGBMergeIterator * it);
int agb_merge_iterator_is_valid( AGBMergeIterator * it);
