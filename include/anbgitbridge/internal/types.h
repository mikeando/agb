#pragma once
#include "git2.h"

struct ANBGitBridge {
	git_repository * repository;
	const char * origin_name; // Usually "origin

	ANBGitBridgeCallback fetch_callback;
	void* fetch_callback_userdata;
};

struct ANBGitBridgeError {
	const char * message;
	int error_code_git;
	int error_code;
};

typedef struct ANBGitBridgeMergeIteratorEntry {
	const char * name;
	const git_oid * ids[3];
} ANBGitBridgeMergeIteratorEntry;

struct ANBGitBridgeMergeIterator {
	git_tree * head_tree;
	git_tree * branch_tree;
	git_tree * base_tree;

	ANBGitBridgeMergeIteratorEntry * entries;
	int n_entries;
	int idx;
};

