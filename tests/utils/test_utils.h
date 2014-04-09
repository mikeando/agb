extern const char * temp_dir;
extern int g_debug;


/**
 * Run the cmd as specified, returning the exits status.
 *
 * Runs inside the bash interpreter (due to how popen3 works)
 * and so you can chain together commands.
 */
int system_fmt(const char * cmd_fmt, ... );

/**
 * Creates a test directory of the form 
 * "TEMPDIR/git_test_2014-07-30-23-21-56_asdfg"
 * where TEMPDIR is either $ANB_TEST_DIR, $TMPDIR or /tmp/
 *
 * Also sets the temp_dir to the name of the created directory.
 * Doesn't recreate the directory if temp_dir is already set.
 */
void create_temp_dir();

/**
 * Creates a minimal repository called "simple_repo" inside `temp_dir`.
 *
 * Created repository contains only one file, README.txt, and a single commit
 * to add that file.
 */
void create_repo();

/**
 * Creates a minimal repository called "simple_repo" inside `temp_dir`.
 *
 * Created repository contains only one file, README.txt, and a single commit
 * to add that file.
 */
void create_simple_repo();

/** 
 * Create a file in the simple_repo repository with the specified name, and 
 * content.
 *
 * Doesn't currently create any of the required subdirectories.
 */
void simple_repo_create_file(const char* filename, const char* content);

/** 
 * Create a file in the named repository with the specified name, and 
 * content.
 *
 * Doesn't currently create any of the required subdirectories.
 */
void repo_create_file(const char * repo_name, const char* filename, const char* content);

void repo_delete_file(const char * repo_name, const char* filename);
void simple_repo_delete_file(const char * filename);

/**
 * Test whether the simple_repo repository contains the named file.
 *
 * Returns non-zero if file exists, zero if it does not.
 */
int simple_repo_contains(const char * filename );

/**
 * Test whether the named repository contains the named file.
 *
 * Returns non-zero if file exists, zero if it does not.
 */
int repo_contains(const char * repo_name, const char * filename );

/**
 * Return the status code for the given file in the given commit.
 * Will often be called as `status_in_commit(repo,"HEAD","a_file.txt")`
 */
char status_in_commit(const char * repo_name, const char * commit_id, const char * filename);

pid_t start_serving_repos(int port);

void init_bridge_with_repo(ANBGitBridge* anbGitBridge, const char * repo_name);
void init_bridge(ANBGitBridge* anbGitBridge);
void uninit_bridge(ANBGitBridge* anbGitBridge);

