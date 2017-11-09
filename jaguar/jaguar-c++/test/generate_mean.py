import time, random, json

star_num = 1000000
mean = {}

for i in xrange(1, star_num + 1):
    mean[i] = random.uniform(5, 15)

mean_file = open('mean.py', 'w')
mean_file.write(json.dumps(mean))
mean_file.close()
