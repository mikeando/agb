#pragma once
#include "agb.h"

typedef struct AGBStatus AGBStatus;

int agb_get_status_new(AGBStatus ** status, AGBCore * repo );

/**
 * Uses the git_status_t enum, except for failure, where it returns -1.
 */
int agb_status_for_filename(const char * filename, AGBStatus * status);
