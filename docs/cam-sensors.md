# Camera Sensors

## Basic Camera Sensor

All Camera sensors are inherited from HoloLens Sensor class. 

- Camera Sensors can be created using the **Create Sensor** function

![Create Sensor function with all arguments](images/create-sensor.png)

- To use a Camera Sensor, you need to wait until the camera has actually started. You can do this by subscribing to the **On Video Texture Created** event.

 ![On Video Texture Created with subscribed event](images/on-video-texture-created.png)

- Camera Sensors must be started to get data

![Start Capturing](images/start-capturing.png)

- Camera Sensors should be stopped to save resources when no longer needed

![Stop Capturing](images/stop-capturing.png)

- After starting, Camera Sensors have a **Camera Image** property, where images are stored. That's a video texture, it can change every frame.

![Camera Image](images/camera-image.png)

- Also after starting, Camera Sensors have their own resolution. You can request it by calling **Get Hololens Sensor Resolution**. The result is false if the camera hasn't started yet.

![Get Hololens Sensor Resolution](images/get-hololens-sensor-resolution.png)

## Depth Cam

Depth Cam Sensors don't have any additional functions or parameters beyond the basic Camera Sensor. However, Depth Cam Sensors have an important runtime requirement: only one depth sensor can be active at a time. If you want start another depth sensor, you must stop the previous one.

## VLC Cam

VLC Cams can return their instrinsics via **Get Instrinsics**

![Get Instrinsics](images/get-intrinsics.png)
