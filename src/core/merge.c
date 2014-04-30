#include "anbgitbridge.h"
#include "anbgitbridge/internal/types.h"
#include "git2.h"

#include <memory.h>

int sort_entries(const void * e1r, const void * e2r) {
	ANBGitBridgeMergeIteratorEntry * e1 = (ANBGitBridgeMergeIteratorEntry*)e1r;
	ANBGitBridgeMergeIteratorEntry * e2 = (ANBGitBridgeMergeIteratorEntry*)e2r;

	return strcmp(e1->name, e2->name);
}

int copy_entries(ANBGitBridgeMergeIteratorEntry * entries, git_tree * tree, int idx) {
	int n = git_tree_entrycount(tree);
	int i;
	for(i=0; i<n; ++i) {
		const git_tree_entry * gte = git_tree_entry_byindex(tree,i);
		entries[i].name = git_tree_entry_name(gte);
		entries[i].ids[idx] = git_tree_entry_id(gte);
	}
	return n;
}

//A NULL safe git_oid_equal
static inline int anb_git_oid_equal(const git_oid * oid_a, const git_oid * oid_b ) {
	if(oid_a==NULL && oid_b==NULL) return 1;
	if(oid_a==NULL || oid_b==NULL) return 0;
	return git_oid_equal(oid_a,oid_b);
}

/*
#include <stdio.h>
void debug_dump_me_one(ANBGitBridgeMergeIteratorEntry * entry ) {
	printf("%s ", entry->name );
	char buf[7];
	printf("%s ", entry->ids[0]?git_oid_tostr(buf, 7, entry->ids[0]):"(null)"); 
	printf("%s ", entry->ids[1]?git_oid_tostr(buf, 7, entry->ids[1]):"(null)"); 
	printf("%s\n", entry->ids[2]?git_oid_tostr(buf, 7, entry->ids[2]):"(null)"); 
}

void debug_dump_me(ANBGitBridgeMergeIteratorEntry * entries, int count) {
	int i;
	for(i=0; i<count; ++i) {
		printf("  %d ",i);
		debug_dump_me_one(entries+i);
	}
}
*/


ANBGitBridgeMergeIterator * create_merge_iterator(git_tree * head_tree, git_tree * branch_tree, git_tree * base_tree, uint32_t merge_iterator_options ) {

	if(!head_tree) return NULL;	
	if(!branch_tree) return NULL;	
	if(!base_tree) return NULL;

	// Allocate enough space for all the entries of all three trees.
	// We'll go through and prune out duplicates later.
	
	int n_entries = 0;
	n_entries += git_tree_entrycount(head_tree);
	n_entries += git_tree_entrycount(branch_tree);
	n_entries += git_tree_entrycount(base_tree);

	ANBGitBridgeMergeIteratorEntry * all_entries = (ANBGitBridgeMergeIteratorEntry*) malloc( n_entries * sizeof(ANBGitBridgeMergeIteratorEntry) );

	memset(all_entries, 0, n_entries * sizeof(ANBGitBridgeMergeIteratorEntry) ); 

	// printf("BEFORE FILL\n");
	// debug_dump_me(all_entries, n_entries);


	ANBGitBridgeMergeIteratorEntry * cursor = all_entries;

	cursor+=copy_entries(cursor,head_tree,0);
	cursor+=copy_entries(cursor,branch_tree,1);
	cursor+=copy_entries(cursor,base_tree,2);

	// printf("AFTER FILL\n");
	// debug_dump_me(all_entries, n_entries);

	qsort(all_entries, n_entries, sizeof(ANBGitBridgeMergeIteratorEntry), &sort_entries);

	// printf("AFTER SORT\n");
	// debug_dump_me(all_entries, n_entries);

	// Now we merge the duplicate filename entries.
	ANBGitBridgeMergeIteratorEntry * write_point = all_entries;
	ANBGitBridgeMergeIteratorEntry * read_point = all_entries;
	int n_merged_entries = n_entries>0 ? 1 : 0;
	for(int i=0; i<n_entries; ++i) {
		if(strcmp(read_point->name,write_point->name)==0) {
			for(int j=0; j<3; ++j) {
				if(read_point->ids[j]!=NULL) {
					write_point->ids[j]=read_point->ids[j];
				}
			}
			read_point++;
			continue;
		}

		//They differ so move the write point on, and write into it.	
		write_point++;
		if(read_point!=write_point) {
			memcpy(write_point,read_point,sizeof(ANBGitBridgeMergeIteratorEntry));
		}
		n_merged_entries++;
		read_point++;
	}
	// printf("AFTER MERGE\n");
	// debug_dump_me(all_entries, n_entries);


	// Now (optionally) prune out the unchanged values
	if( (merge_iterator_options & anb_gitbridge_merge_iterator_options_ALL_ENTRIES)==0 ) {
		int n_changed = 0;
		write_point = all_entries;
		read_point = all_entries;
		for(int i=0; i<n_merged_entries; ++i ) {
			if( // Something changed
					//git_oid_equal dies on NULL...

					anb_git_oid_equal(read_point->ids[0], read_point->ids[1])==0 ||
					anb_git_oid_equal(read_point->ids[1], read_point->ids[2])==0 
			  ) {
				if(read_point!=write_point) {
					memcpy(write_point,read_point,sizeof(ANBGitBridgeMergeIteratorEntry));
				}
				read_point++;
				write_point++;
				n_changed++;
				continue;
			}
			// Nothing changed, skip it.
			read_point++;
		}
		n_merged_entries = n_changed;
		//printf("AFTER FILTER\n");
		//debug_dump_me(all_entries, n_entries);
	}

	// Now we only need the merged entries;

	ANBGitBridgeMergeIteratorEntry * merged_entries = (ANBGitBridgeMergeIteratorEntry*) malloc( n_merged_entries * sizeof(ANBGitBridgeMergeIteratorEntry) );

	memcpy(merged_entries, all_entries,  n_merged_entries * sizeof(ANBGitBridgeMergeIteratorEntry));
	

	free(all_entries);

	ANBGitBridgeMergeIterator * retval = (ANBGitBridgeMergeIterator*)malloc(sizeof(ANBGitBridgeMergeIterator));
	memset(retval,0, sizeof(ANBGitBridgeMergeIterator));
	retval->head_tree = head_tree;
	retval->branch_tree = branch_tree;
	retval->base_tree = base_tree;


	retval->entries = merged_entries;
	retval->n_entries = n_merged_entries;

	return retval;
}

int anb_gitbridge_merge_iterator_next(ANBGitBridgeMergeIterator *it) {
	it->idx++;
	return it->idx<it->n_entries?0:1;
}
int anb_gitbridge_merge_iterator_is_valid(ANBGitBridgeMergeIterator *it) {
	return it->idx<it->n_entries;
}



//TODO: Add error handling.
int commit_oid_to_tree(git_tree ** tree, git_repository * repo, git_oid * oid ) {
	if(!repo) return 1;
	git_commit * commit;
	git_commit_lookup(&commit, repo, oid);
	const git_oid * tree_oid = git_commit_tree_id(commit);
	git_tree_lookup(tree, repo, tree_oid);
	git_commit_free(commit);
	return 0;
}

//TODO: Add error handling.
/*
 * Get the SHA of the two trees we're going to merge and create an iterator from them.
 */
ANBGitBridgeMergeIterator * merge( ANBGitBridge * anbGitBridge, ANBGitBridgeError * error ) {

	git_oid head_oid;
	git_oid branch_oid;
	git_oid base_oid;
	git_tree * head_tree = NULL;
	git_tree * branch_tree = NULL;
	git_tree * base_tree = NULL;

	/* TODO: FIX THIS */
	const char * branch_name = "refs/heads/branch_a";
	git_repository * repo = anbGitBridge->repository;
	if(!repo) return NULL;
	int ok;


	git_reference_name_to_id(&head_oid, repo, "HEAD");
	git_reference_name_to_id(&branch_oid, repo, branch_name);

	ok = git_merge_base(&base_oid, repo, &head_oid, &branch_oid);

	commit_oid_to_tree(&head_tree, repo, &head_oid);
	commit_oid_to_tree(&branch_tree, repo, &branch_oid);
	commit_oid_to_tree(&base_tree, repo, &base_oid);


	return create_merge_iterator(head_tree, branch_tree, base_tree, anb_gitbridge_merge_iterator_options_NONE);
}

const git_oid * anb_gitbridge_merge_iterator_base_id( ANBGitBridgeMergeIterator * it) {
	return git_tree_id(it->base_tree);
}
const git_oid * anb_gitbridge_merge_iterator_branch_id( ANBGitBridgeMergeIterator * it) {
       return git_tree_id(it->branch_tree);
}       
const git_oid * anb_gitbridge_merge_iterator_head_id( ANBGitBridgeMergeIterator * it) {
	return git_tree_id(it->head_tree);
}

const char * anb_gitbridge_merge_iterator_entry_name( ANBGitBridgeMergeIterator * it) {
       return it->entries[it->idx].name;
}       
const git_oid * anb_gitbridge_merge_iterator_base_entry_id( ANBGitBridgeMergeIterator * it) {
	return it->entries[it->idx].ids[2];
}
const git_oid * anb_gitbridge_merge_iterator_branch_entry_id( ANBGitBridgeMergeIterator * it) {
       return it->entries[it->idx].ids[1];
}       
const git_oid * anb_gitbridge_merge_iterator_head_entry_id( ANBGitBridgeMergeIterator * it) {
	return it->entries[it->idx].ids[0];
}

