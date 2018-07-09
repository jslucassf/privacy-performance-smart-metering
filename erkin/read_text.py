# Read text files to create datasheet

numbers = [ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200
]
SM_results = []
SM_lines = []

for j in numbers:
	file_name = str(j) + ".txt"

	results = open(file_name, "r");
	result = ""
	append = True

	for i in results:
		SM_results.append(i[:8])

for i in range(10):
	temp = ""
	for k in range(i, len(SM_results), 10):
		temp += (SM_results[k] + " ")
		
	SM_lines.append(temp)


for i in SM_lines:
	print i
#temp = ""
#for i in range(20):
#	temp += "0.0 "

#print temp
