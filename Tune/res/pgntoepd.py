import chess
import chess.pgn
import sys
import os

pgn_file = open(sys.argv[1], "r")
epd_file = open(os.path.splitext(sys.argv[1])[0] + ".epd", "w")

curr_game = chess.pgn.read_game(pgn_file)

board = chess.Board()

while curr_game is not None:
    curr_node = curr_game
    board = chess.Board()
    while curr_node.next() is not None:
        curr_node = curr_node.next()
        board.push(curr_node.move)

        if curr_node.comment == "book":
            #print("Skipping book move")
            continue
        if curr_node.comment[1] == 'M':
            #print("Skipping mate score")
            continue
        #evalScore = int(float(curr_node.comment[0:curr_node.comment.find('/')]) * 100)
        #evalDepth = int(curr_node.comment[curr_node.comment.find('/') + 1:curr_node.comment.find(' ')])
        #clock = float(curr_node.comment[curr_node.comment.find(' ') + 1:curr_node.comment.find('s')])
        #print(evalScore, evalDepth, clock)

        epd_file.write(board.epd() + " " + curr_game.headers.get("Result") + '\n')
    curr_game = chess.pgn.read_game(pgn_file)