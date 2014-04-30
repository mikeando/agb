#ifndef ANBGITBRIDGE_H
#define ANBGITBRIDGE_H

#include "git2.h"

void agb_init();


struct AGBCore;
typedef struct AGBCore AGBCore;

struct AGBCore {
	git_repository * repository;
	const char * origin_name; // Usually "origin
};

typedef struct AGBError {
	const char * message;
	int error_code;
} AGBError;

void agb_error_init(AGBError * e);
void agb_error_free(AGBError * e);

//Callback typedefs

typedef void(*AGBFetchCallback)(AGBError*, void* userdata);


#endif
