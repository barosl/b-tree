#!/usr/bin/env python
# -*- coding: utf-8 -*-

class BNode:
	def __init__(self, order):
		self.order = order
		self.order_2 = order*2
		self.order_3 = order*2+1
		self.recs = []
		self.childs = []

	def __str__(self):
		output = []

		for i in xrange(self.order_3):
			if i < len(self.childs): output.append(str(self.childs[i]))
			if i < len(self.recs): output.append('[%s]' % str(self.recs[i]))

		return '(%s)' % ' '.join(output)

	def insert_rec(self, rec, parents=[], parent_is=[]):
		cur_i = len(self.recs)
		for i, _rec in enumerate(self.recs):
			if _rec > rec:
				cur_i = i
				break
			elif _rec == rec:
				raise ValueError

		if self.childs:
			self.childs[cur_i].insert_rec(rec, parents+[self], parent_is+[cur_i])
		else:
			self.recs.insert(cur_i, rec)
			self.insert_rec_adjust(parents, parent_is)

	def insert_rec_adjust(self, parents, parent_is):
		parent = parents[-1] if parents else None
		parent_i = parent_is[-1] if parent_is else -1

		if len(self.recs) <= self.order_2:
			return

		mid_rec = self.recs[len(self.recs)/2]

		left = BNode(self.order)
		left.recs = self.recs[:len(self.recs)/2]
		left.childs = self.childs[:len(self.childs)/2]

		right = BNode(self.order)
		right.recs = self.recs[len(self.recs)/2+1:]
		right.childs = self.childs[len(self.childs)/2:]

		self.childs = [left, right]

		if parent:
			parent.recs = parent.recs[:parent_i] + [mid_rec] + parent.recs[parent_i:]
			parent.childs = parent.childs[:parent_i] + [left, right] + parent.childs[parent_i+1:]
			parent.insert_rec_adjust(parents[:-1], parent_is[:-1])
		else:
			self.recs = [mid_rec]

	def remove_rec(self, rec, parents=[], parent_is=[]):
		for i, _rec in enumerate(self.recs):
			if _rec > rec:
				if self.childs: return self.childs[i].remove_rec(rec, parents+[self], parent_is+[i])
				else: raise ValueError
			elif _rec == rec:
				if not self.childs:
					self.recs[i:i+1] = []
					return self.remove_rec_adjust(parents, parent_is)
				else:
					prev_i = i+1
					prev_suc = self
					suc = prev_suc.childs[prev_i]

					new_parents = parents + [prev_suc]
					new_parent_is = parent_is + [prev_i]

					while suc.childs:
						prev_suc = suc
						prev_i = 0
						suc = suc.childs[prev_i]

						new_parents.append(prev_suc)
						new_parent_is.append(prev_i)

					_rec = suc.recs[0]

					self.recs[i] = _rec
					return suc.remove_rec(_rec, new_parents, new_parent_is)
		else:
			if self.childs: return self.childs[-1].remove_rec(rec, parents+[self], parent_is+[len(self.childs)-1])
			else: raise ValueError

	def remove_rec_adjust(self, parents, parent_is):
		parent = parents[-1] if parents else None
		parent_i = parent_is[-1] if parent_is else -1

		if not parent:
			if not self.recs and self.childs: return self.childs[0]
			return

		elif len(self.recs) >= self.order:
			return

		elif parent_i+1 < len(parent.childs) and len(parent.childs[parent_i+1].recs) > self.order:
			self.recs.append(parent.recs[parent_i])
			if parent.childs[parent_i+1].childs:
				self.childs.append(parent.childs[parent_i+1].childs[0])
				parent.childs[parent_i+1].childs[0:1] = []

			parent.recs[parent_i] = parent.childs[parent_i+1].recs[0]
			parent.childs[parent_i+1].recs[0:1] = []

		elif parent_i-1 >= 0 and len(parent.childs[parent_i-1].recs) > self.order:
			self.recs.insert(0, parent.recs[parent_i-1])
			if parent.childs[parent_i-1].childs:
				self.childs.insert(0, parent.childs[parent_i-1].childs[-1])
				parent.childs[parent_i-1].childs[-1:] = []

			parent.recs[parent_i-1] = parent.childs[parent_i-1].recs[-1]
			parent.childs[parent_i-1].recs[-1:] = []

		elif parent_i+1 < len(parent.childs):
			self.recs = self.recs + [parent.recs[parent_i]] + parent.childs[parent_i+1].recs
			self.childs += parent.childs[parent_i+1].childs

			parent.recs[parent_i:parent_i+1] = []
			parent.childs[parent_i+1:parent_i+2] = []

			return parent.remove_rec_adjust(parents[:-1], parent_is[:-1])

		elif parent_i-1 >= 0:
			self.recs = parent.childs[parent_i-1].recs + [parent.recs[parent_i-1]] + self.recs
			self.childs = parent.childs[parent_i-1].childs + self.childs

			parent.recs[parent_i-1:parent_i] = []
			parent.childs[parent_i-1:parent_i] = []

			return parent.remove_rec_adjust(parents[:-1], parent_is[:-1])

		else:
			assert False

	def search_rec(self, key):
		for i, _rec in enumerate(self.recs):
			if _rec > key:
				if self.childs: return self.childs[i].search_rec(key)
				else: raise ValueError
			elif _rec == key:
				return _rec
		else:
			if self.childs: return self.childs[-1].search_rec(key)
			else: raise ValueError

class BTree:
	def __init__(self, order):
		self.order = order
		self.root = BNode(self.order)

	def __str__(self):
		return str(self.root)

	def insert_rec(self, rec):
		self.root.insert_rec(rec)

	def remove_rec(self, rec):
		new_root = self.root.remove_rec(rec)
		if new_root: self.root = new_root

	def search_rec(self, rec):
		return self.root.search_rec(rec)
