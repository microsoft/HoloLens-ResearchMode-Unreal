# IMU Sensors

All IMU sensors are inherited from Hololens Sensor class. 

- IMU Sensors can be created using **Create Sensor** function

![Create Sensor function with all arguments](images/create-sensor.png)

- IMU Sensors should be started to get the data

![Start Capturing](images/start-capturing.png)

- IMU Sensors can be stopped to save resources if no longer needed

![Stop Capturing](images/stop-capturing.png)

## Accelerometer

Accelerometer provide data in the Accelerometer Frame. The Accelerometer Frame has current sampling parameters and host time in milliseconds.

![Get Accelerometer Frame](images/accelerometer.png)

Accelerometer has own fixed position related to the device. You can request this position by calling **Get Extrinsincs Matrix**

![Get Extrinsincs Matrix from Accelerometer](images/accel-matrix.png)

## Gyroscope

Gyroscope provide data in the Gyroscope Frame. The Gyroscope Frame has current sampling parameters and host time in milliseconds.

![Get Gyroscope Frame](images/gyroscope.png)

Gyroscope has own fixed position related to the device. You can request this position by calling **Get Extrinsincs Matrix**

![Get Extrinsincs Matrix from Gyroscope](images/gyro-matrix.png)

## Magnetometer

Magnetometer provide data in the Magnetometer Frame. The Magnetometer Frame has current sampling parameters and host time in milliseconds.

![Get Magnetometer Frame](images/magnetometer.png)
