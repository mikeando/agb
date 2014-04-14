#include "anbgitbridge.h"
#include "anbgitbridge/internal/eh.h"
#include "anbgitbridge/internal/types.h"
#include <stdio.h>

int anb_git_bridge__error_translate(ANBGitBridgeError * error, const char * message, int errcode) {
	//TODO: Do something with the git error info and plug it into the error object
	const char * git_err_message = NULL;
	if(errcode==0) {
		git_err_message = "no error.";
		error->error_code = 0;
		error->error_code_git = 0;
	}
	if(errcode==GIT_ENOTFOUND) {
		git_err_message = "remote not found.";
		error->error_code = 1;
		error->error_code_git = GIT_ENOTFOUND;
	}
	else if(errcode==GIT_EINVALIDSPEC) {
		git_err_message = "remote has invalid spec.";
		error->error_code = 1;
		error->error_code_git = GIT_EINVALIDSPEC;
	}
	else if (errcode < 0) {
		git_err_message = giterr_last()->message;
		error->error_code = 1;
		error->error_code_git = errcode;
	}
	else {
		git_err_message = "Unknown error";
		error->error_code = 1;
		error->error_code_git = errcode;
	}

	char * fmt_mesg;
	asprintf(&fmt_mesg, "%s : %s [git:%d]", message, git_err_message, errcode);
	error->message = fmt_mesg;
	return (errcode==0)?0:1;
}

