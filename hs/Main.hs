module Main where

import BTree
import System.IO
import qualified Data.List as List

order = 1

data RecordType = Record String Double Double String deriving Ord

record name x y addr = Record name x y addr

instance Show RecordType where
	show (Record name x y addr) = "[" ++ name ++ " (" ++ (show x) ++ ", " ++ (show y) ++ ")" ++ "]"

instance Eq RecordType where
	(Record name1 _ _ _) == (Record name2 _ _ _) = name1 == name2

loop tree = do
	hPutStr stdout "> "
	hFlush stdout
	line <- getLine
	case line of
		'i':' ':name -> do
			rec <- return (record name 0 0 "")
			tree <- return (insert rec tree)
			putStrLn ("Record inserted: " ++ (show rec))
			loop tree

		'r':' ':name -> do
			rec <- return (record name 0 0 "")
			tree <- return (remove rec tree)
			putStrLn ("Record removed: " ++ (show rec))
			loop tree

		's':' ':name -> do
			rec <- return (record name 0 0 "")
			rec <- return (search rec tree)
			case rec of
				Nothing -> do
					putStrLn "Record not found."
				Just rec -> do
					putStrLn ("Record found: " ++ (show rec))
			loop tree

		"p" -> do
			save_to_file "pout.txt" tree
			loop tree

		"k" -> do
			tree <- read_file_and_remove_recs "../names_200k.txt" tree
			save_to_file "pout.txt" tree
			loop tree

		"q" -> return ()

		_ -> do
			putStrLn "Invalid input"
			loop tree

load_from_fp tree fp idx = do
	is_eof <- hIsEOF fp
	if is_eof
		then return tree
		else do
			name <- hGetLine fp
			rec <- return (record name idx idx "")
			tree <- return (insert rec tree)
			load_from_fp tree fp (idx+1)

load_from_file fpath tree = do
	putStrLn "Inserting records..."
	fp <- openFile fpath ReadMode
	tree <- load_from_fp tree fp 1
	hClose fp
	return tree

read_fp_and_remove_recs tree fp cnt = do
	if cnt == 0
		then return tree
		else do
			name <- hGetLine fp
			rec <- return (record name 0 0 "")
			tree <- return (remove rec tree)
			read_fp_and_remove_recs tree fp (cnt-1)

read_file_and_remove_recs fpath tree = do
	fp <- openFile fpath ReadMode
	tree <- read_fp_and_remove_recs tree fp 10000
	hClose fp
	return tree

_save [] [] = []
_save [] childs =
	save (head childs) :
	_save [] (tail childs)
_save recs [] =
	("" ++ show (head recs) ++ "") :
	_save (tail recs) []
_save recs childs =
	save (head childs) :
	("" ++ show (head recs) ++ "") :
	_save (tail recs) (tail childs)

save (BNode _ recs childs) = "" ++ List.intercalate "\n" (_save recs childs) ++ ""
--save (BNode _ recs childs) = "" ++ List.intercalate "\n" [show recs] ++ ""

save_to_fp node fp = do
	recs node

save_to_file fpath tree = do
	fp <- openFile fpath WriteMode
	hPutStrLn fp (save tree)
	hClose fp

main = do
	tree <- return (tree order)

	tree <- load_from_file "../names_200k.txt" tree

	loop tree
