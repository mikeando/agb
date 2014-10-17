#include "clar/clar.h"
#include "agb.h"
#include "external/popen3.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "utils/test_utils.h"
#include <sys/wait.h>


//=====================================
//Other functions
//=====================================

const char * temp_dir = NULL;
int g_debug = 0;

//Get the environment variables for where to run

int system_fmt(const char * cmd_fmt, ... ) {
	va_list ap;
	va_start(ap,cmd_fmt);
	char * cmd;
	vasprintf(&cmd,cmd_fmt,ap);
	va_end(ap);
	if(g_debug) printf("Executing %s\n",cmd);

	int pstderr;
	int pstdout;


	pid_t pid = popen3(cmd,NULL,(g_debug)?NULL:&pstdout,(g_debug)?NULL:&pstderr);

	int status=0;
	waitpid(pid,&status,0);
	
	if(!g_debug) close(pstderr);
	if(!g_debug) close(pstdout);

	if(WIFEXITED(status)) {
		if(g_debug) printf("%s returned %d\n",cmd, WEXITSTATUS(status));
		return WEXITSTATUS(status);
	}

	if(g_debug) printf("%s had badness %d",cmd,status);

	free(cmd);
	return status;
}

void create_temp_dir(void) {
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

void create_simple_repo(void) {
	create_repo("simple_repo");
}

void create_repo(const char * repo_name) {
	create_temp_dir();
	system_fmt("( cd %s ; rm -rf %s )", temp_dir, repo_name );
	const char * create_repo_str = 
		"( "
		"cd %s ; "
		"mkdir %s ; "
		"cd %s ; "
		"git init . ; "
		"touch README.txt ; "
		"git add README.txt ; "
		"git commit -m'Added README.txt' ; "
		")";
	system_fmt(create_repo_str, temp_dir, repo_name, repo_name );
}

void repo_create_file(const char * repo_name, const char* filename, const char* content) {
	char * filepath;
	asprintf(&filepath, "%s/%s/%s", temp_dir, repo_name, filename);
	FILE* f = fopen(filepath,"w");
	if(!f) {
		free(filepath);
		cl_fail("Unable to create file");
	}
	fwrite(content,1,strlen(content),f);
	fclose(f);
	free(filepath);
}

void simple_repo_create_file(const char* filename, const char* content) {
	repo_create_file("simple_repo",filename,content);
}

void repo_delete_file(const char * repo_name, const char* filename) {
	char * filepath;
	asprintf(&filepath, "%s/%s/%s", temp_dir, repo_name, filename);
	cl_must_pass(unlink(filepath));
	free(filepath);
}

void simple_repo_delete_file(const char * filename) {
	repo_delete_file("simple_repo",filename);
}

int repo_contains(const char * repo_name, const char * filename ) {

	char * cmd;
	int readfd;
	asprintf(&cmd,"(cd %s/%s ; git ls-tree -r HEAD | grep %s)", temp_dir, repo_name, filename);
	if(g_debug) printf("repo_contains: running %s\n", cmd);
	pid_t pid = popen3(cmd, NULL, &readfd, NULL);


	char result[5]={0};
	ssize_t bytes_read = read(readfd,result,4);
	if(g_debug) printf("Got %d bytes : '%s'\n", (int)bytes_read, result);

	int status=0;
	waitpid(pid,&status,0);

	if(WIFEXITED(status)) {
		if(g_debug) printf("%s returned %d\n",cmd, WEXITSTATUS(status));
		return WEXITSTATUS(status)==0;
	}

	if(g_debug) printf("%s had badness %d",cmd,status);

	return 0;
}

int simple_repo_contains(const char * filename ) {
	return repo_contains("simple_repo", filename);
}

char status_in_commit(const char * repo_name, const char * commit_id, const char * filename) {
	char * cmd;
	int readfd;
	asprintf(&cmd,"(cd %s/%s ; git diff --name-status %s~..%s | grep '\\<%s\\>')", temp_dir, repo_name, commit_id, commit_id, filename);
	if(g_debug) printf("repo_contains: running %s\n", cmd);
	pid_t pid = popen3(cmd, NULL, &readfd, NULL);


	char result=0;
	ssize_t bytes_read = read(readfd,&result,1);

	int status=0;
	waitpid(pid,&status,0);
	free(cmd);

	if(!WIFEXITED(status) || WEXITSTATUS(status)!=0 ) {
		cl_fail("git diff failed");
	}

	if( bytes_read==0 ) 
		return 0;
	return result;
}

//TODO: This serves too many repos. But I couldn't get whitelisting them working 
//      and its not worth digging for too long.
//TODO: Take a nap and check it hasn't failed, and if its failed retry starting.
//      This can happen when another test using that port has just completed.
pid_t start_serving_repos(int port) {
	char * cmd;
	asprintf(&cmd,"git daemon --port=%d --base-path=%s --export-all", port, temp_dir);
	pid_t pid = popen3(cmd, NULL, NULL, NULL);
	free(cmd);
	return pid;
}

//TODO: This may be what we really move into the core library!
#include "agb/internal/types.h"
int init_bridge_with_repo(AGBCore* anbGitBridge, const char * repo_name) {

	char * repo_path;
	asprintf(&repo_path, "%s/%s", temp_dir, repo_name ); 
	int ok = git_repository_open(&anbGitBridge->repository,repo_path);
	if(ok!=0)
		return ok;
	anbGitBridge->origin_name = "origin";
	anbGitBridge->local_branch_name = "master";
	anbGitBridge->remote_branch_name = "origin/master";
	return 0;
}

int init_bridge(AGBCore* anbGitBridge) {

	char * repo_path;
	asprintf(&repo_path, "%s/simple_repo", temp_dir ); 
	int ok = git_repository_open(&anbGitBridge->repository,repo_path);
	if(ok!=0)
		return ok;

	anbGitBridge->origin_name = "origin";
	anbGitBridge->local_branch_name = "master";
	anbGitBridge->remote_branch_name = "origin/master";
	return 0;
}


void uninit_bridge(AGBCore* anbGitBridge) {
	git_repository_free(anbGitBridge->repository);
}
