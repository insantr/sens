from RF24 import *
from struct import *
import requests
from influxdb import InfluxDBClient
import time
from datetime import datetime

radio = RF24(21,0)

radio.begin()
radio.setPALevel(3); # RF24_PA_MAX
radio.setDataRate(RF24_250KBPS); # RF24_250KBPS RF24_2MBPS
radio.setChannel(125);
radio.openReadingPipe(1,"1NODE");
radio.startListening();
radio.printDetails();

influxdb_url = 'http://localhost:8086/write?db=temp_1'
influxdb_client = InfluxDBClient(host='localhost', port=8086, database='temp_1')

while(1):
        if radio.available():
                while(radio.available()):
                        r = radio.read( (13+4) )
                        (humd, temp, batt, _time, id) = unpack("fffLc", r)
                        print "H: %f; T: %f; B: %f; T: %d; Id: %d" % (humd, tem$
                        cur_time = datetime.utcnow().strftime('%Y-%m-%dT%H:%M:%$
                        data = [
{"measurement": "temperature", "tags": {"sens_id": ord(id), "sens_time": _time}$
{"measurement": "humidity", "tags": {"sens_id": ord(id), "sens_time": _time}, "$
{"measurement": "battery", "tags": {"sens_id": ord(id), "sens_time": _time}, "t$
]
                        print str(data)
                        print influxdb_client.write_points(data)
                        #f = requests.post('http://localhost:8086/write?db=temp$
#                       print request.post(influxdb_url, data = {}).text
#                       print f.text
                        #for c in r: print(c)
                        #print type(r)






