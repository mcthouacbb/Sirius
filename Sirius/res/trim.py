import sys

file = open(sys.argv[1], "r")

lines = file.readlines()

modified_lines = []

for line in lines:
	if not line.startswith('[') and len(line) > 1:
		if line.endswith(' \n'):
			line = line[:-1]
		elif not line.endswith('*\n'):
			line = line[:-1]
			line = list(line)
			line.append(' ')
			line = "".join(line)
		modified_lines.append(line)

file2 = open(sys.argv[2], "w")
file2.write("".join(modified_lines))