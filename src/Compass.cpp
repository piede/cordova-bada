/*
 * Compass.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: Anis Kadri
 */

#include "../inc/Compass.h"

Compass::Compass(Web* pWeb) : PhoneGapCommand(pWeb) {
}

Compass::~Compass() {
	__sensorMgr.Construct();
	started = false;
	x = y = z = 0.0;
	timestamp = 0;
}

void
Compass::Run(const String& command) {
	if (!command.IsEmpty()) {

		String args;
		String delim(L"/");
		command.SubString(String(L"gap://").GetLength(), args);
		StringTokenizer strTok(args, delim);
		String method;
		strTok.GetNextToken(method);
		// Getting callbackId
		for(int i = 0 ; i < 2 && strTok.HasMoreTokens() ; i++, strTok.GetNextToken(callbackId));
		AppLogDebug("Method %S, callbackId: %S", method.GetPointer(), callbackId.GetPointer());
		// used to determine callback ID
		if(method == L"com.phonegap.Compass.watchHeading" && !callbackId.IsEmpty() && !IsStarted()) {
			AppLogDebug("watching compass...");
			StartSensor();
		}
		if(method == L"com.phonegap.Compass.clearWatch" && !callbackId.IsEmpty() && IsStarted()) {
			AppLogDebug("stop watching compass...");
			StopSensor();
		}
		if(method == L"com.phonegap.Compass.getCurrentHeading" && !callbackId.IsEmpty() && !IsStarted()) {
			AppLogDebug("getting current compass...");
			GetLastAcceleration();
		}
		AppLogDebug("Compass command %S completed", command.GetPointer());
	} else {
		AppLogDebug("Can't run empty command");
	}
}

bool
Compass::StartSensor(void) {
	result r = E_SUCCESS;

	if(__sensorMgr.IsAvailable(SENSOR_TYPE_MAGNETIC)) {
		r = __sensorMgr.AddSensorListener(*this, SENSOR_TYPE_MAGNETIC, 50, true);
		if(IsFailed(r)) {
			return false;
		}
	} else {
		AppLogException("Compass sensor is not available");
		String res;
		res.Format(256, L"PhoneGap.callbacks['%S'].fail({message:'Acceleration sensor is not available',code:'001'});", callbackId.GetPointer());
		pWeb->EvaluateJavascriptN(res);
		return false;
	}
	started = true;
	AppLogDebug("Start Watching Sensor");
	return true;
}

bool
Compass::StopSensor(void) {
	result r = E_SUCCESS;

	r = __sensorMgr.RemoveSensorListener(*this, SENSOR_TYPE_MAGNETIC);
	if(IsFailed(r)) {
		return false;
	}
	started = false;
	AppLogDebug("Stopped Watching Sensor");
	return true;
}

bool
Compass::IsStarted() {
	return started;
}

void
Compass::GetLastAcceleration() {
	String res;
	res.Format(256, L"PhoneGap.callbacks['%S']({x:%f,y:%f,z:%f,timestamp:%d});", callbackId.GetPointer(), x, y, z, timestamp);
	pWeb->EvaluateJavascriptN(res);
}

void
Accelerometer::OnDataReceived(SensorType sensorType, SensorData& sensorData, result r) {

	sensorData.GetValue((SensorDataKey)MAGNETIC_DATA_KEY_TIMESTAMP, timestamp);
	sensorData.GetValue((SensorDataKey)MAGNETIC_DATA_KEY_X, x);
	sensorData.GetValue((SensorDataKey)MAGNETIC_DATA_KEY_Y, y);
	sensorData.GetValue((SensorDataKey)MAGNETIC_DATA_KEY_Z, z);

	AppLogDebug("x: %f, y: %f, z: %f timestamp: %d", x, y, z, timestamp);

	String res;
	res.Format(256, L"PhoneGap.callbacks['%S']({x:%f,y:%f,z:%f,timestamp:%d});", callbackId.GetPointer(), x, y, z, timestamp);
	AppLogDebug("%S", res.GetPointer());
	pWeb->EvaluateJavascriptN(res);
}
