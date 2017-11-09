'''This script generates 176 thousand lines of data with randomly selected anomalous patterns stamped with current time'''

import random, time, os, json

TEMP_DIR = '../temp/'
temp_name_list = os.listdir(TEMP_DIR)
mean = {}

def generate():
    ''' The main function'''
    star_num = 176000
    
    print "Write side of pipe opened, waiting for the server to open the read side..."
    pipe_handle = open('../pipe/namedpipe', 'w')
    print "Pipe connected"
    print "\n"

    print "Loading mean file..."
    mean_file = open('mean.py', 'r')
    global mean
    mean = json.loads(mean_file.read())
    print "Mean file loaded"
    print "\n"
    
    anomaly_happening = False

    temp_list = []
    temp_len = 0

    first_num = -1
    second_num = -1

    cursor = -1

    offset = 0

    while True:

        current_time = time.localtime()

        tm_year = str(current_time.tm_year)

        tm_mon = str(current_time.tm_mon)
        if current_time.tm_mon < 10:
            tm_mon = '0' + tm_mon

        tm_mday = str(current_time.tm_mday)
        if current_time.tm_mday < 10:
            tm_mday = '0' + tm_mday

        tm_hour = str(current_time.tm_hour)
        if current_time.tm_hour < 10:
            tm_hour = '0' + tm_hour

        tm_min = str(current_time.tm_min)
        if current_time.tm_min < 10:
            tm_min = '0' + tm_min

        tm_sec = str(current_time.tm_sec)
        if current_time.tm_sec < 10:
            tm_sec = '0' + tm_sec

        file_name = tm_year + '_' + tm_mon + '_' + tm_mday + '_' \
                  + tm_hour + '_' + tm_min + '_' + tm_sec

        print "Begin writing the current batch of data, timestamp:" + file_name

        pipe_handle.write('start\n')
        pipe_handle.write(file_name + '\n')

        if not anomaly_happening:
            # generate anomalous patterns with probability 0.1
            if random.randint(0, 10) == 0:
                anomaly_happening = True
                print "Anomaly injection trigged"

                first_num = random.randint(1, star_num)
                second_num = random.randint(1, star_num)
                while second_num == first_num:
                    second_num = random.randint(1, star_num)
                print "Num of stars chosen to be injected:" + str(first_num) + ', ' + str(second_num)

                temp_name = ''.join(random.sample(temp_name_list, 1))
                print "Anomaly type: " + temp_name

                temp_file = open(os.path.join(TEMP_DIR, temp_name), 'r')
                temp_file_lines = temp_file.readlines()
                temp_len = int(temp_file_lines[0])
                print "Anomaly length:" + str(temp_len)

                temp_list = [float(elem) for elem in temp_file_lines[1].split()]
                cursor = 0
                print "Position of anomaly template: " + str(cursor) 
                offset = temp_list[cursor]
            else:
                print "No anomaly happened"
                offset = 0
        else:
            cursor += 1
            if cursor < temp_len:
                print "Num of anomalous stars:" + str(first_num) + ', ' + str(second_num)
                print "Anomaly length:" + str(temp_len)
                print "Position of anomaly template: " + str(cursor)
                offset = temp_list[cursor]
            else:
                print "Anomaly ceased"
                anomaly_happening = False
                offset = 0

        for i in xrange(1, star_num + 1):
            if i == first_num or i == second_num:
                pipe_handle.write(str(i) + ' ' + str(mean[str(i)] + offset) + ' ' + str(time.time()) + '\n')
            else:
                pipe_handle.write(str(i) + ' ' + str(mean[str(i)] + random.uniform(-0.5, 0.5)) + ' ' + str(time.time()) + '\n')

        pipe_handle.write('end\n')

        print "Finish the writing of current batch" 

        print "\n"
        
        time.sleep(3)

    pipe_handle.close()


if __name__ == '__main__':
    generate()
        
