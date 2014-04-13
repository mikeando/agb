typedef struct ANBGitBridge ANBGitBridge;
typedef struct ANBGitBridgeError ANBGitBridgeError;

int anb_git_bridge_sync_files(ANBGitBridge* anbGitBridge, ANBGitBridgeError * error);
int anb_git_bridge_fetch(ANBGitBridge * anbGitBridge, ANBGitBridgeError * error);

int anb_gitbridge_error_new( ANBGitBridgeError ** error );
int anb_gitbridge_bridge_new( ANBGitBridge ** error );
int anb_gitbridge_error_delete( ANBGitBridgeError * error );
int anb_gitbridge_bridge_delete( ANBGitBridge * error );
const char * anb_gitbridge_error_message( const ANBGitBridgeError * error);
