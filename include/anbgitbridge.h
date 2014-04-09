typedef struct ANBGitBridge ANBGitBridge;
typedef struct ANBGitBridgeError ANBGitBridgeError;

int anb_git_bridge_sync_files(ANBGitBridge* anbGitBridge, ANBGitBridgeError * error);
int anb_git_bridge_fetch(ANBGitBridge * anbGitBridge, ANBGitBridgeError * error);

