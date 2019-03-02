from mainwindow import *
import sys
import requests
import json
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

dev_num = 0
dev_dict = {}


class Thread(QThread):
    trigger = pyqtSignal()
    def __init__(self,url,head):
        super().__init__()
        self.url = url
        self.head = head

    def run(self):
        self.get_dat()
        self.trigger.emit()

    def get_dat(self):
        global dev_num
        url = self.url+'/datastreams/Online_Device'
        msg_dict = self.get_url(url,self.head)
        dev_num = int(msg_dict['data']['current_value'])
        temp_list = dev_dict.keys()
        for item in temp_list:
            url = self.url+'/datastreams/'+item
            msg_dict = self.get_url(url,self.head)
            dev_dict[item] = msg_dict['data']['current_value']


    def get_url(self,url,head):
        try:
            msg = requests.get(url=url, headers=head)
            msg.raise_for_status()
            msg.encoding = msg.apparent_encoding
            msg_dict = json.loads(msg.text)
            return msg_dict
        except:
            print('Error')



class Smart_Light(QWidget,Ui_MainWidget):
    url = 'http://api.heclouds.com/devices/503223399'
    head = {'api-key': '2ar=0yznUqoLTIw6t4P2fOIprog='}
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.work = Thread(self.url,self.head)
        self.time = QTimer()
        self.setStyleSheet('QWidget{background-color:rgb(255,255,255)}')
        self.setWindowOpacity(1)
        msg_dict = self.get_url(self.url,self.head)
        self.label.setText("协议："+msg_dict['data']['protocol'])
        if msg_dict['data']['online'] == False:
            self.label_2.setText("状态：离线")
        else:
            self.label_2.setText("状态：在线")
        self.label_3.setText("上传时间："+msg_dict['data']['last_ct'])
        for item in msg_dict['data']['datastreams']:
            if item['id'] != 'Online_Device':
                dev_dict[item['id']] = '0'
        self.init_data()
        self.time.start(1000)
        self.slots()


    def init_data(self):
        global dev_num
        self.work.get_dat()
        self.label_4.setText("在线设备：" + str(dev_num))
        for i in range(dev_num):
            self.listWidget.addItem("         设备" + str(i+1))
        currentrow = self.listWidget.currentRow()
        dev_name = list(dev_dict.keys())
        state =  dev_dict[dev_name[currentrow]][4:6]
        bright = dev_dict[dev_name[currentrow]][:2]
        per = dev_dict[dev_name[currentrow]][2:4]
        self.label_5.setText('MAC地址：'+dev_name[currentrow])
        if int(state):
            self.label_10.setText('状态：开')
        else:
            self.label_10.setText('状态：关')
        self.label_8.setNum(int(bright))
        self.label_9.setNum(int(per))
        self.dial.setValue(int(bright))
        self.dial_2.setValue(int(per))


    def display_data(self):
        global dev_num
        msg_dict = self.get_url(self.url, self.head)
        self.label.setText("协议：" + msg_dict['data']['protocol'])
        if msg_dict['data']['online'] == False:
            self.label_2.setText("状态：离线")
        else:
            self.label_2.setText("状态：在线")
        currentrow = self.listWidget.currentRow()
        dev_name = list(dev_dict.keys())
        state =  dev_dict[dev_name[currentrow]][4:6]
        bright = dev_dict[dev_name[currentrow]][:2]
        per = dev_dict[dev_name[currentrow]][2:4]
        self.label_5.setText('MAC地址：'+dev_name[currentrow])
        if int(state):
            self.label_10.setText('状态：开')
        else:
            self.label_10.setText('状态：关')
        self.label_8.setNum(int(bright))
        self.label_9.setNum(int(per))



    def set_bright(self):
        url = 'http://api.heclouds.com/cmds?device_id=503223399'
        head = {'api-key': '2ar=0yznUqoLTIw6t4P2fOIprog=', 'Content-Length': '14'}
        currentrow = self.listWidget.currentRow()
        dev_name = list(dev_dict.keys())
        mac = dev_name[currentrow]
        dev_dict[mac] = str(self.dial.value()).zfill(2)+dev_dict[mac][2:6]
        cmd_msg = mac+'10'+dev_dict[mac]
        print(cmd_msg)
        response = requests.post(url, cmd_msg, headers=head)
        self.textBrowser.append(response.text)

    def set_per(self):
        url = 'http://api.heclouds.com/cmds?device_id=503223399'
        head = {'api-key': '2ar=0yznUqoLTIw6t4P2fOIprog=', 'Content-Length': '14'}
        currentrow = self.listWidget.currentRow()
        dev_name = list(dev_dict.keys())
        mac = dev_name[currentrow]
        dev_dict[mac] = dev_dict[mac][0:2]+str(self.dial_2.value()).zfill(2)+dev_dict[mac][4:6]
        cmd_msg = mac+'11'+dev_dict[mac]
        print(cmd_msg)
        response = requests.post(url, cmd_msg, headers=head)
        self.textBrowser.append(response.text)


    def slots(self):
        self.time.timeout.connect(self.work.start)
        self.work.trigger.connect(self.display_data)
        self.listWidget.currentRowChanged.connect(self.display_data)
        self.dial.valueChanged.connect(self.set_bright)
        self.dial_2.valueChanged.connect(self.set_per)



    def get_url(self,url,head):
        try:
            msg = requests.get(url=url, headers=head)
            msg.raise_for_status()
            msg.encoding = msg.apparent_encoding
            msg_dict = json.loads(msg.text)
            return msg_dict
        except:
            return 'Error'


if __name__ == '__main__':
    app = QApplication(sys.argv)
    w = Smart_Light()
    w.show()

    sys.exit(app.exec_())

