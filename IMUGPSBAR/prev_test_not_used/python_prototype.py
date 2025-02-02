import websocket
import json
import threading
import asyncio
import queue
import math

types_order = ["android.sensor.accelerometer","android.sensor.gyroscope","android.sensor.magnetic_field","android.sensor.pressure", "gps"]
collected_values = [None] * len(types_order)
ctrl = 0

def on_IMU(values):

    global ctrl
    global collected_values

    if ctrl == 0:
        gps_sensor.send_requests("getLastKnownLocation")

    ctrl += 1
    
    data, type = values['values'], values['type']    

    collected_values[types_order.index(type)] = data
    #print(f"collected_values = {collected_values}")

    if None not in collected_values:
        #print(f"IMU = {collected_values}")
        calc_vel(collected_values)

        ctrl = 0
        collected_values = [None] * len(types_order)

def on_GPS(values):
    global collected_values

    if values['lastKnowLocation']:
        lat, lon, alt, time = values['latitude'], values['longitude'], values['altitude'], values['time']
        ar = [lat, lon, alt, time]
        #print(f"GPS = {ar}")
        collected_values[types_order.index('gps')] = ar

lat, lon, alt, time = 0, 0, 0, 0

def calc_vel(arr):

    global lat, lon, alt, time

    R = 6371000

    lat1, lon1, alt1, time1 = arr[4]

    if lat == 0:
        lat, lon, alt, time = lat1, lon1, alt1, time1
    
    else:
        lat2, lon2, alt2, time2 = lat, lon, alt, time
        lat, lon, alt, time = lat1, lon1, alt1, time1

        dlat = math.radians(lat2 - lat1)
        dlon = math.radians(lon2 - lon1)
        dalt = alt2 - alt1

        delta_y = R * dlat
        delta_x = R * dlon
        dt = (time2 - time1)/1000

        if dt != 0:
            vel_x = delta_x/dt
            vel_y = delta_y/dt
            vel_z = dalt/dt
        else:
            vel_x = 0
            vel_y = 0
            vel_z = 0

        payload = [arr[0][0], arr[0][1], arr[0][2], arr[1][0], arr[1][1], arr[1][2], arr[2][0], arr[2][1], arr[2][2], arr[4][0], arr[4][1], arr[4][2], arr[4][3], vel_x, vel_y, vel_z, arr[3][0]]
        print(f"payload = {payload}")
        

class Sensor:
    def __init__(self, address, sensor_type, on_sensor_event):
        self.address = address
        self.sensor_type = sensor_type
        self.on_sensor_event = on_sensor_event
        self.ws = None
        self.message_queue = queue.Queue()  # Fila para mensagens a serem enviadas

    def on_message(self, ws, message):
        values = json.loads(message)
        self.on_sensor_event(values=values)

    def on_error(self, ws, error):
        print(f"Error occurred: {error}")

    def on_close(self, ws, close_code, reason):
        print(f"Connection closed: {reason}")

    def on_open(self, ws):
        print(f"Connected to: {self.address}")

    def make_websocket_connection(self):
        # Método de conexão do WebSocket
        self.ws = websocket.WebSocketApp(
            f"ws://{self.address}{self.sensor_type}",
            on_open=self.on_open,
            on_message=self.on_message,
            on_error=self.on_error,
            on_close=self.on_close,
        )

        # Loop para enviar mensagens da fila
        def run_with_queue():
            while True:
                try:
                    # Bloqueia até que uma mensagem esteja na fila
                    message = self.message_queue.get(block=True)
                    if self.ws and self.ws.sock and self.ws.sock.connected:
                        self.ws.send(message)
                    else:
                        print("WebSocket is not connected.")
                except Exception as e:
                    print(f"Error sending message: {e}")
                    break

        # Inicia a thread para enviar mensagens
        threading.Thread(target=run_with_queue, daemon=True).start()

        # Inicia a execução do WebSocket
        self.ws.run_forever()

    # Método para conectar o WebSocket em uma nova thread
    def connect(self):
        thread = threading.Thread(target=self.make_websocket_connection)
        thread.start()

    # Método para enviar mensagens para o WebSocket
    def send_requests(self, message):
        self.message_queue.put(message)  # Adiciona a mensagem na fila



# Instância dos sensores
address = "192.168.1.19:8080"

imu_sensor = Sensor(
    address=address,
    sensor_type='/sensors/connect?types=["android.sensor.accelerometer","android.sensor.gyroscope","android.sensor.magnetic_field","android.sensor.pressure"]',
    on_sensor_event=on_IMU,
)

gps_sensor = Sensor(
    address=address,
    sensor_type="/gps",
    on_sensor_event=on_GPS,
)

# Conectar os sensores
imu_sensor.connect()
gps_sensor.connect()

