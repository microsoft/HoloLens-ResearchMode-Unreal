# IMU Sensors

All IMU sensors are inherited from the HoloLens Sensor class. 

- Create an IMU Sensors using the **Create Sensor** function

![Create Sensor function with all arguments](images/create-sensor.png)

- Start the IMU Sensor to start capturing data

![Start Capturing](images/start-capturing.png)

- Stop the IMU Sensor to save resources when capturing data is no longer needed

![Stop Capturing](images/stop-capturing.png)

## Accelerometer

The Accelerometer provides data in the Accelerometer Frame. The Accelerometer Frame has current sampling parameters and host time in milliseconds.

![Get Accelerometer Frame](images/accelerometer.png)

The Accelerometer has its own fixed position relative to the device. You can request this position by calling **Get Extrinsincs Matrix**

![Get Extrinsincs Matrix from Accelerometer](images/accel-matrix.png)

## Gyroscope

The Gyroscope provides data in the Gyroscope Frame. The Gyroscope Frame has current sampling parameters and host time in milliseconds.

![Get Gyroscope Frame](images/gyroscope.png)

The Gyroscope has its own fixed position related to the device. You can request this position by calling **Get Extrinsincs Matrix**

![Get Extrinsincs Matrix from Gyroscope](images/gyro-matrix.png)

## Magnetometer

The Magnetometer provides data in the Magnetometer Frame. The Magnetometer Frame has current sampling parameters and host time in milliseconds.

![Get Magnetometer Frame](images/magnetometer.png)
