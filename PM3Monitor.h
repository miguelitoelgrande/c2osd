/*
 *  Pm3.h
 *  PMSDKDemo
 *
 *  Created by tijmen on 17-11-12.
 *
 *  xcode c++ project ot make concept2 PM3 monitoring easy
 *  thanks to Andrew Lewis for the great .net example. a large part of
 *  his example could be used for this project.trainingData
 */

/*
 Copyright 2012 Tijmen van Gulik
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */ 

#ifndef PM3MONITOR_HEADER
#define PM3MONITOR_HEADER

#include <exception>
//#include <Carbon/Carbon.h>
//MM:
#include  <sys/types.h> 
#include <stdint.h>
//#include <cstdint>
//#include <inttypes.h>
typedef unsigned int uint;
typedef unsigned long UInt32;

#include <string>
#include "PM3DDICP.h"

using namespace std;



enum StrokePhase
{
	StrokePhase_Idle = 0,
	StrokePhase_Catch = 1,
	StrokePhase_Drive = 2,
	StrokePhase_Dwell = 3,
	StrokePhase_Recovery = 4
};

//convert the phase to a display text
const char* strokePhaseToString(StrokePhase strokePhase);

class PM3Exception:public exception
{ 
  public:
    PM3Exception(int a_errorCode,const string a_errorText);
    PM3Exception(const PM3Exception &e);
    ~PM3Exception() throw();
    const char* what() ;
    void remove();
    string errorText;
	//negative for PM3 concept2 errors positive numbers for PM3Monitor errors
    int errorCode;
};

const int CM_DATA_BUFFER_SIZE = 64;
const int MAX_PLOT_POINTS = 512; //todo find the real limit maybe a bit largen then needed

const int ERROR_INIT_DEVICE_NOT_FOUND = 1;
const int ERROR_POINT_BUFFER_OVERFLOW = 2;
const int ERROR_INIT_NOT_CALLED =3;

struct StrokeData 
{
	unsigned short int forcePlotPoints[MAX_PLOT_POINTS];
	unsigned short int forcePlotCount;
	uint dragFactor;
	uint workDistance;
	uint workTimehours;
	uint workTimeminutes;
	uint workTimeseconds; 
	double workTime;
	double splitMinutes; 
	double splitSeconds;
	uint power;
	double strokesPerMinuteAverage;
	uint strokesPerMinute;
	// MM: additional stuff:
	uint totCalories; // accumulated calories burned  CSAFE_GETCALORIES_CMD 
	uint calHr;  // calories/Hr derived from pace (GETPACE)
};

struct TrainingData 
{ 
	uint programNr; 
	uint hours; 
	uint minutes; 
	uint seconds;	
	uint distance;
	
};

class PM3Monitor;

//public Events, override the handler functions to get the data
class PM3MonitorHandler
{ 
public:
	// attach to this event for live drawing of the curve. (onStrokeDataUpdate gives only the curve at the end of the stroke 
	virtual void onIncrementalPowerCurveUpdate(PM3Monitor& monitor,unsigned short int forcePlotPoints[],unsigned short int a_beginIndex,unsigned short int a_endIndex) {};
	//called on catch,drive,dwell and recovery ( the catch is not  allways send )
	virtual void onNewStrokePhase(PM3Monitor& monitor,StrokePhase strokePhase) {} ;
	//strokeData contains info about the stroke. Send at the same time as the dwell
	virtual void onStrokeDataUpdate(PM3Monitor &monitor,StrokeData &strokeData) {};
	virtual void onTrainingDataChanged(PM3Monitor &monitor,TrainingData &trainingData) {};
	
};

class PM3Monitor
{ 
private:
	
    bool _initialized;
    UINT16_T _deviceCount;
	UINT16_T _deviceNumber;
	StrokeData _strokeData;
	
	StrokePhase _previousStrokePhase;
	StrokePhase _currentStrokePhase;
	
	UINT32_T _cmd_data[CM_DATA_BUFFER_SIZE];
	UINT16_T _cmd_data_size;
	UINT32_T _rsp_data[CM_DATA_BUFFER_SIZE];
	UINT16_T _rsp_data_size;
	
	PM3MonitorHandler* _handler;
	
	uint _nSPMReads;
	uint _nSPM;
	
	uint _distance; 
	
	TrainingData _trainingData;
	
    void initCommandBuffer();
	void addCSafeData(UInt32 data);
	void executeCSafeCommand(const char* description);
	
    virtual void highResolutionUpdate();
	void accumulateForceCurve();
	void lowResolutionUpdate();
	
	
protected:
	//events
	virtual void newStrokePhase(StrokePhase strokePhase);
	virtual void incrementalPowerCurveUpdate(unsigned short int forcePlotPoints[],unsigned short int a_beginIndex,unsigned short int a_endIndex);
	virtual void strokeDataUpdate(StrokeData &strokeData);
	virtual void trainingDataChanged();
	
	void setDeviceCount(UINT16_T value);
public:
	//this mst be called after the create
	unsigned short int initialize();
	
	//call start to connect to the device numer and set the event data which is returned for each event
	void start(unsigned short int a_deviceNumber,PM3MonitorHandler& handler);
	
	//resets the PM3, already called in the initialize at the first time
	void reset();
	
	//updates the performance data, call this about ever 80 mili seconds ( or faster)
	void update();
	void update2();

	//return nr of devices attached to the USB
    unsigned short int deviceCount();
	
	//return current device number a_deviceNumber 
	unsigned short int deviceNumber();
	
	void setDeviceNumber(unsigned short int value);
	
	PM3Monitor() : _initialized(false),_deviceCount(0),_deviceNumber(0),
	   _previousStrokePhase(StrokePhase_Idle),_currentStrokePhase(StrokePhase_Idle),
	_nSPMReads(0), _nSPM(0),_handler(NULL) {}
	
	
	void setHandler(PM3MonitorHandler& handler);
	PM3MonitorHandler* handler();
};

#endif //PM3MONITOR_HEADER