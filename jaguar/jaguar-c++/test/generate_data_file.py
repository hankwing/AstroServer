'''This script generates 170 thousand lines of data stamped with current time'''

import random
import time
import fcntl


def generate():
    ''' The main function'''
    star_num = 176000
  
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

    file_handle = open('/home/wamdm/jaguar/jaguar-go/data/' + file_name + '.txt', 'w')
    fcntl.flock(file_handle, fcntl.LOCK_EX | fcntl.LOCK_NB)

    for i in range(1, star_num + 1):
        file_handle.write(str(i) + ' ' + str(random.uniform(0, 15)) + ' ' + str(time.time()) + '\n')

    fcntl.flock(file_handle, fcntl.LOCK_UN)
    file_handle.close()


if __name__ == '__main__':
    while True:
        generate()
        time.sleep(1)
