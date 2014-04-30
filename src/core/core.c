#include "agb.h"
#include "agb/internal/types.h"
#include <memory.h>

int agb_error_new( AGBError ** error ) {
	AGBError * p = malloc(sizeof(AGBError));

	if(!p) return 1;
	memset(p,0,sizeof(AGBError));

	*error = p;
	return 0;
}

int agb_bridge_new( AGBCore ** anbGitBridge ) {
	AGBCore * p = malloc(sizeof(AGBCore));

	if(!p) return 1;
	memset(p,0,sizeof(AGBCore));

	*anbGitBridge = p;
	return 0;
}

int agb_error_delete( AGBError * error ) {
	free((char*)error->message);
	free(error);
	return 0;
}

int agb_bridge_delete( AGBCore * anbGitBridge ) {
	free(anbGitBridge);
	return 0;
}

int agb_merge_iterator_free( AGBMergeIterator * it ) { 
	free(it);
	return 0;
}

const char * agb_error_message( const AGBError * error) {
	return error->message;
}

