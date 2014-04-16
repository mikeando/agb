#include "anbgitbridge.h"
#include "anbgitbridge/internal/types.h"
#include <memory.h>

int anb_gitbridge_error_new( ANBGitBridgeError ** error ) {
	ANBGitBridgeError * p = malloc(sizeof(ANBGitBridgeError));

	if(!p) return 1;
	memset(p,0,sizeof(ANBGitBridgeError));

	*error = p;
	return 0;
}

int anb_gitbridge_bridge_new( ANBGitBridge ** anbGitBridge ) {
	ANBGitBridge * p = malloc(sizeof(ANBGitBridge));

	if(!p) return 1;
	memset(p,0,sizeof(ANBGitBridge));

	*anbGitBridge = p;
	return 0;
}

int anb_gitbridge_error_delete( ANBGitBridgeError * error ) {
	free((char*)error->message);
	free(error);
	return 0;
}

int anb_gitbridge_bridge_delete( ANBGitBridge * anbGitBridge ) {
	free(anbGitBridge);
	return 0;
}

int anb_gitbridge_merge_iterator_free( ANBGitBridgeMergeIterator * it ) { 
	free(it);
	return 0;
}

const char * anb_gitbridge_error_message( const ANBGitBridgeError * error) {
	return error->message;
}

