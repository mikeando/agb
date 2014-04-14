#include "anbgitbridge.h"
#include "anbgitbridge/internal/types.h"
#include "anbgitbridge/internal/eh.h"
#include <stdio.h>


static int update_cb(const char *refname, const git_oid *a, const git_oid *b, void *data) {
	return 0;
}
static int progress_cb(const char *str, int len, void *data) {
	ANBGitBridge * anbGitBridge = (ANBGitBridge*)data;
	if(anbGitBridge->fetch_callback) {
		(*anbGitBridge->fetch_callback)(anbGitBridge->fetch_callback_userdata);
	}
	return 0;
}

int cred_acquire_cb(git_cred **out,
		const char * url,
		const char * username_from_url,
		unsigned int allowed_types,
		void * payload) {
	char username[128] = {0};
	char password[128] = {0};

	printf("Username: ");
	scanf("%s", username);

	/* Yup. Right there on your terminal. Careful where you copy/paste output. */
	printf("Password: ");
	scanf("%s", password);

	return git_cred_userpass_plaintext_new(out, username, password);
}

int anb_git_bridge_fetch(ANBGitBridge * anbGitBridge, ANBGitBridgeError * error) {
	git_repository * repo = anbGitBridge->repository;
	const char * origin_name = anbGitBridge->origin_name;
	int ok;

	//Get the remote
	git_remote * remote = NULL;
	ok = git_remote_load(&remote, repo, origin_name);
	if(ok!=0) {
		anb_git_bridge__error_translate(error,"git_remote_load failed",ok);
		goto cleanup_error;
	}

	// Set up the callbacks (only update_tips for now)
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	callbacks.update_tips = &update_cb;
	callbacks.progress = &progress_cb;
	callbacks.credentials = cred_acquire_cb;
	callbacks.payload = anbGitBridge;
	git_remote_set_callbacks(remote, &callbacks);

	//TODO: Run this on a worker thread? Like in the fetch.c example

	// Connect to the remote end specifying that we want to fetch
	// information from it.
	//
	ok = git_remote_connect(remote, GIT_DIRECTION_FETCH);
	if (ok!=0) {
		anb_git_bridge__error_translate(error, "git_remote_connect failed",ok);
		goto cleanup_error;
	}

	// Download the packfile and index it. This function updates the
	// amount of received data and the indexer stats which lets you
	// inform the user about progress.
	ok = git_remote_download(remote); 
	if (ok!=0) {
		anb_git_bridge__error_translate(error, "git_remote_download failed",ok);
		goto cleanup_error;
	}

	// Disconnect the underlying connection to prevent from idling.
	git_remote_disconnect(remote);

	// Update the references in the remote's namespace to point to the
	// right commits. This may be needed even if there was no packfile
	// to download, which can happen e.g. when the branches have been
	// changed but all the neede objects are available locally.
	ok=git_remote_update_tips(remote, NULL, NULL);
	if (ok!=0) {
		anb_git_bridge__error_translate(error, "git_remote_download failed",ok);
		goto cleanup_error;
	}

	git_remote_free(remote);
	return 0;

cleanup_error:
	
	if(remote) git_remote_free(remote);
	return error->error_code;
}

int anb_git_bridge_set_fetch_callback(ANBGitBridge * anbGitBridge, ANBGitBridgeCallback fetch_callback, void * userdata, ANBGitBridgeError * error) {
	anbGitBridge->fetch_callback = fetch_callback;
	anbGitBridge->fetch_callback_userdata = userdata;
	return 0;
}

