# esp32-cam-yolo
Objects and Animals detection with Wifi camera and Yolo

## Linux Installatiion

Check your *poetry* version, it needs to >= 1.82

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
