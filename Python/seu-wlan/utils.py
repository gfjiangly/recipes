# -*- encoding:utf-8 -*-
# @Time    : 2019/9/20 22:22
# @Author  : gfjiang
# @Site    : 
# @File    : utils.py
# @Software: PyCharm
import logging
import time


def get_logger(log_level, name=None):
    logging.basicConfig(
        format='%(asctime)s - %(levelname)s - %(message)s', level=log_level)
    logger = logging.getLogger(name)
    return logger


def logger_file_handler(logger,
                        filename=None,
                        mode='w',
                        level=logging.INFO):
    file_handler = logging.FileHandler(filename, mode)
    file_handler.setFormatter(
        logging.Formatter('%(asctime)s - %(levelname)s - %(message)s'))
    file_handler.setLevel(level)
    logger.addHandler(file_handler)
    return logger


def get_time_str(form='%Y%m%d_%H%M%S'):
    return time.strftime(form, time.localtime())


