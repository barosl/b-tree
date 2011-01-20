#pragma once

#define ORDER 15
#define ORDER_2 ((ORDER)*2)
#define ORDER_3 ((ORDER)*2+1)

typedef struct {
	char name[100];
	double x, y;
	char addr[100];
} rec_t;

typedef struct {
	long idx;

	rec_t recs[ORDER_2+1];
	int rec_cnt;

	long childs[ORDER_3+1];
	int child_cnt;
} node_t;
