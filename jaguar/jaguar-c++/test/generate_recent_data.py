import json

LONGWINDOW_LENGTH = 2000

mean_file = open('mean.py', 'r')
mean = json.loads(mean_file.read())
recent_data_file = open('../data/recent.data', 'w')

for i in xrange(1, len(mean) + 1):
    for j in xrange(LONGWINDOW_LENGTH):
        if j != 0:
            recent_data_file.write(' ')
        recent_data_file.write(str(mean[str(i)]))
    recent_data_file.write('\n')
    print('generated lines: %d' % i)
    
recent_data_file.close()