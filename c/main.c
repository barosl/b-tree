#include "b-tree.h" /* FIXME */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

long empty_node_idx = -1;

int rec_cmp(rec_t *a, rec_t *b) {
	return strcmp(a->name, b->name);
}

void rec_print(rec_t *rec) {
	printf("[%s (%.1f, %.1f)]", rec->name, rec->x, rec->y);
}

FILE *fp;
long root_idx;

void load_node(node_t *node) {
	fseek(fp, node->idx*sizeof(node_t), SEEK_SET);
	fread(node, sizeof(*node), 1, fp);
}

void add_node(node_t *node) {
	if (empty_node_idx == -1) fseek(fp, 0, SEEK_END);
	else {
		fseek(fp, empty_node_idx*sizeof(node_t), SEEK_SET);
		fread(&empty_node_idx, sizeof(empty_node_idx), 1, fp);
		fseek(fp, -sizeof(empty_node_idx), SEEK_CUR);
	}
	node->idx = ftell(fp)/sizeof(node_t);
	fwrite(node, sizeof(*node), 1, fp);
}

void save_node(node_t *node) {
	fseek(fp, node->idx*sizeof(node_t), SEEK_SET);
	fwrite(node, sizeof(*node), 1, fp);
}

void set_node_unoccupied(long node_idx) {
	fseek(fp, node_idx*sizeof(node_t), SEEK_SET);
	fwrite(&empty_node_idx, sizeof(empty_node_idx), 1, fp);

	empty_node_idx = node_idx;
}

void insert_rec_adjust(int node_idx, long *parents, int parent_cnt, int *parent_is, int parent_i_cnt) {
	int i;

	node_t node;
	node.idx = node_idx;
	load_node(&node);

	if (node.rec_cnt <= ORDER_2) return;

	rec_t mid_rec = node.recs[ORDER];

	node_t left;
	for (i=0;i<ORDER;i++) left.recs[i] = node.recs[i];
	left.rec_cnt = ORDER;
	for (i=0;i<node.child_cnt/2;i++) left.childs[i] = node.childs[i];
	left.child_cnt = node.child_cnt/2;

	add_node(&left);

	node_t right;
	for (i=ORDER+1;i<node.rec_cnt;i++) right.recs[i-(ORDER+1)] = node.recs[i];
	right.rec_cnt = ORDER;
	for (i=node.child_cnt/2;i<node.child_cnt;i++) right.childs[i-(node.child_cnt/2)] = node.childs[i];
	right.child_cnt = node.child_cnt/2;

	add_node(&right);

	node.childs[0] = left.idx;
	node.childs[1] = right.idx;
	node.child_cnt = 2;

	if (parent_cnt) {
		node_t parent;
		parent.idx = parents[0];
		load_node(&parent);

		int parent_i = parent_is[0];

		for (i=parent.rec_cnt-1;i>=parent_i;i--) parent.recs[i+1] = parent.recs[i];
		parent.rec_cnt++;
		parent.recs[parent_i] = mid_rec;

		for (i=parent.child_cnt-1;i>=(parent_i+1);i--) parent.childs[i+1] = parent.childs[i];
		parent.child_cnt++;
		parent.childs[parent_i] = left.idx;
		parent.childs[parent_i+1] = right.idx;

		save_node(&parent);

		insert_rec_adjust(parent.idx, parents+1, parent_cnt-1, parent_is+1, parent_i_cnt-1);
	} else {
		node.recs[0] = mid_rec;
		node.rec_cnt = 1;

		save_node(&node);
	}
}

int insert_rec_2(long node_idx, rec_t *rec, long *parents, int parent_cnt, int *parent_is, int parent_i_cnt) {
	int i;

	node_t node;
	node.idx = node_idx;
	load_node(&node);

	for (i=0;i<node.rec_cnt;i++) {
		int cmped = rec_cmp(&node.recs[i], rec);

		if (cmped > 0) break;
		else if (cmped == 0) return -1;
	}

	int cur_i = i;

	if (node.child_cnt) {
		fseek(fp, node.idx*sizeof(node_t), SEEK_SET);
		fwrite(&node, sizeof(node), 1, fp);

		parents[-1] = node.idx;
		parent_is[-1] = cur_i;
		return insert_rec_2(node.childs[i], rec, parents-1, parent_cnt+1, parent_is-1, parent_i_cnt+1);
	} else {
		for (i=node.rec_cnt-1;i>=cur_i;i--) node.recs[i+1] = node.recs[i];
		node.rec_cnt++;
		node.recs[cur_i] = *rec;

		fseek(fp, node.idx*sizeof(node_t), SEEK_SET);
		fwrite(&node, sizeof(node), 1, fp);

		insert_rec_adjust(node_idx, parents, parent_cnt, parent_is, parent_i_cnt);
		return 0;
	}
}

int insert_rec(long node_idx, rec_t *rec) {
	long parents[100]; /* FIXME */
	int parent_is[100]; /* FIXME */
	return insert_rec_2(node_idx, rec, parents+sizeof(parents)/sizeof(parents[0]), 0, parent_is+sizeof(parent_is)/sizeof(parent_is[0]), 0);
}

int search_rec(long node_idx, rec_t *rec) {
	int i;

	node_t node;
	node.idx = node_idx;
	load_node(&node);

	for (i=0;i<node.rec_cnt;i++) {
		int cmped = rec_cmp(&node.recs[i], rec);

		if (cmped > 0) {
			if (node.child_cnt) return search_rec(node.childs[i], rec);
			else return -1;
		} else if (cmped == 0) {
			*rec = node.recs[i];
			return 0;
		}
	}

	if (node.child_cnt) return search_rec(node.childs[node.child_cnt-1], rec);
	else return -1;
}

void remove_rec_adjust(long node_idx, long *parents, int parent_cnt, int *parent_is, int parent_i_cnt) {
	int i;

	node_t node;
	node.idx = node_idx;
	load_node(&node);

	node_t parent;
	parent.idx = parents[0];
	load_node(&parent);

	int parent_i = parent_is[0];

	if (!parent_cnt) {
		if (!node.rec_cnt && node.child_cnt) {
			root_idx = node.childs[0];
			set_node_unoccupied(node.idx);
		}
		return;
	}

	if (node.rec_cnt >= ORDER) return;

	if (parent_i+1 < parent.child_cnt) {
		node_t parent_child;
		parent_child.idx = parent.childs[parent_i+1];
		load_node(&parent_child);
		if (parent_child.rec_cnt > ORDER) {
			node.recs[node.rec_cnt++] = parent.recs[parent_i];
			if (parent_child.child_cnt) {
				node.childs[node.child_cnt++] = parent_child.childs[0];

				for (i=1;i<parent_child.child_cnt;i++) parent_child.childs[i-1] = parent_child.childs[i];
				parent_child.child_cnt--;
			}

			parent.recs[parent_i] = parent_child.recs[0];

			for (i=1;i<parent_child.rec_cnt;i++) parent_child.recs[i-1] = parent_child.recs[i];
			parent_child.rec_cnt--;

			save_node(&node);
			save_node(&parent);
			save_node(&parent_child);

			return;
		}
	}

	if (parent_i-1 >= 0) {
		node_t parent_child;
		parent_child.idx = parent.childs[parent_i-1];
		load_node(&parent_child);
		if (parent_child.rec_cnt > ORDER) {
			for (i=node.rec_cnt-1;i>=0;i--) node.recs[i+1] = node.recs[i];
			node.rec_cnt++;
			node.recs[0] = parent.recs[parent_i-1];

			if (parent_child.child_cnt) {
				for (i=node.child_cnt-1;i>=0;i--) node.childs[i+1] = node.childs[i];
				node.child_cnt++;

				node.childs[0] = parent_child.childs[parent_child.child_cnt-1];

				parent_child.child_cnt--;
			}

			parent.recs[parent_i-1] = parent_child.recs[parent_child.rec_cnt-1];

			parent_child.rec_cnt--;

			save_node(&node);
			save_node(&parent);
			save_node(&parent_child);

			return;
		}
	}

	if (parent_i+1 < parent.child_cnt) {
		node_t parent_child;
		parent_child.idx = parent.childs[parent_i+1];
		load_node(&parent_child);

		node.recs[node.rec_cnt++] = parent.recs[parent_i];
		for (i=0;i<parent_child.rec_cnt;i++) node.recs[node.rec_cnt++] = parent_child.recs[i];

		for (i=0;i<parent_child.child_cnt;i++) node.childs[node.child_cnt++] = parent_child.childs[i];

		for (i=parent_i+1;i<parent.rec_cnt;i++) parent.recs[i-1] = parent.recs[i];
		parent.rec_cnt--;

		set_node_unoccupied(parent.childs[parent_i+1]);
		for (i=parent_i+2;i<parent.child_cnt;i++) parent.childs[i-1] = parent.childs[i];
		parent.child_cnt--;

		save_node(&node);
		save_node(&parent);

		remove_rec_adjust(parent.idx, parents+1, parent_cnt-1, parent_is+1, parent_i_cnt-1);
		return;
	}

	if (parent_i-1 >= 0) {
		node_t parent_child;
		parent_child.idx = parent.childs[parent_i-1];
		load_node(&parent_child);

		for (i=0;i<node.rec_cnt;i++) node.recs[i+(parent_child.rec_cnt+1)] = node.recs[i];
		node.rec_cnt += (parent_child.rec_cnt+1);
		for (i=0;i<parent_child.rec_cnt;i++) node.recs[i] = parent_child.recs[i];
		node.recs[parent_child.rec_cnt] = parent.recs[parent_i-1];

		for (i=0;i<node.child_cnt;i++) node.childs[i+parent_child.child_cnt] = node.childs[i];
		node.child_cnt += parent_child.child_cnt;
		for (i=0;i<parent_child.child_cnt;i++) node.childs[i] = parent_child.childs[i];

		for (i=parent_i;i<parent.rec_cnt;i++) parent.recs[i-1] = parent.recs[i];
		parent.rec_cnt--;

		set_node_unoccupied(parent.childs[parent_i-1]);
		for (i=parent_i;i<parent.child_cnt;i++) parent.childs[i-1] = parent.childs[i];
		parent.child_cnt--;

		save_node(&node);
		save_node(&parent);

		remove_rec_adjust(parent.idx, parents+1, parent_cnt-1, parent_is+1, parent_i_cnt-1);
		return;
	}
}

int remove_rec_2(long node_idx, rec_t *rec, long *parents, int parent_cnt, int *parent_is, int parent_i_cnt) {
	int i, j;

	node_t node;
	node.idx = node_idx;
	load_node(&node);

	for (i=0;i<node.rec_cnt;i++) {
		int cmped = rec_cmp(&node.recs[i], rec);

		if (cmped > 0) {
			if (node.child_cnt) {
				parents[-1] = node.idx;
				parent_is[-1] = i;
				return remove_rec_2(node.childs[i], rec, parents-1, parent_cnt+1, parent_is-1, parent_i_cnt+1);
			} else {
				return -1;
			}
		} else if (cmped == 0) {
			if (!node.child_cnt) {
				for (j=i+1;j<node.rec_cnt;j++) node.recs[j-1] = node.recs[j];
				node.rec_cnt--;

				save_node(&node);

				remove_rec_adjust(node.idx, parents, parent_cnt, parent_is, parent_i_cnt);
				return 0;
			} else {
				int prev_i = i+1;
				long prev_suc = node.idx;

				node_t suc;
				suc.idx = node.childs[prev_i];
				load_node(&suc);

				int new_parent_cnt = 0;
				int new_parent_i_cnt = 0;

				parents[--new_parent_cnt] = prev_suc;
				parent_is[--new_parent_i_cnt] = prev_i;

				while (suc.child_cnt) {
					prev_suc = suc.idx;
					prev_i = 0;

					suc.idx = suc.childs[prev_i];
					load_node(&suc);

					parents[--new_parent_cnt] = prev_suc;
					parent_is[--new_parent_i_cnt] = prev_i;
				}

				rec_t _rec = suc.recs[0];

				node.recs[i] = _rec;
				save_node(&node);

				return remove_rec_2(suc.idx, &_rec, parents+new_parent_cnt, parent_cnt-new_parent_cnt, parent_is+new_parent_i_cnt, parent_i_cnt-new_parent_i_cnt);
			}
		}
	}

	if (node.child_cnt) {
		parents[-1] = node.idx;
		parent_is[-1] = node.child_cnt-1;
		return remove_rec_2(node.childs[node.child_cnt-1], rec, parents-1, parent_cnt+1, parent_is-1, parent_i_cnt+1);
	} else return -1;
}

int remove_rec(long node_idx, rec_t *rec) {
	long parents[100]; /* FIXME */
	int parent_is[100]; /* FIXME */
	return remove_rec_2(node_idx, rec, parents+sizeof(parents)/sizeof(parents[0]), 0, parent_is+sizeof(parent_is)/sizeof(parent_is[0]), 0);
}

#define DELIMITER "\n"
void print_node_2(FILE *fp, int node_idx) {
	int i;

	node_t node;
	node.idx = node_idx;
	load_node(&node);

//	printf("(");

	bool done = false;
	i = 0;
	while (i < ORDER_3) {
		if (i < node.child_cnt) {
			if (done) fprintf(fp, "%s", DELIMITER);
			done = true;

			print_node_2(fp, node.childs[i]);
		}

		if (i < node.rec_cnt) {
			if (done) fprintf(fp, "%s", DELIMITER);
			done = true;

//			rec_print(&node.recs[i]);
			fprintf(fp, "%s", node.recs[i].name);
		}

		i++;
	}

//	printf(")");
}

void print_node(FILE *fp, int node_idx) {
	print_node_2(fp, node_idx);
	fprintf(fp, "\n");
}

int get_cnt(int node_idx) {
	node_t node;
	node.idx = node_idx;
	load_node(&node);

	int res = 0;

	int i;
	i = 0;
	while (i < ORDER_3) {
		if (i < node.child_cnt) {
			res += get_cnt(node.childs[i]);
		}

		if (i < node.rec_cnt) {
			res++;
		}

		i++;
	}

	return res;
}

void input() {
	rec_t rec;
	char line[100];
	int idx;

	printf("Inserting nodes...\n");

	FILE *fin = fopen("../names_200k.txt", "r");

	idx = 1;
	while (fgets(line, sizeof(line), fin)) {
		if (*line) {
			char *end = line+strlen(line)-1;
			if (*end == '\n') *end = '\0';
		}

		snprintf(rec.name, sizeof(rec.name), "%s", line);
		rec.x = idx;
		rec.y = idx++;
		*rec.addr = '\0';

		insert_rec(root_idx, &rec);
	}

	fclose(fin);
}

void process() {
	int i;
	char line[100];
	char cmd, arg[100];
	bool done;
	rec_t rec;

	done = false;
	while (1) {
		printf("> ");
		fflush(stdout);

		if (!fgets(line, sizeof(line), stdin)) break;

		sscanf(line, "%c %[^\n]", &cmd, arg);

		switch (cmd) {
			FILE *fout, *fin;

			case 'i':
				snprintf(rec.name, sizeof(rec.name), "%s", arg);
				rec.x = 0;
				rec.y = 0;
				*rec.addr = '\0';

				if (insert_rec(root_idx, &rec)) {
					fprintf(stderr, "Duplicate found.\n");
				}
				break;

			case 'r':
				snprintf(rec.name, sizeof(rec.name), "%s", arg);
				remove_rec(root_idx, &rec);
				break;

			case 's':
				snprintf(rec.name, sizeof(rec.name), "%s", arg);
				if (!search_rec(root_idx, &rec)) {
					printf("Record found: ");
					rec_print(&rec);
					printf("\n");
				} else {
					fprintf(stderr, "Record not found.\n");
				}
				break;

			case 'p':
				fout = fopen("pout.txt", "w");
				print_node(fout, root_idx);
				fclose(fout);
				break;

			case 'k':
				fin = fopen("../names_200k.txt", "r");
				for (i=0;i<10000;i++) {
					if (!fgets(line, sizeof(line), fin)) break;

					if (*line) {
						char *end = line+strlen(line)-1;
						if (*end == '\n') *end = '\0';
					}

					snprintf(rec.name, sizeof(rec.name), "%s", line);
					remove_rec(root_idx, &rec);
				}
				fclose(fin);

				fout = fopen("pout.txt", "w");
				print_node(fout, root_idx);
				fclose(fout);
				break;

			case 'q':
				done = true;
				break;

			default:
				fprintf(stderr, "Invalid input.\n");
				break;
		}

		if (done) break;
	}

}

int main(void) {
	fp = fopen("data.bin", "w+b");

	node_t root;
	root.rec_cnt = 0;
	root.child_cnt = 0;

	add_node(&root);

	root_idx = root.idx;

	input();

	process();

	fclose(fp);

	return 0;
}
