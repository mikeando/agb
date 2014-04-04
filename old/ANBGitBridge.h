#ifndef ANBGITBRIDGE_H
#define ANBGITBRIDGE_H

#include "git2.h"

void anb_git_bridge_init();


struct ANBGitBridge;
typedef struct ANBGitBridge ANBGitBridge;

struct ANBGitBridge {
	git_repository * repository;
	const char * origin_name; // Usually "origin
};

typedef struct ANBGitBridgeError {
	const char * message;
	int error_code;
} ANBGitBridgeError;

void anb_git_bridge_error_init(ANBGitBridgeError * e);
void anb_git_bridge_error_free(ANBGitBridgeError * e);

//Callback typedefs

typedef void(*ANBGitBridgeFetchCallback)(ANBGitBridgeError*, void* userdata);


#endif
