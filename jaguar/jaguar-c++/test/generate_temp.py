import os
import os.path

TEMP_DIR = '../temp/'
temp_list = os.listdir(TEMP_DIR)

for name in temp_list:
    fullname = os.path.join(TEMP_DIR, name)
    f = open(fullname, 'r')
    mean_value = 0
    flag = False
    temp = []
    for line in f.readlines():
        if line[0] == '#':
            continue
        num_and_value = line.split()
        if not flag:
            mean_value = float(num_and_value[1])
            flag = True
        else:
            n_value = float(num_and_value[1]) - mean_value
            if abs(n_value) < 0.01:
                pass
            else:
                temp.append(n_value)
    f.close()
    tempname = os.path.join(TEMP_DIR, name[:12] + '.temp')
    f = open(tempname, 'w')
    f.write(str(len(temp)))
    f.write('\n')
    f.write(' '.join([str(elem) for elem in temp]))
    f.write('\n')
    f.close()

