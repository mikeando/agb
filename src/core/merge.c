#include "agb.h"
#include "agb/internal/types.h"
#include "git2.h"

#include <memory.h>

int sort_entries(const void * e1r, const void * e2r) {
	AGBMergeIteratorEntry * e1 = (AGBMergeIteratorEntry*)e1r;
	AGBMergeIteratorEntry * e2 = (AGBMergeIteratorEntry*)e2r;

	return strcmp(e1->name, e2->name);
}

int copy_entries(AGBMergeIteratorEntry * entries, const git_tree * tree, int idx) {
	int n = git_tree_entrycount(tree);
	int i;
	for(i=0; i<n; ++i) {
		const git_tree_entry * gte = git_tree_entry_byindex(tree,i);
		entries[i].name = git_tree_entry_name(gte);
		entries[i].treeentries[idx] = gte;
	}
	return n;
}

static inline int agb_git_tree_entry_equal(const git_tree_entry * tree_a, const git_tree_entry * tree_b ) {
	if(tree_a==NULL && tree_b==NULL) return 1;
	if(tree_a==NULL || tree_b==NULL) return 0;
	const git_oid * oid_a = git_tree_entry_id(tree_a);
	const git_oid * oid_b = git_tree_entry_id(tree_b);
	return git_oid_equal(oid_a,oid_b);
}

static inline const git_oid * agb_git_tree_entry_id(const git_tree_entry * tree) {
	if(!tree) return NULL;
	return git_tree_entry_id(tree);
}

static inline git_filemode_t agb_git_tree_entry_filemode(const git_tree_entry * tree) {
	if(!tree) return 0;
	return git_tree_entry_filemode(tree);
}

/*
#include <stdio.h>
void debug_dump_me_one(AGBMergeIteratorEntry * entry ) {
	printf("%s ", entry->name );
	char buf[7];
	printf("%s ", entry->ids[0]?git_oid_tostr(buf, 7, entry->ids[0]):"(null)"); 
	printf("%s ", entry->ids[1]?git_oid_tostr(buf, 7, entry->ids[1]):"(null)"); 
	printf("%s\n", entry->ids[2]?git_oid_tostr(buf, 7, entry->ids[2]):"(null)"); 
}

void debug_dump_me(AGBMergeIteratorEntry * entries, int count) {
	int i;
	for(i=0; i<count; ++i) {
		printf("  %d ",i);
		debug_dump_me_one(entries+i);
	}
}
*/


AGBMergeIterator * agb_merge__create_iterator(const git_tree * head_tree, const git_tree * branch_tree, const git_tree * base_tree, uint32_t merge_iterator_options ) {

	if(!head_tree) return NULL;	
	if(!branch_tree) return NULL;	
	if(!base_tree) return NULL;

	// Allocate enough space for all the entries of all three trees.
	// We'll go through and prune out duplicates later.
	
	int n_entries = 0;
	n_entries += git_tree_entrycount(head_tree);
	n_entries += git_tree_entrycount(branch_tree);
	n_entries += git_tree_entrycount(base_tree);

	AGBMergeIteratorEntry * all_entries = (AGBMergeIteratorEntry*) malloc( n_entries * sizeof(AGBMergeIteratorEntry) );

	memset(all_entries, 0, n_entries * sizeof(AGBMergeIteratorEntry) ); 

	// printf("BEFORE FILL\n");
	// debug_dump_me(all_entries, n_entries);


	AGBMergeIteratorEntry * cursor = all_entries;

	cursor+=copy_entries(cursor,head_tree,0);
	cursor+=copy_entries(cursor,branch_tree,1);
	cursor+=copy_entries(cursor,base_tree,2);

	// printf("AFTER FILL\n");
	// debug_dump_me(all_entries, n_entries);

	qsort(all_entries, n_entries, sizeof(AGBMergeIteratorEntry), &sort_entries);

	// printf("AFTER SORT\n");
	// debug_dump_me(all_entries, n_entries);

	// Now we merge the duplicate filename entries.
	AGBMergeIteratorEntry * write_point = all_entries;
	AGBMergeIteratorEntry * read_point = all_entries;
	int n_merged_entries = n_entries>0 ? 1 : 0;
	for(int i=0; i<n_entries; ++i) {
		if(strcmp(read_point->name,write_point->name)==0) {
			for(int j=0; j<3; ++j) {
				if(read_point->treeentries[j]!=NULL) {
					write_point->treeentries[j]=read_point->treeentries[j];
				}
			}
			read_point++;
			continue;
		}

		//They differ so move the write point on, and write into it.	
		write_point++;
		if(read_point!=write_point) {
			memcpy(write_point,read_point,sizeof(AGBMergeIteratorEntry));
		}
		n_merged_entries++;
		read_point++;
	}
	// printf("AFTER MERGE\n");
	// debug_dump_me(all_entries, n_entries);


	// Now (optionally) prune out the unchanged values
	if( (merge_iterator_options & agb_merge_iterator_options_ALL_ENTRIES)==0 ) {
		int n_changed = 0;
		write_point = all_entries;
		read_point = all_entries;
		for(int i=0; i<n_merged_entries; ++i ) {
			if( // Something changed
					//git_oid_equal dies on NULL...

					agb_git_tree_entry_equal(read_point->treeentries[0], read_point->treeentries[1])==0 ||
					agb_git_tree_entry_equal(read_point->treeentries[1], read_point->treeentries[2])==0 
			  ) {
				if(read_point!=write_point) {
					memcpy(write_point,read_point,sizeof(AGBMergeIteratorEntry));
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

	AGBMergeIteratorEntry * merged_entries = (AGBMergeIteratorEntry*) malloc( n_merged_entries * sizeof(AGBMergeIteratorEntry) );

	memcpy(merged_entries, all_entries,  n_merged_entries * sizeof(AGBMergeIteratorEntry));
	

	free(all_entries);

	AGBMergeIterator * retval = (AGBMergeIterator*)malloc(sizeof(AGBMergeIterator));
	memset(retval,0, sizeof(AGBMergeIterator));
	retval->trees[0] = head_tree;
	retval->trees[1] = branch_tree;
	retval->trees[2] = base_tree;


	retval->entries = merged_entries;
	retval->n_entries = n_merged_entries;

	return retval;
}

int agb_merge_iterator_next(AGBMergeIterator *it) {
	it->idx++;
	return it->idx<it->n_entries?0:1;
}
int agb_merge_iterator_is_valid(const AGBMergeIterator *it) {
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
AGBMergeIterator * merge( AGBCore * anbGitBridge, AGBError * error ) {

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


	return agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);
}

const git_oid * agb_merge_iterator_tree_id( const AGBMergeIterator * it, int idx) {
	return git_tree_id(it->trees[idx]);
}

const char * agb_merge_iterator_entry_name( const AGBMergeIterator * it) {
       return it->entries[it->idx].name;
}

const git_oid * agb_merge_iterator_entry_id( const AGBMergeIterator * it, int idx) {
	return agb_git_tree_entry_id(it->entries[it->idx].treeentries[idx]);
}
git_filemode_t agb_merge_iterator_entry_filemode( const AGBMergeIterator * it, int idx) {
	return agb_git_tree_entry_filemode(it->entries[it->idx].treeentries[idx]);
}

