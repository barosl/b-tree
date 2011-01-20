#!/usr/bin/env python
# -*- coding: utf-8 -*-

from btree import BTree, BNode

def main():
	tree = BTree(1)
#	tree = BTree(2)

#	for rec in ['cat', 'ant', 'dog', 'cow', 'rat', 'pig', 'gnu']:
#	for rec in [80, 50, 100, 90, 60, 65, 70, 75, 55, 64, 51, 76, 77, 78, 200, 300, 150]:
#	for rec in [51, 11, 30, 66, 78, 2, 7, 12, 15, 33, 41, 53, 54, 68, 69, 79, 84, 93, 22, 63, 71, 76]:
#	for rec in [1, 2, 3, 6, 7, 4, 5]:
#	for rec in [10, 20, 5, 6, 15, 16, 25, 26]:
#	for rec in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]:
#	for rec in [1, 2, 3, 4, 5, 6, 7, 8, 9]:
#	for rec in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]:
	for rec in [1, 2, 3]:
#		print '* insert %s' % rec
		tree.insert_rec(rec)
#		print tree

	a1 = BNode(2)
	a1.recs = [51]

	a2 = BNode(2)
	a2.recs = [11, 30]

	a3 = BNode(2)
	a3.recs = [66, 78]

	a4 = BNode(2)
	a4.recs = [2, 7]

	a5 = BNode(2)
	a5.recs = [12, 15, 22]

	a6 = BNode(2)
	a6.recs = [33, 41]

	a7 = BNode(2)
	a7.recs = [53, 54, 63]

	a8 = BNode(2)
	a8.recs = [68, 69, 71, 76]

	a9 = BNode(2)
	a9.recs = [79, 84, 93]

	a1.childs = [a2, a3]
	a2.childs = [a4, a5, a6]
	a3.childs = [a7, a8, a9]

	tree.root = a1

	print tree

#	for rec in ['cat', 'ant', 'dog', 'cow', 'rat', 'pig', 'gnu']:
	for rec in [68, 51, 63, 53]:
#	for rec in [51, 11, 30, 66, 78, 2, 7, 12, 15, 33, 41, 53, 54, 68, 69, 79, 84, 93, 22, 63, 71, 76]:
#	for rec in [1, 2, 3, 6, 7, 4, 5]:
#	for rec in [15]:
#	for rec in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]:
#	for rec in [1, 2, 3, 4, 5, 6, 7, 8, 9]:
#	for rec in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]:
#	for rec in [1, 2, 3]:
		print '* remove %s' % rec
		tree.remove_rec(rec)
		print tree

if __name__ == '__main__':
	main()
