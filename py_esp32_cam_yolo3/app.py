import platform
import re
import sys
import tempfile
import threading
import time
from urllib.parse import urlparse
from dotenv import set_key
import cv2
import numpy as np
import urllib.request
import os
from decouple import config, Config, RepositoryEnv
import requests
import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk


def yolo_detect(host):
    loop = True
    url_str = ''
    cap = None
    try:
        url_str = 'http://' + host + '/cam-lo.jpg'
        cap = cv2.VideoCapture(url_str)
    except:
        show_alert("Camera connection failed.", "Exit")
        loop = False
        sys.exit()
    whT = 320
    model_config = 'yolov3.cfg'
    model_weights = 'yolov3.weights'
    net = cv2.dnn.readNetFromDarknet(model_config, model_weights)
    net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
    net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)

    while loop:
        try:
            img_resp = None
            imgnp = None
            try:
                img_resp = urllib.request.urlopen(url_str)
                imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
            except:
                show_alert("Camera connection failed.", "Exit")
                loop = False
                sys.exit()
            im = cv2.imdecode(imgnp, -1)
            success, img = cap.read()
            blob = cv2.dnn.blobFromImage(im, 1 / 255, (whT, whT), [0, 0, 0], 1, crop=False)
            net.setInput(blob)
            layer_names = net.getLayerNames()

            output_names = [layer_names[i - 1] for i in net.getUnconnectedOutLayers()]
            outputs = net.forward(output_names)
            find_object(outputs, im)

            cv2.imshow('Realtime detection screen', im)
            key = cv2.waitKey(1)

            # Check if the user pressed any key
            if key != -1:
                print(f"Key pressed: {key}")
                # Close the window
                cv2.destroyAllWindows()
                loop = False
                sys.exit()
        except:
            pass


def read_serial_output(serial_port):
    env_file = '.env'
    show_alert("Getting Camera IP Address...", "Continue...")
    host = None
    with serial.Serial(serial_port, baudrate=115200, timeout=1) as ser:
        ser.setDTR(False)
        ser.setRTS(False)
        read = True
        counter = 0
        while read:
            counter = counter + 1
            print(counter)
            try:
                line = ser.readline().decode().strip()
                if line:
                    urls_found = re.findall(r'(https?://\S+)', line)
                    if len(urls_found) > 0:
                        for url in urls_found:
                            host = urlparse(url).hostname
                            set_key(env_file, "ESP32_CAM_IP", host)
                            show_alert("Camera IP Address: " + host, "Continue...")
                            read = False
                            break
                    elif "SoftAP" in line:
                        show_alert(
                            "You need to configure the WiFi connection of your camera.\nConnect into SCICROP network "
                            "and enter in http://192.168.4.1 to configure.\nThen run this program again.", "Exit")
                        read = False
                        sys.exit()

            except:
                pass
    yolo_detect(host)


def list_serial_ports():
    serial_ports = [port.device for port in serial.tools.list_ports.comports()]
    return serial_ports


def select_serial_port():
    selected_port = combo_ports.get()
    if selected_port:
        print("Selected serial port:", selected_port)
        window.destroy()
        read_serial_output(selected_port)
    else:
        print("No serial port selected.")


def download_yolov3_weights():
    if not os.path.exists('yolov3.weights'):
        yolo3_weights_url = get_config('YOLO3_WEIGHTS')
        if yolo3_weights_url:
            show_alert("Preparing AI detection data, it can take a while (1-5min).", "Continue...")
            response = requests.get(yolo3_weights_url)
            if response.status_code == 200:
                with open('yolov3.weights', 'wb') as f:
                    f.write(response.content)
                print("yolov3.weights file downloaded and saved in the project root.")
            else:
                print("Error downloading the yolov3.weights file.")
        else:
            print("YOLO3_WEIGHTS environment variable is not defined.")
    else:
        print("yolov3.weights file already exists in the project root.")


def save_detected_image(detected_object, i, obj_class):
    timestamp_ms = int(time.time() * 1000)
    file_name = f"detected_object_{i}_{obj_class}_{timestamp_ms}.jpg"
    cv2.imwrite(f"{sys.path[0]}/detected_files/{file_name}", detected_object)


def find_object(outputs, im):
    pref_class = get_config('PREF_CLASS')
    conf_threshold = 0.5
    nms_threshold = 0.3
    classes_file = 'coco.names'
    class_names = []
    with open(classes_file, 'rt') as f:
        class_names = f.read().rstrip('\n').split('\n')
    hT, wT, cT = im.shape
    bbox = []
    class_ids = []
    confs = []
    for output in outputs:
        for det in output:
            scores = det[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            if confidence > conf_threshold:
                w, h = int(det[2] * wT), int(det[3] * hT)
                x, y = int((det[0] * wT) - w / 2), int((det[1] * hT) - h / 2)
                bbox.append([x, y, w, h])
                class_ids.append(class_id)
                confs.append(float(confidence))
    indices = cv2.dnn.NMSBoxes(bbox, confs, conf_threshold, nms_threshold)

    for i in indices:
        box = bbox[i]
        x, y, w, h = box[0], box[1], box[2], box[3]
        # print(class_names[class_ids[i]])
        if class_names[class_ids[i]] == pref_class:
            detected_object = im[y:y + h, x:x + w]
            thread = threading.Thread(target=save_detected_image, args=(detected_object, i, pref_class))
            thread.start()

        cv2.rectangle(im, (x, y), (x + w, y + h), (255, 0, 255), 2)
        cv2.putText(im, f'{class_names[class_ids[i]].upper()} {int(confs[i] * 100)}%', (x, y - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 255), 2)


def show_alert(message, button_label):
    alert_window = tk.Tk()
    alert_window.title("Message")

    # Create a label with the message
    lbl_message = tk.Label(alert_window, text=message)
    lbl_message.pack(padx=20, pady=20)

    # Function called when the "OK" button is clicked
    def close_window():
        alert_window.destroy()

    # Create an "OK" button to close the window
    btn_ok = tk.Button(alert_window, text=button_label, command=close_window)
    btn_ok.pack(pady=10)
    alert_window.mainloop()


def is_valid_ip(ip):
    # Verifica se a string não é nula ou vazia
    if ip is None or ip.strip() == '':
        return False
    ip_pattern = r'^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$'
    if re.match(ip_pattern, ip):
        octets = ip.split('.')
        for octet in octets:
            if not (0 <= int(octet) <= 255):
                return False
        return True
    else:
        return False


def is_windows():
    return platform.system().lower() == "windows"


def check_dot_env():
    env_dest = '.env'
    template_content = ''
    if is_windows():
        temp_folder = tempfile.gettempdir()
        env_dest = temp_folder + '\\' + env_dest
    # Check if '.env' file exists
    if not os.path.exists(env_dest):
        # If '.env' does not exist, check if '.env_template' exists
        if os.path.exists('.env_template'):
            # Read content from '.env_template'
            with open('.env_template', 'r') as template_file:
                template_content = template_file.read()

            with open(env_dest, 'w') as env_file:
                env_file.write(template_content)

            print("'.env' file created from '.env_template'.")
        else:
            print("'.env_template' file does not exist. Cannot create '.env' file.")
    else:
        print("'.env' file already exists.")


def get_config(key):
    dotenv_path = ".env"
    if is_windows():
        temp_folder = tempfile.gettempdir()
        dotenv_path = f"{temp_folder}/.env"
    my_config = Config(RepositoryEnv(dotenv_path))
    return my_config(key)


check_dot_env()
download_yolov3_weights()
esp32_cam_ip = get_config('ESP32_CAM_IP')
if not is_valid_ip(esp32_cam_ip):
    available_ports = list_serial_ports()
    if len(available_ports) > 0:
        window = tk.Tk()
        window.title("Select your serial port")
        combo_ports = ttk.Combobox(window, values=available_ports)
        combo_ports.grid(row=0, column=0, padx=10, pady=10)
        btn_select = ttk.Button(window, text="Choose", command=select_serial_port)
        btn_select.grid(row=0, column=1, padx=10, pady=10)
        window.mainloop()
    else:
        show_alert("No serial port found. Please check:\n1) Is your camera connect with USB cable?\n2) Did you "
                   "installed the USB-Serial driver?", "Exit")
else:
    yolo_detect(esp32_cam_ip)
