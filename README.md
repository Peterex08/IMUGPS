# IMUGPS
A project that uses data from IMU, GPS and Barometer modules to estimate position, velocity, attitude and a rocket parachute behavior based on space state filters implementation using a Extended Kalman Filter Algorithm.

## How to install everything?
To use the application you will need multiple stuff, including from android apps to c++, python and javascript libs.

 1. Download **SensorServer** in your phone to stream IMU and GPS data for Extended Kalman FIlter Analysis.
    **NOTE:** Your cellphone plays the role for a rocket, in this case, substitutes a TAU 1201 GPS Module and a MPU 9250 IMU. It does not work properly for the filtering conditions, but can be used to test all the process.
	 [umer0586 - SensorServer (Github Repo)](https://github.com/umer0586/SensorServer)
	 
 3. Download all Cpp libraries:
**Threads**:`sudo apt-get install libpthread-stubs0-dev`
**OpenSSL**:`sudo apt-get install libssl-dev`
**Boost**:`sudo apt-get install libboost-system-dev`
**Eigen3**:`sudo apt-get install libeigen3-dev`
**Paho MQTT C++**:`sudo apt-get install libpaho-mqttpp-dev libpaho-mqtt3a-dev`
**CMake**:`sudo apt-get install cmake`
**Compiler C++**:`sudo apt-get install g++`
The projects also needs **nlohmann/json** and **WebSocket++** libraries, but they were configured to be downloaded during the execution of cmake.

 5. Download [NodeJS](https://nodejs.org/) to run frontend application based on React + Vite
	 Then, go to EPTALAUNCHSERVER/frontend and run `npm install`, this will install everything inside package.json, making the frontend capable of running everything.
	
 6. Python needs to be installed as well to run all backend user management using Django, so run `sudo apt install python3` and `sudo apt install python3-pip` to make sure pip is installed.
	 Then go to EPTALAUNCHSERVER/backend and run `pip install -r requirements.txt` to install all python libs.

## How to use it?

 1. Go on your cellphone and start Sensor Server app, then copy the websocket ip that the cellphone will display to you after clicking on start.
 2. Prepare 3 ubuntu terminals to run all the 3 applications at the same time, the c++, python and JavaScript.
 3. Open IMUGPSBAR/src/main code and on "int main()" section, substitute the current URL `192.168.1.4:8080` with the new one generated by your sensor server app.
 4. Then, in **terminal ONE**, go to IMUGPSBAR, and run `rm -rf build`, then `mkdir build`, then `cd build` and finally `cmake ..` and then `make`
 5. In **terminal TWO**, go to EPTALAUNCHSERVER/backend and run `python3 manage.py runserver`
 6. In **terminal THREE**, go to EPTALAUNCHSERVER/frontend and run `npm run dev`, this will prompt a local URL, you can click it to go to the website responsible to show the "rocket" data streaming.
 7. Finally, to start streaming the data from your phone, go to **terminal ONE** and run `./sensor_monitoring`
 
 You should see the screen of the app being populated with the data streamed from the cellphone.

 
