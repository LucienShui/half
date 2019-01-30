def show(x, upper, a, b):
	for i in range(upper, -1, -1):
		if i == a or i == b:
			print(' ', end = '')
		print(x >> i & 1, end = '')
	print('')

def sf(x):
	show(x, 31, 30, 22)

def sh(x):
	show(x, 15, 14, 9)

