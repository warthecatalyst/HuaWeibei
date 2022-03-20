import random

f = open("result.txt",'w')
for i in range(100):
    ranint = random.randint(20000,100000)
    f.write(str(ranint)+"\n")
f.close()