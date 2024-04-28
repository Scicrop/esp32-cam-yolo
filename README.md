# esp32-cam-yolo
Objects and Animals detection with Wifi camera and Yolo

**This content is part of SCICROP-ACADEMY learning series.**

## Linux Dev Environment

Check your *poetry* version, it needs to be >= 1.82

`poetry --version`

You can install poery manually with this command:

```
curl -sSL https://install.python-poetry.org | python3 -
```
However thru manual poetry istallation it will be located in your user folder, like this *~/.local/bin/poetry*

```
git clone https://github.com/Scicrop/esp32-cam-yolo
poetry update
cd python/
curl -O https://pjreddie.com/media/files/yolov3.weights
cp .env_template .env
poetry run python app.py

```
## Windows 11 Dev environment
- Install Python 3.12
- Install Poetry

### Windows 11 Packaging
- Run `poetry.exe run build`
- Install Inno Setup
- Build setup application with Inno Setup, using dist/setup.iss file

## Windows 11 User Installation

### Before you run
- If you bought your own **esp32-cam** you will need to install the arduino **WifiCam.ino** program located at https://github.com/Scicrop/esp32-cam-yolo/tree/master/arduino/WifiCam, then plug it to your usb port.
- If you received your **esp32-cam** from SciCrop, you just need to plug it in your usb port.

### Installation
- Get the latest installer version from https://github.com/Scicrop/esp32-cam-yolo/releases
- Install it in your Windows machine. In the end of installation it will ask you to install the USB Camera driver, don't forget to install it. If necessary restart your PC.
- Go to your programs, find **esp32-cam-yolo** and run it as **administrator**.

## Results
https://github.com/Scicrop/esp32-cam-yolo/assets/692043/433036f7-b282-4ae5-81f0-fcfb3fb30fed


