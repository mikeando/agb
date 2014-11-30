#pragma once
#include "git2.h"

struct AGBCore {
	git_repository * repository;
	const char * origin_name; // Usually "origin

	AGBCallback fetch_callback;
	void* fetch_callback_userdata;
    char const *local_branch_name;
    char const *remote_branch_name;
};

struct AGBError {
	const char * message;
	int error_code_git;
	int error_code;
};


struct AGBMergeIterator {
	const git_tree * trees[3];
	AGBMergeEntry * entries;
	int n_entries;
	int idx;
};

struct AGBBranch {
	AGBCore * core;
	git_reference * ref;
	git_oid * oid;
};

