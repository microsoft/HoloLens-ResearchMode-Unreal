# Camera Sensors

## Basic Camera Sensor

All Camera sensors are inherited from Hololens Sensor class. 

- Camera Sensors can be created using **Create Sensor** function

![Create Sensor function with all arguments](images/create-sensor.png)

- To use Camera Sensors, you should await until it's actually started, to so you need to subscribe to **On Video Texture Created** event

 ![On Video Texture Created with subscribed event](images/on-video-texture-created.png)

- Camera Sensors should be started to get the data

![Start Capturing](images/start-capturing.png)

- Camera Sensors can be stopped to save resources if no longer needed

![Stop Capturing](images/stop-capturing.png)

- After starting, Camera Sensors have **Camera Image** property, where images are being stored. That's a video texture, it can change every frame.

![Camera Image](images/camera-image.png)

- Also after starting, Camera Sensors have own resolution, you can request it by calling **Get Hololens Sensor Resolution**. The result is false if the cam isn't started yet.

![Get Hololens Sensor Resolution](images/get-hololens-sensor-resolution.png)

## Depth Cam

Depth Cam Sensors hasn't additional functions or parameters related to basic Camera Sensor. But Depth Cam Sensors has important runtime requirment: only one single sensor can be active at one time. If you want start the next depth sensor, you must stop the previous one.

## VLC Cam

VLC Cams can return its instrinsics using **Get Instrinsics**

![Get Instrinsics](images/get-intrinsics.png)
