#include "ANBGitBridge.h"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#define RUN_TEST(X) X()
#define TEST(X) void X()

#define ASSERT(X) do { if(X!=0) FAIL(#X); return; } while(false) 
#define FAIL(X) printf("%s:%d: Test Failed %s\n",__FILE__,__LINE__, X)

//Get the environment variables for where to run

int system_fmt(const char * cmd_fmt, ... ) {
	va_list ap;
	va_start(ap,cmd_fmt);
	char * cmd;
	vasprintf(&cmd,cmd_fmt,ap);
	va_end(ap);
	printf("Executing %s\n",cmd);
	int ok = system(cmd);
	free(cmd);
	return ok;
}


const char * temp_dir = NULL;

void create_temp_dir() {
	if(temp_dir) return;

	const char* ztemp_dir = getenv("ANB_TEST_DIR");
	if(ztemp_dir==NULL) ztemp_dir=getenv("TMPDIR");
	if(ztemp_dir==NULL) ztemp_dir="/tmp/";

	system_fmt("mkdir -p \"%s\"", ztemp_dir);
	char * repo_dir_name_template;

	char timestr[25]={0};
	time_t now = time(0);
	struct tm *mytime = localtime(&now); 
	strftime(timestr, sizeof timestr, "%Y-%m-%d-%H-%M-%S", mytime);


	asprintf(&repo_dir_name_template, "%s/git_test_%s__XXXXXXX", ztemp_dir, timestr);
	temp_dir = mkdtemp(repo_dir_name_template);
}
	


void create_simple_repo() {
	create_temp_dir();
	system_fmt("( cd %s; rm -rf simple_repo )", temp_dir );
	system_fmt("( cd %s; mkdir simple_repo ; cd simple_repo ; git init . )", temp_dir );
}

void create_test_file_in_repo(const char* filename, const char* content) {
	char * filepath;
	asprintf(&filepath, "%s/simple_repo/%s", temp_dir, filename);
	FILE* f = fopen(filepath,"w");
	fwrite(content,1,strlen(content),f);
	fclose(f);
	free(filepath);
}

int repo_contains(const char * filename ) {

	int ok = system_fmt("( cd %s/simple_repo ; git ls-files --error-unmatch %s )", temp_dir, filename );

	printf("ls-files returned %d\n",ok);

	return -1;
}

void init_bridge(ANBGitBridge* anbGitBridge) {
}

//TODO: Move this into ANBGitBridge.h
//TODO: Errors and Error handling
void anb_git_bridge_sync_files(ANBGitBridge* anbGitBridge) {
}


TEST(check_sync_commits_files) {
	create_simple_repo();
	create_test_file_in_repo("dummy.dat","hello world");

	ANBGitBridge anbGitBridge;
	init_bridge(&anbGitBridge);

	anb_git_bridge_sync_files(&anbGitBridge);

	ASSERT(repo_contains("dummy.dat"));
}

int main() {
	RUN_TEST(check_sync_commits_files);
}


