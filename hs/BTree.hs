module BTree (BTree(BNode), tree, recs, childs, insert, remove, search) where

import qualified Data.List as List

data BTree rec = BNode Int [rec] [BTree rec]

_show [] [] = []
_show [] childs =
	show (head childs) :
	_show [] (tail childs)
_show recs [] =
	("[" ++ show (head recs) ++ "]") :
	_show (tail recs) []
_show recs childs =
	show (head childs) :
	("[" ++ show (head recs) ++ "]") :
	_show (tail recs) (tail childs)

instance (Show a) => Show (BTree a) where
	show (BNode _ recs childs) = "(" ++ List.intercalate " " (_show recs childs) ++ ")"

tree order = BNode order [] []
recs (BNode _ recs _) = recs
childs (BNode _ _ childs) = childs

insert rec (BNode order recs childs) =
	if not (null childs)
		then
			let BNode _ child_recs child_childs = insert rec $! (childs !! cur_i)
			in
				if length child_recs == 1 && length (BTree.recs (childs !! cur_i)) /= 1
					then insert_adjust $! (BNode order
						((take cur_i recs) ++ [head child_recs] ++ (drop cur_i recs))
						((take cur_i childs) ++ child_childs ++ (drop (cur_i+1) childs)))
					else BNode order
						recs
						((take cur_i childs) ++ [BNode order child_recs child_childs] ++ (drop (cur_i+1) childs))
		else
			insert_adjust $! (BNode order
				((take cur_i recs) ++ [rec] ++ (drop cur_i recs))
				childs)
	where cur_i = case List.findIndex (>= rec) recs of
		Just i -> i
		Nothing -> length recs

insert_adjust (BNode order recs childs) =
	if length recs > order*2
		then BNode order
			[recs !! order]
			[BNode order
				(take order recs)
				(take (length childs `div` 2) childs),
			BNode order
				(drop (order+1) recs)
				(drop (length childs `div` 2) childs)]
		else BNode order recs childs

remove rec (BNode order recs childs)
	| not (null (BTree.recs node)) = node
	| otherwise =
		if not (null (BTree.childs node))
			then head (BTree.childs node)
			else node
	where node = remove2 rec (BNode order recs childs)

remove2 rec (BNode order recs childs) =
	if not (null childs)
		then
			if cur_i < length recs && recs !! cur_i == rec
				then
					let suc = remove_suc (childs !! (cur_i+1))
					in remove_adjust (cur_i+1) (BNode order
						((take cur_i recs) ++ [suc] ++ (drop (cur_i+1) recs))
						((take (cur_i+1) childs) ++ [(remove2 suc (childs !! (cur_i+1)))] ++ (drop (cur_i+2) childs)))
				else remove_adjust cur_i (BNode order
					recs
					((take cur_i childs) ++ [(remove2 rec (childs !! cur_i))] ++ (drop (cur_i+1) childs)))
		else BNode order
			((take cur_i recs) ++ (drop (cur_i+1) recs))
			childs
	where cur_i = case List.findIndex (>= rec) recs of
		Just i -> i
		Nothing -> length recs

remove_adjust cur_i (BNode order recs childs) =
	if length (BTree.recs child) < order
		then
			if cur_i+1 < length childs && length (BTree.recs right) > order then
				let
					rec = recs !! cur_i
					right_rec = head (BTree.recs right)
				in BNode order
					((take cur_i recs) ++ [right_rec] ++ (drop (cur_i+1) recs))
					((take cur_i childs) ++
						[BNode order
							(BTree.recs child ++ [rec])
							(BTree.childs child ++ if not (null (BTree.childs right)) then [head (BTree.childs right)] else []),
						BNode order
							(tail (BTree.recs right))
							(if not (null (BTree.childs right)) then tail (BTree.childs right) else [])] ++
						(drop (cur_i+2) childs))
			else if cur_i-1 >= 0 && length (BTree.recs left) > order then
				let
					rec = recs !! (cur_i-1)
					left_rec = last (BTree.recs left)
				in BNode order
					((take (cur_i-1) recs) ++ [left_rec] ++ (drop cur_i recs))
					((take (cur_i-1) childs) ++
						[BNode order
							(init (BTree.recs left))
							(if not (null (BTree.childs left)) then init (BTree.childs left) else []),
						BNode order
							(rec : BTree.recs child)
							((if not (null (BTree.childs left)) then [last (BTree.childs left)] else []) ++ BTree.childs child)] ++
						(drop (cur_i+1) childs))
			else if cur_i+1 < length childs then
				let rec = recs !! cur_i
				in BNode order
					((take cur_i recs) ++ (drop (cur_i+1) recs))
					((take cur_i childs) ++
						[BNode order
							(BTree.recs child ++ [rec] ++ BTree.recs right)
							(BTree.childs child ++ BTree.childs right)] ++
						(drop (cur_i+2) childs))
			else if cur_i-1 >= 0 then
				let rec = recs !! (cur_i-1)
				in BNode order
					((take (cur_i-1) recs) ++ (drop cur_i recs))
					((take (cur_i-1) childs) ++
						[BNode order
							(BTree.recs left ++ [rec] ++ BTree.recs child)
							(BTree.childs left ++ BTree.childs child)] ++
						(drop (cur_i+1) childs))
			else
				BNode order [] [] -- assert False
		else BNode order recs childs
	where
		child = childs !! cur_i
		right = childs !! (cur_i+1)
		left = childs !! (cur_i-1)

remove_suc (BNode order recs childs)
	| not (null childs) = remove_suc (head childs)
	| otherwise = head recs

search rec (BNode order recs childs) =
	case List.findIndex (>= rec) recs of
		Just i ->
			if recs !! i == rec
				then Just (recs !! i)
				else
					if i < length childs
						then search rec (childs !! i)
						else Nothing
		Nothing ->
			if not (null childs)
				then search rec (last childs)
				else Nothing
