#include "agb.h"
#include "agb/internal/eh.h"
#include <stdio.h>
#include <string.h>

int agb__error_translate(AGBError * error, const char * message, int errcode) {
	const char * git_err_message = NULL;
	if(errcode==0) {
		git_err_message = "no error.";
		error->error_code = 0;
		error->error_code_git = 0;
	}
	else if(errcode==GIT_ENOTFOUND) {
		git_err_message = "not found.";
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
	free((char*)error->message);
	error->message = fmt_mesg;
	return (errcode==0)?0:1;
}

int agb__error_from_string(AGBError * error, const char * message, int errcode, int git_errcode) {
    error->error_code = errcode;
    error->error_code_git = git_errcode;
    error->message = strdup(message);
    return (errcode==0)?0:1;
}



