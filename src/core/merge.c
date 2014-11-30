#include "agb.h"
#include "agb/internal/types.h"

#include <memory.h>
#include <assert.h>
#include <stdio.h>

static int sort_entries(const void * e1r, const void * e2r) {
	AGBMergeEntry * e1 = (AGBMergeEntry *)e1r;
	AGBMergeEntry * e2 = (AGBMergeEntry *)e2r;

	return strcmp(e1->name, e2->name);
}

static size_t copy_entries(AGBMergeEntry * entries, const git_tree * tree, enum AGBMergeIndex idx) {
	size_t n = git_tree_entrycount(tree);
	size_t i;
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

void print_merge_entry_component(AGBMergeEntry * entry, enum AGBMergeIndex idx) {
    char buf[7];
    const git_oid * id = agb_merge_entry_id(entry,idx);
    printf("%s ", id?git_oid_tostr(buf, 7, id):"(null)");
}

void debug_dump_me_one(AGBMergeEntry * entry ) {
	printf("%s ", entry->name );
    print_merge_entry_component(entry,AGB_MERGE_LOCAL); printf(" ");
    print_merge_entry_component(entry,AGB_MERGE_REMOTE); printf(" ");
    print_merge_entry_component(entry,AGB_MERGE_BASE); printf("\n");
}

void debug_dump_me(AGBMergeEntry * entries, int count) {
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
	
	size_t n_entries = 0;
	n_entries += git_tree_entrycount(head_tree);
	n_entries += git_tree_entrycount(branch_tree);
	n_entries += git_tree_entrycount(base_tree);

	AGBMergeEntry * all_entries = (AGBMergeEntry *) malloc( n_entries * sizeof(AGBMergeEntry) );

	memset(all_entries, 0, n_entries * sizeof(AGBMergeEntry) );

	// printf("BEFORE FILL\n");
	// debug_dump_me(all_entries, n_entries);


	AGBMergeEntry * cursor = all_entries;

	cursor+=copy_entries(cursor,head_tree,AGB_MERGE_LOCAL);
	cursor+=copy_entries(cursor,branch_tree,AGB_MERGE_REMOTE);
	cursor+=copy_entries(cursor,base_tree,AGB_MERGE_BASE);

	// printf("AFTER FILL\n");
	// debug_dump_me(all_entries, n_entries);

	qsort(all_entries, n_entries, sizeof(AGBMergeEntry), &sort_entries);

	// printf("AFTER SORT\n");
	// debug_dump_me(all_entries, n_entries);

	// Now we merge the duplicate filename entries.
	AGBMergeEntry * write_point = all_entries;
	AGBMergeEntry * read_point = all_entries;
	int n_merged_entries = n_entries>0 ? 1 : 0;
	for(size_t i=0; i<n_entries; ++i) {
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
			memcpy(write_point,read_point,sizeof(AGBMergeEntry));
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
					agb_git_tree_entry_equal(read_point->treeentries[AGB_MERGE_LOCAL], read_point->treeentries[AGB_MERGE_BASE])==0 ||
					agb_git_tree_entry_equal(read_point->treeentries[AGB_MERGE_REMOTE], read_point->treeentries[AGB_MERGE_BASE])==0
			  ) {
				if(read_point!=write_point) {
					memcpy(write_point,read_point,sizeof(AGBMergeEntry));
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

	AGBMergeEntry * merged_entries = (AGBMergeEntry *) malloc( n_merged_entries * sizeof(AGBMergeEntry) );

	memcpy(merged_entries, all_entries,  n_merged_entries * sizeof(AGBMergeEntry));
	

	free(all_entries);

	AGBMergeIterator * retval = (AGBMergeIterator*)malloc(sizeof(AGBMergeIterator));
	memset(retval,0, sizeof(AGBMergeIterator));
	retval->trees[AGB_MERGE_LOCAL] = head_tree;
	retval->trees[AGB_MERGE_REMOTE] = branch_tree;
	retval->trees[AGB_MERGE_BASE] = base_tree;

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
static int commit_oid_to_tree(git_tree ** tree, git_repository * repo, git_oid * oid ) {
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
__attribute__((unused))
static AGBMergeIterator * merge( 
		AGBCore * core,
		AGBError * error __attribute((unused))
		) {

	git_oid local_oid;
	git_oid remote_oid;
	git_oid base_oid;
	git_tree * local_tree = NULL;
	git_tree * remote_tree = NULL;
	git_tree * base_tree = NULL;

	//TODO: Use a real error?
	assert(core);
	assert(core->repository);

	/* TODO: FIX THIS */
	const char * local_branch_name = core->local_branch_name;
	const char * remote_branch_name = core->remote_branch_name;
	assert(local_branch_name);
	assert(remote_branch_name);
	git_repository * repo = core->repository;
	int ok;

	git_reference_name_to_id(&local_oid, repo, local_branch_name);
	git_reference_name_to_id(&remote_oid, repo, remote_branch_name);

	ok = git_merge_base(&base_oid, repo, &local_oid, &remote_oid);

	commit_oid_to_tree(&local_tree, repo, &local_oid);
	commit_oid_to_tree(&remote_tree, repo, &remote_oid);
	commit_oid_to_tree(&base_tree, repo, &base_oid);


	return agb_merge__create_iterator(local_tree, remote_tree, base_tree, agb_merge_iterator_options_NONE);
}

const git_oid * agb_merge_iterator_tree_id( const AGBMergeIterator * it, enum AGBMergeIndex i) {
	return git_tree_id(it->trees[i]);
}

const char * agb_merge_entry_name( const AGBMergeEntry * entry) {
       return entry->name;
}

const git_oid * agb_merge_entry_id( const AGBMergeEntry * entry, enum AGBMergeIndex i) {
	return agb_git_tree_entry_id(entry->treeentries[i]);
}

git_filemode_t agb_merge_entry_filemode( const AGBMergeEntry * entry, enum AGBMergeIndex i) {
	return agb_git_tree_entry_filemode(entry->treeentries[i]);
}

AGBMergeEntry * agb_merge_entry_from_iterator(const AGBMergeIterator * it) {
	assert(it->idx<it->n_entries);
	return &it->entries[it->idx];
}

//A NULL safe git_oid_equal
//TODO: Copied from merge.c - probably could make this common
 int agb_git_oid_equal(const git_oid * oid_a, const git_oid * oid_b ) {
    if(oid_a==NULL && oid_b==NULL) return 1;
    if(oid_a==NULL || oid_b==NULL) return 0;
    return git_oid_equal(oid_a,oid_b);
}

int agb_merge( AGBMerger * merger, AGBError * error) {
	AGBMergeIterator * it = merge( merger->core, error);

    AGBMerger_vtable * callbacks = merger->vtable;
	//TODO: Use a real error code.
	if(it==NULL)
		return 1;

	//TODO: Dispose of the iterator

    //TODO: This doesn't handle type changes, or permission changes
    for( ; agb_merge_iterator_is_valid(it) ; agb_merge_iterator_next(it) ) {
        AGBMergeEntry *mergeEntry = agb_merge_entry_from_iterator(it);

        const git_oid * base_id = agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE);
        const git_oid * local_id = agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL);
        const git_oid * remote_id = agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE);
        int hasLocalChanged = !agb_git_oid_equal(base_id, local_id );
        int hasRemoteChanged = !agb_git_oid_equal(base_id, remote_id );
        if(callbacks->onEveryEntry) {
            callbacks->onEveryEntry(merger, mergeEntry);
        }

        if( !hasLocalChanged && !hasRemoteChanged ) {
            continue;
        }

        if(callbacks->onEveryChange) {
            callbacks->onEveryChange(merger, mergeEntry);
        }

        if( hasLocalChanged && !hasRemoteChanged ) {

            //We want the local version.
            if(agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL) == NULL) {

                if(callbacks->onRemove) {
                    callbacks->onRemove(merger, mergeEntry, AGB_MERGE_LOCAL);
                }

                continue;
            }

            if(agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE) == NULL) {
                if(callbacks->onAdd) {
                    callbacks->onAdd(merger, mergeEntry, AGB_MERGE_LOCAL);
                }
                continue;
            }

            if(callbacks->onModify) {
                callbacks->onModify(merger, mergeEntry, AGB_MERGE_LOCAL);
            }

            //TODO: Correctly forward this.
            //printf("ADDING OR UPDATING %s to tree\n", agb_merge_entry_name(mergeEntry) );
            //git_treebuilder_insert(NULL, builder, agb_merge_entry_name(mergeEntry), agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL), agb_merge_entry_filemode(mergeEntry,AGB_MERGE_LOCAL) );
            continue;
        }

        if( !hasLocalChanged && hasRemoteChanged ) {


            //We want the local version.
            if(agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE) == NULL) {

                if(callbacks->onRemove) {
                    callbacks->onRemove(merger, mergeEntry, AGB_MERGE_REMOTE);
                }

                //TODO: Correctly forward this.
                //printf("REMOVING %s from tree\n", agb_merge_entry_name(mergeEntry) );
                //git_treebuilder_remove(builder, agb_merge_entry_name(mergeEntry) );
                continue;
            }

            //We want the head version.
            if(agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE) == NULL) {
                if(callbacks->onAdd) {
                    callbacks->onAdd(merger, mergeEntry, AGB_MERGE_REMOTE);
                }
                //TODO: Correctly forward this.
                //printf("REMOVING %s from tree\n", agb_merge_entry_name(mergeEntry) );
                //git_treebuilder_remove(builder, agb_merge_entry_name(mergeEntry) );
                continue;
            }


            if(callbacks->onModify) {
                callbacks->onModify(merger, mergeEntry, AGB_MERGE_REMOTE);
            }

            //TODO: Correctly forward this.
            //printf("ADDING OR UPDATING %s to tree\n", agb_merge_entry_name(mergeEntry) );
            //git_treebuilder_insert(NULL, builder, agb_merge_entry_name(mergeEntry), agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE), agb_merge_entry_filemode(mergeEntry,AGB_MERGE_REMOTE) );
            continue;
        }

        //TODO: Forward it.
        printf("CONFLICT %s in tree\n", agb_merge_entry_name(mergeEntry) );
        //TODO: CONFLICT - Handle it!

    }
#if 0
    // Our tree builder should now be full...
    // Lets write it out to a tree
    //
    //TODO: Check for errors
    git_oid new_tree_oid = {};
    git_treebuilder_write(&new_tree_oid, repo, builder);

    char hexid[GIT_OID_HEXSZ+1];
    printf("Tree SHA is %s\n", git_oid_tostr(hexid,GIT_OID_HEXSZ+1, &new_tree_oid));

    git_tree * new_tree = NULL;
    git_tree_lookup(&new_tree, repo, &new_tree_oid);

    // Now we need to create the commit.
    const git_commit** parents = (const git_commit**)malloc(sizeof(git_commit*)*2);

    parents[0] = head_commit;
    parents[1] = branch_commit;


    git_signature * author_signature = NULL;

    // Time since epoch
    // TODO: Get these correctly -
    // Could use git_signature_now instead...
    {

        git_time_t author_time = time(NULL);
        int timezone_offset = 0;
        int ok;

        if((ok=git_signature_new(&author_signature,"Someone","someone@somewhere.com", author_time, timezone_offset))!=0) {
            agb__error_translate(error,"git_signature_new failed",ok);
            goto cleanup_error;
        }
    }



    git_oid commit_id;
    int ok = git_commit_create(
            &commit_id,
            repo,
            NULL,
            author_signature,
            author_signature,
            "UTF-8",
            "An exciting commit",
            new_tree,
            2, //Two parents
            parents
    );
    if(ok!=0) {
        agb__error_translate(error,"git_commit_create failed",ok);
        goto cleanup_error;
    }

    // Then update the refs.
    //TODO: Do we need to release this ref?
    git_reference * ref;
    ok = git_reference_create_matching(
            &ref,
            repo,
            "refs/heads/branch_c",
            &commit_id,
            1,
            NULL,
            author_signature,
            "merged by libagb");

    if(ok!=0) {
        agb__error_translate(error,"git_reference_create failed",ok);
        goto cleanup_error;
    }

    git_signature_free(author_signature);

    // Now check we got the expected files
    cl_assert_equal_c('A', test_core_merge__compare_with_parents_merge_base("branch_c", "created_in_a.txt"));
    cl_assert_equal_c('A', test_core_merge__compare_with_parents_merge_base("branch_c", "created_in_b.txt"));

    return;

    cleanup_error:

    printf("ERROR: %s\n",error->message);

    if(author_signature) {
        git_signature_free(author_signature);
    }
    cl_fail(error->message);


#endif

    return 0;
}
