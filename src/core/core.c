#include "anbgitbridge.h"
#include "anbgitbridge/internal/types.h"
#include <memory.h>

//TODO: Handle allocation errors
int anb_gitbridge_error_new( ANBGitBridgeError ** error ) {
	*error = malloc(sizeof(ANBGitBridgeError));
	memset(*error,0,sizeof(ANBGitBridgeError));
	return 0;
}

int anb_gitbridge_bridge_new( ANBGitBridge ** anbGitBridge ) {
	*anbGitBridge= malloc(sizeof(ANBGitBridge));
	memset(*anbGitBridge,0,sizeof(ANBGitBridge));
	return 0;

}

int anb_gitbridge_error_delete( ANBGitBridgeError * error ) {
	free(error);
	return 0;
}

int anb_gitbridge_bridge_delete( ANBGitBridge * anbGitBridge ) {
	free(anbGitBridge);
	return 0;
}

const char * anb_gitbridge_error_message( const ANBGitBridgeError * error) {
	return error->message;
}

