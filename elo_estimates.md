# Elo Estimates
## Search

All tests done at 8+0.08 1.3mnps on UHO_Lichess_4852_v1.epd<br>
All features tested against commit a049bb98 or functionally equivalent commits except for correction history, singular extensions and internal iterative reductions

**feature** indicates that the estimate was based off of a gainer test because I'm too lazy to run a test<br>
***feature*** indicates that the estimate is SSS because I'm too lazy to keep running the test

### In "proper" order
- Aspiration windows: ~108 elo https://mcthouacbb.pythonanywhere.com/test/358/
- TT Cutoffs: ~101 elo https://mcthouacbb.pythonanywhere.com/test/360/
- **Internal iterative reductions**: ~23 elo https://mcthouacbb.pythonanywhere.com/test/396/
- Improving: ~31 elo https://mcthouacbb.pythonanywhere.com/test/381/
- TT Eval adjustment: ~8 elo https://mcthouacbb.pythonanywhere.com/test/363/
- Correction history: ~91 elo https://mcthouacbb.pythonanywhere.com/test/318/
- Static Exchange Evaluation(overall): ~258 elo https://mcthouacbb.pythonanywhere.com/test/388
- Time Management: ~64 elo https://mcthouacbb.pythonanywhere.com/test/389/
- Quiescence search(overall): ~184 elo https://mcthouacbb.pythonanywhere.com/test/359/
	- QS pruning: ~55 elo https://mcthouacbb.pythonanywhere.com/test/382/
- Move ordering features
	- TT Move: ~106 elo https://mcthouacbb.pythonanywhere.com/test/364/
	- Killers: ~6 elo https://mcthouacbb.pythonanywhere.com/test/387/
	- History: ~527 elo https://mcthouacbb.pythonanywhere.com/test/375/
		- Quiet history: ~30 elo https://mcthouacbb.pythonanywhere.com/test/374/
		- Capture history: ~19 elo https://mcthouacbb.pythonanywhere.com/test/373/
		- Continuation history: ~40 elo https://mcthouacbb.pythonanywhere.com/test/372/
- Whole node pruning: ~228 elo https://mcthouacbb.pythonanywhere.com/test/367/
	- **Razoring**: ~6 elo https://mcthouacbb.pythonanywhere.com/test/205/
	- Null Move Pruning: ~31 elo https://mcthouacbb.pythonanywhere.com/test/370/
	- Reverse Futility Pruning: ~88 elo https://mcthouacbb.pythonanywhere.com/test/369/
	- **Probcut**: ~3 elo https://mcthouacbb.pythonanywhere.com/test/346/
- Move Loop Pruning: ~184 elo https://mcthouacbb.pythonanywhere.com/test/366/
	- ***Futility pruning***: ~1 elo https://mcthouacbb.pythonanywhere.com/test/376/
	- Late move pruning: ~23 elo https://mcthouacbb.pythonanywhere.com/test/371/
	- SEE pruning: ~5 elo https://mcthouacbb.pythonanywhere.com/test/377/
	- History pruning: ~14 elo https://mcthouacbb.pythonanywhere.com/test/378/
- Extensions
	- Check extensions: ~13 elo https://mcthouacbb.pythonanywhere.com/test/392/
	- Singular extensions: ~73 elo https://mcthouacbb.pythonanywhere.com/test/350/
- Late move reductions: ~111 elo https://mcthouacbb.pythonanywhere.com/test/368/

### In order of elo gain
- History: ~527 elo
- Static Exchange Evaluation(overall): ~258 elo
- Whole node pruning: ~228 elo
- Move Loop Pruning: ~184 elo
- Quiescence search(overall): ~184 elo
- Late move reductions: ~111 elo
- Aspiration windows: ~108 elo
- TT Move Ordering: ~106 elo
- TT Cutoffs: ~101 elo
- Correction history: ~91 elo
- Reverse Futility Pruning: ~88 elo
- Singular extensions: ~73 elo
- Time Management: ~64 elo
- QS pruning: ~55 elo
- Continuation history: ~40 elo
- Null Move Pruning: ~31 elo
- Improving: ~31 elo
- Quiet history: ~30 elo
- Late move pruning: ~23 elo
- **Internal iterative reductions**: ~23 elo
- Capture history: ~19 elo
- History pruning: ~14 elo
- Check extensions: ~13 elo
- TT Eval adjustment: ~8 elo
- Killers: ~6 elo
- **Razoring**: ~6 elo
- SEE pruning: ~5 elo
- **Probcut**: ~3 elo
- ***Futility pruning***: ~1 elo