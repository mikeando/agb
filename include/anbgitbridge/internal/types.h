#pragma once
#include "git2.h"

struct ANBGitBridge {
	git_repository * repository;
	const char * origin_name; // Usually "origin
};

struct ANBGitBridgeError {
	const char * message;
	int error_code_git;
	int error_code;
};

