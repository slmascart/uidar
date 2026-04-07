# WE ARE MAKING A UiDAR!!
### What's a UiDAR?
It stands for Ultrasound imaging, Detection and Ranging. Inspired by the infamous LiDAR.

### What does it do?
The goal of the project is to use a couple tools from the SunFounder kit to create a kind of environment mapping object.

Using the ultrasound emitter+sensor, and a gyroscope+accelerometer, the goal is to create an object able to map a 3d environment onto a cloud point.

### ROADMAP
- [x] Ultrasonic sensor
- [x] Compute speed of sound based on measured room temperature
- [x] Display Distance and temperature on LCD
- [ ] Compute position of ultrasonic sensor in space
- [ ] Map detected obstacles as 3d point cloud
- [ ] Display real-time point cloud as 3d cube

# Usage
Executable `bin/uidar` has been compiled for raspberry pi zero 2W. Run it using `sudo ./bin/uidar`

Project can be compied using the provided Makefile by running command `make`

It will create a `.build` directory containing the object files and linker files. It can be deleted running `make clean`
