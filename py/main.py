#!/usr/bin/env python
# -*- coding: utf-8 -*-

from btree import BTree

import random

def main():
	tree = BTree(1)

	names = open('../names_uniq.txt').read().splitlines()

	random.shuffle(names)
	names = names[:10000]

	for name in names:
		tree.insert_rec(name)

	print tree

	for name in names:
		print tree.search_rec(name)

	for name in names:
		tree.remove_rec(name)

	print tree

if __name__ == '__main__':
	main()
