typedef struct ANBGitBridge ANBGitBridge;
typedef struct ANBGitBridgeError ANBGitBridgeError;
typedef struct ANBGitBridgeMergeIterator ANBGitBridgeMergeIterator;

typedef void(*ANBGitBridgeCallback)(void*);

int anb_git_bridge_sync_files(ANBGitBridge* anbGitBridge, ANBGitBridgeError * error);
/**
 * For now we only support fetching from origin.
 */
int anb_git_bridge_fetch(ANBGitBridge * anbGitBridge, ANBGitBridgeError * error);
int anb_git_bridge_set_fetch_callback(ANBGitBridge * anbGitBridge, ANBGitBridgeCallback fetch_callback, void * userdata, ANBGitBridgeError * error);

int anb_gitbridge_error_new( ANBGitBridgeError ** error );
int anb_gitbridge_bridge_new( ANBGitBridge ** error );
int anb_gitbridge_error_delete( ANBGitBridgeError * error );
int anb_gitbridge_bridge_delete( ANBGitBridge * error );
const char * anb_gitbridge_error_message( const ANBGitBridgeError * error);


//TODO: Move/rename me
#include "git2.h"
ANBGitBridgeMergeIterator * create_merge_iterator(git_tree * head_tree, git_tree * branch_tree, git_tree * base_tree);
int anb_gitbridge_merge_iterator_free( ANBGitBridgeMergeIterator * it );
