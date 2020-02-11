#!/usr/bin/python3
# -*- coding:utf-8 -*-
import os
import os.path as osp
import time
import getpass
import requests
import socket
import platform
import smtplib
import json
import logging

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

from utils import get_logger, logger_file_handler, get_time_str


class ConnectSeuWlan(object):

    def __init__(self,
                 recored='.record',
                 fromaddr=None,
                 toaddr=None,
                 email_key=None):
        self.recored = recored
        self.fromaddr = fromaddr
        self.toaddr = toaddr
        self.email_key = email_key
        self.record_path = recored
        self.username = ''
        self.password = ''
        self.pc_name = ''
        self.ip = ''
        self.last_ip = ''
        self.get_device_info()
        self.logger = self.init_logger()

    def connect(self):
        if self.ping():
            return
        self.get_config()
        params = {
            'c': 'Portal',
            'a': 'login',
            'callback': 'dr1003',
            'login_method': 1,
            'user_account': ',0,'+self.username,
            'user_password': self.password,
            'wlan_user_ip': self.ip,
        }
        res = requests.get("https://w.seu.edu.cn:801/eportal/", params=params)
        res = json.loads(str(res.content, 'utf-8')[7:-1])
        time.sleep(3)

        if res['msg'] != "认证成功":
            self.logger.info("认证失败：{}".format(res))
            return
        self.logger.info("认证成功")
        self.logger.info('主机名：' + self.pc_name)
        self.logger.info('IP：' + self.ip)

        if self.ip != self.last_ip:
            self.write_record()
            self.send_email()

    def get_device_info(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        self.ip = s.getsockname()[0]
        self.pc_name = socket.getfqdn(socket.gethostname())
        s.close()

    def get_config(self):
        if not osp.isfile(self.record_path):
            # self.ip = input("当前设备ip:")
            self.username = input("seu-wlan网络认证账户:")
            self.password = getpass.getpass("密码:")
            show_password = input("显示密码?[y/n]:")
            if show_password == 'y' or show_password == 'Y':
                print(self.password)
        else:
            self.read_record()

    def ping(self):
        if platform.system() == 'Windows':
            # print('当前系统为Windows，'
            #       '请在浏览器输入http://202.119.25.2认证')
            status = os.system('ping www.baidu.com')
            if status != 1:
                self.logger.info("已连接网络！")
                return True
        elif platform.system() == 'Linux':
            status = os.popen(
                "ping -c 3 www.baidu.com | grep '0 received' | wc -l").readlines()[0]
            # self.logger.info("status: {}".format(status))
            if status == '0\n':
                self.logger.info("已连接网络！")
                return True
        else:
            raise Exception("不支持的平台：{}".format(
                platform.system()))
        return False

    def send_email(self):
        if not self.fromaddr or not self.toaddr:
            return
        try:
            msg = MIMEMultipart()
            msg['From'] = fromaddr
            msg['To'] = toaddr
            # server_yy should be changed by computer name.
            msg['Subject'] = self.pc_name + " ip"
            body = '主机名：' + self.pc_name + '\nIP：' + self.ip
            msg.attach(MIMEText(body, 'plain'))
            server = smtplib.SMTP("smtp.qq.com")
            server.ehlo()
            server.starttls()
            server.login(fromaddr, self.email_key)
            server.sendmail(fromaddr, toaddr.split(','), msg.as_string())
            server.quit()
        except Exception as e:
            self.logger.info(e)

    def read_record(self):
        with open(self.record_path, 'r', encoding='utf-8') as f:
            self.username = f.readline().strip()
            self.password = f.readline().strip()
            self.pc_name = f.readline().strip()
            self.last_ip = f.readline().strip()

    def write_record(self):
        with open(self.record_path, 'w', encoding='utf-8') as f:
            f.write(self.username + '\n')
            f.write(self.password + '\n')
            f.write(self.pc_name + '\n')
            f.write(self.ip + '\n')

    def init_logger(self, log_dir=None, level=logging.INFO):
        """Init the logger.

        Args:
            log_dir(str, optional): Log file directory. If not specified, no
                log file will be used.
            level (int or str): See the built-in python logging module.

        Returns:
            :obj:`~logging.Logger`: Python logger.
        """
        logger = get_logger(level, name=__name__)
        if log_dir:
            filename = '{}.log'.format(get_time_str())
            log_file = osp.join(log_dir, filename)
            logger_file_handler(logger, log_file, level=level)
        return logger


if __name__ == '__main__':
    c = ConnectSeuWlan()
    c.connect()
