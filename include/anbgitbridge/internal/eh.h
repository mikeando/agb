#pragma once
#include "anbgitbridge/internal/types.h"

int anb_git_bridge__error_translate(ANBGitBridgeError * error, const char * message, int errcode);
