#include "Custom_Piezo.h"
#include <Wire.h>
#include <SPI.h>
#include "SparkFunLSM9DS1.h"
#include "Custom_Piezo.h"
#include "TinyGPS++.h"															  

LSM9DS1 imu;
TinyGPSPlus gps;

//Setting up the address of the gyroscope
#define LSM9DS1_M    0x1C  //LOW: 0x1E if high
#define LSM9DS1_AG   0x6A  //LOW: 0x6B if high

#define SEND_TO_PI
//#define PRINT_TO_CONSOLE

#define PRINT_PAUSE 20					//Data output pause time (millis)
#define DECLINATION -3.7166				//Declination (degrees) Palatine, IL
#define SERIAL_OUT_BAUD  128000
#define GPS_BAUD 9600
#define PIEZO_PIN 6
#define DATA_START	"|S|"
#define DATA_END "|E|"
#define DATA_SEP "|"
#define RPI_SIGNAL 't'					//Arduino calibrated, 'd' for display in rpi, 't' for writing to text file in rpi

static unsigned long lastPrint = 0;
static unsigned long systemStartTime;
static unsigned short int cycleTime;
static double pitchOffset;
static double rollOffset;

static float pitchRadians;
static float rollRadians;
static float yawRadians = 0;
static float headingRadians;
static double location[2];
static double groundSpeed;
static long currentSystemTime;
static int satellitesConnected;

void setupIMU(){
	imu.settings.device.commInterface = IMU_MODE_I2C;
	imu.settings.device.mAddress = LSM9DS1_M;
	imu.settings.device.agAddress = LSM9DS1_AG;
			
	while(!imu.begin()){
		Serial.println("Failure to establish connection to w/Gyro");
	}
}

void setupGPS(){
	Serial1.begin(GPS_BAUD);
	
	#ifdef PRINT_TO_CONSOLE
		Serial.print(F("Waiting on GPS Connection"));
	#endif
	while(millis() < 5000 && gps.charsProcessed() < 10){
		gps.encode(Serial1.read());
		#ifdef PRINT_TO_CONSOLE
			Serial.print(F("."));
		#endif
	}if(gps.charsProcessed() < 10){
		Serial.println(F("GPS Connection Failure"));
		Serial1.end();
	}else{
		Serial.println();
	}
}

void setupOffsets(){
	for(int i = 0; i < 2; i++){
		playWaiting(PIEZO_PIN, 750);
	}
	runCycle();
	rollOffset = getRoll(imu.ax, imu.ay, imu.az);
	pitchOffset = getPitch(imu.ay, imu.az);
}

void setup(){
	Serial.begin(SERIAL_OUT_BAUD);
	pinMode(PIEZO_PIN, OUTPUT);	

	setupIMU();
	setupGPS();
	setupOffsets();
	
	playStartUpMelody(PIEZO_PIN);
	#ifdef SEND_TO_PI
		Serial.println(RPI_SIGNAL);
	#endif
	systemStartTime = millis();
}

//Cycles take around 15millis for gather & print raw
void loop(){
	cycleTime = 0;
	unsigned long cycleStart = millis();
	
	runCycle();
	assignToVariables();
	if((lastPrint + PRINT_PAUSE) < millis()){
		lastPrint = millis();
		#ifdef PRINT_TO_CONSOLE
			printCycleGyro();
			printCycleGPS();
			//Serial.print(F("cycleTime: ")); 
			//Serial.println(millis() - cycleStart);
			Serial.println();
		#elif defined SEND_TO_PI
			sendPacket(); 
		#endif
	}
}

//Function to gather data
void runCycle(){
	if(imu.gyroAvailable()){
		imu.readGyro();
	}
		
	if(imu.accelAvailable()){
		imu.readAccel();
	}
		
	if(imu.magAvailable()){
		imu.readMag();
	}
	
	byte gpsData;
	while(Serial1.available() > 0)
		gps.encode(Serial1.read());

	assignToVariables();		
}

void assignToVariables(){
	if(gps.location.isValid()){
		location[0] = gps.location.lat();
		location[1] = gps.location.lng();
	}

	if(gps.satellites.isValid()){
		satellitesConnected = gps.satellites.value();
	}

	if(gps.speed.isValid()){
		groundSpeed = gps.speed.mph();
	}
	
	pitchRadians = getPitch(imu.ay, imu.az) - pitchOffset;
	if (pitchRadians > PI) pitchRadians -= (2 * PI);
	else if (pitchRadians < -PI) pitchRadians += (2 * PI);	

	rollRadians = getRoll(imu.ax, imu.ay, imu.az) - rollOffset;
	if (rollRadians > PI) rollRadians -= (2 * PI);
	else if (rollRadians < -PI) rollRadians += (2 * PI);

	headingRadians = getHeadingRadians(-imu.my, -imu.mx);
	currentSystemTime = millis();
}

void sendPacket(){
	String controlSurfaceFiller = "0";
	
	Serial.print(DATA_START);
	
	Serial.print(systemStartTime);
	Serial.print(DATA_SEP);
	Serial.print(currentSystemTime);
	Serial.print(DATA_SEP);
	Serial.print(rollRadians);
	Serial.print(DATA_SEP);
	Serial.print(pitchRadians);
	Serial.print(DATA_SEP);
	Serial.print(yawRadians);
	Serial.print(DATA_SEP);
	Serial.print(location[1], 6);
	Serial.print(DATA_SEP);
	Serial.print(location[0], 6);
	Serial.print(DATA_SEP);
	Serial.print(headingRadians);
	Serial.print(DATA_SEP);
	Serial.print(groundSpeed);
	Serial.print(DATA_SEP);
	Serial.print(satellitesConnected);
	
	//To fill in for eventual input received from the RC receiver
	for(int i = 0; i < 4; i++){
		Serial.print(DATA_SEP);
		Serial.print(controlSurfaceFiller);
	}

	Serial.println(DATA_END);
}

float getPitch(float ay, float az){
	return atan2(ay, az);
}

float getRoll(float ax, float ay, float az){
	return atan2(-ax, sqrt(ay * ay + az * az));
}

//Figure out what the heck is wrong with this heading calculation, does not work
float getHeadingRadians(float mx, float my){
	float heading;
	if (my == 0)
		heading = (mx < 0) ? PI : 0;
	else
		heading = atan2(mx, my);
	
	heading -= DECLINATION * PI / 180;
	if (heading > PI) 
		heading -= (2 * PI);
	else if (heading < -PI) 
		heading += (2 * PI);
	return heading;
}