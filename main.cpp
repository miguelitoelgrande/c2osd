// This little "Hello World" program uses the Concept2 SDK dll's to
// retrieve performance data from the Indoor Rowers Performance Monitor via USB
// this data will be displayed in the top screen corner as green OSD overlay (a transparent Window)
// it will also dump this data to the console (if "console" instantiated below) 
// use together with a Video player of your choice (note: "Always on top" in the Player's preferences may interfer with this window)
// Pay attention to the Task Bar Icon. Sometimes toggeling "hide OSD, show OSD" may help bringing the OSD on top of all other Windows again.

// Tested with Windows 7 and Concept2 Modell D, PM5
// should also support PM3 and PM4 connected via USB
// Should compile with "Microsoft Visual Studio Express 2012 for Windows Desktop"

// Credits:
// Concept2 SDK for the DLLs to talk to the Performance Monitor via USB
// "tijmenvangulik" for providing the PM3Monitor wrapper at http://webergometer.codeplex.com/
// OSD code inspired by: http://codes-sources.commentcamarche.net/source/view/38898/1062876#browser 

// To get this working in VisualStudio Express 2012:
// - General: No Unicode -> otherwise string constants below may generate errors where used as params...
// - Linker:  (/SUBSYSTEM:WINDOWS)

#include <windows.h>

#include "PM3Monitor.h"

// for additional console:
#include "console.h"
#include <iostream>

using namespace std;

// MM: uncomment for extra console window showing all cout's and cerr's
// console MyConsole;  // Well, this is not a VC "console" application -> get cout/cerr as usual..

// Menu items:
#define ID_SHOWOSD 1001
#define ID_HIDEOSD  1002
#define ID_RESET 1003
#define ID_QUIT  1004
// required below.
#define WM_SHELLNOTIFY WM_APP +1
#define ID_TRAY 1000


// Area for the OSD
int osd_width = 850; //550;
int osd_height = 200;
int screen_width=GetSystemMetrics(SM_CXSCREEN);
int screen_height=GetSystemMetrics(SM_CYSCREEN);
int osd_x = 10; // (screen_width-osd_width)/2 ;  //30;
int osd_y = 5; // (screen_height-osd_height)/2; // 50;
#define OSD_FONTSIZE 65
char osdTxt[1024];


class HandlePM3Data : public PM3MonitorHandler {
public:
	// attach to this event for live drawing of the curve. (onStrokeDataUpdate gives only the curve at the end of the stroke 
	void onIncrementalPowerCurveUpdate(PM3Monitor &monitor,unsigned short int a_value,unsigned short int a_index)
	{ 
		//no code yet
	}
	// called on catch,drive,dwell and recovery ( the catch is not  always send )
	void onNewStrokePhase(PM3Monitor &monitor,StrokePhase strokePhase)
	{   
		std::cerr << " new stroke phase: " <<  strokePhaseToString(strokePhase) << "\n";  	
	}
	
	void onTrainingDataChanged(PM3Monitor &monitor,TrainingData trainingData)
	{
		std::cout << " onTrainingDataChanged() \n";
		std::cout << " ProgramNr: " << trainingData.programNr  << "\n";  	
		std::cout << " Training Time: " <<  trainingData.hours << ":" <<  trainingData.minutes << ":" <<  trainingData.seconds << "\n";
		std::cout << " Distance: " << trainingData.distance  << "\n";  	
	}

	//strokeData contains info about the stroke. Send at the same time as the dwell
	void onStrokeDataUpdate(PM3Monitor &monitor,StrokeData &strokeData)
	{   
		cout << "----------------------------------------------------\n";
/*
		cout << "curve :"<< strokeData.forcePlotCount<<" \n";
		for(int i=0;i<strokeData.forcePlotCount;i++)
		{   cout << strokeData.forcePlotPoints[i];
			for(int i2=0;i2<strokeData.forcePlotPoints[i];i2+=5) 
			{ cout << "*";
			}
			cout << "\n";
		}
*/
	//	sprintf(osdTxt, "Time:  %02d:%02d:%02d\nDist:  %4d m\nPower:  %d Watt\nSplit:  %02.0f:%02.0f\nStokes/min:  %d\nAvg.Str/min:  %02.1f"
		sprintf(osdTxt, "%02d:%02d:%02d   %4d m\n%d W   Split: %02.0f:%02.0f\n%d s/m (avg %02.1f)"
			   , strokeData.workTimehours , strokeData.workTimeminutes, strokeData.workTimeseconds  
			   , strokeData.workDistance 		
			   , strokeData.power 
			   , strokeData.splitMinutes , strokeData.splitSeconds			
			   , strokeData.strokesPerMinute 
			   , strokeData.strokesPerMinuteAverage 		  
			);

		cout << "Drag factor: " << strokeData.dragFactor << "\n";
		cout << "Work distance: " <<  strokeData.workDistance << "\n";
		cout << "Work time " <<  strokeData.workTimehours << ":" 
		<< strokeData.workTimeminutes<< ":" 
		<< strokeData.workTimeseconds << "\n"; 
		cout << "Work time: " <<  strokeData.workTime << "\n";
		cout << "Split: " <<  strokeData.splitMinutes << ":" 
		<< strokeData.splitSeconds << "\n";
		cout << "Power: " <<  strokeData.power << "\n";
		cout << "Strokes Per Minute Average: " <<  strokeData.strokesPerMinuteAverage << "\n";
		cout << "Strokes Per Minute: " <<  strokeData.strokesPerMinute << "\n";
		cout << "\n";
		
	}	
};


PM3Monitor monitor = PM3Monitor();		
//object for the events which prints the performance data
HandlePM3Data handler; 
int attempt = 0;

// attempt to (re)connect to PM, initially, but also e.g. if USB was removed, etc.
void connectPM() {
	try {	
		//initializes the monitor, returns the nr of devices, but ignore
        monitor.initialize();  
		// well, just using the first found PM device (0), if you have more Indoor Rowers 
		// connected to the PC, you might want to add some selection code.
		
		// // handler which handles the events 		
	    monitor.start(0, handler ); // add handler to consume data and populate the OSD accordingly
		// subsequently, poll for updates via "monitor.update()" -> see timer code...
	}
	catch ( PM3Exception& e) {
		cerr << e.errorText;
		sprintf(osdTxt, "%s\n",e.errorText);
		//return e.errorCode;
	} 	
}

// reset the PM: Not sure if really useful -> Just restart App and/or USB connection in the worst case :-)
void resetPM() {
	try {			
        monitor.reset();
	}
	catch ( PM3Exception& e) {    
		cerr << e.errorText;
		sprintf(osdTxt, "%s\n",e.errorText);
	} 	
}

///////////// Defines the transparent OSD Window //////////////
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam)
{
	static 	HMENU hPopupMenu ;
	static	HFONT hFont;
	static 	NOTIFYICONDATA ndata; 

	switch (msg)
	{
	case WM_CREATE:
		// Arial Font, bold in size defined by Macro above
		hFont = CreateFont(OSD_FONTSIZE, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
		// Init context Menu (of TrayIcon)
		hPopupMenu = CreatePopupMenu();
		AppendMenu(hPopupMenu, MF_STRING, ID_SHOWOSD, "Show OSD");
		AppendMenu(hPopupMenu, MF_STRING, ID_HIDEOSD, "Hide OSD");
		//AppendMenu(hPopupMenu, MF_STRING, ID_RESET, "Reset PM");
		AppendMenu(hPopupMenu, MF_STRING, ID_QUIT, "Quit C2osd");

		// Initialization of NOTIFYDATA structure:
		ndata.cbSize = sizeof(NOTIFYICONDATA);
		ndata.hWnd = hwnd;
		ndata.uID = ID_TRAY;
		ndata.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		ndata.uCallbackMessage = WM_SHELLNOTIFY;
		ndata.hIcon = LoadIcon(GetModuleHandle(0),"IDI_ICON");
		lstrcpy(&ndata.szTip[0], "Concept2 OSD");
		////Tell system to add the TrayIcon to the taskbar status area
		Shell_NotifyIcon(NIM_ADD, &ndata);		
		SetTimer(hwnd,1,1500,0);  // create timer to trigger OSD updates every 1.5 seconds.
		return 0;	
	case WM_PAINT: 
		PAINTSTRUCT ps;
		HDC hdc; // the context		
		hdc=BeginPaint(hwnd,&ps);	
		SetTextColor(hdc, RGB(10, 250, 10));		
		SelectObject(hdc,hFont);		
		RECT rc; // Set the rectangular area/canvas for the operation 
		SetRect(&rc,0,0,osd_width,osd_height);
		// Draw whatever is in the osdTxt string buffer.
		DrawTextEx(hdc,osdTxt,-1,&rc,DT_LEFT|DT_WORDBREAK,0); // Wordbreak for error messages only ;-)		
		EndPaint(hwnd,&ps); // indicates we are finished painting the window
		return 0;
	case WM_TIMER: // called periodically via defined timer, so retrieve data from PM and prepare data for OSD update:					
		{   
			try {			
				//monitor.update(); // turned out to only update with stroke events...
				monitor.update2(); // calling formerly internal "lowResolutionUpdate()"
			}
			catch ( PM3Exception& e) {			
				cerr << e.errorText;
				sprintf(osdTxt, "%s\n",e.errorText);
				// try to reconnect after five failed updates...
				if ((attempt++) > 5) { connectPM(); resetPM(); attempt = 0; }  
				//return e.errorCode;
			} 	
			// As long as the window is visible, force redraw by invalidating area:
			if(IsWindowVisible(hwnd)) InvalidateRect(hwnd,0,1);
			return 0;
		}
	case WM_COMMAND:
		if (lParam == 0) {
			switch(LOWORD (wParam)) {			
				case ID_SHOWOSD:  // Show OSD display:
					ShowWindow(hwnd, 1);
					return 0;
				case ID_HIDEOSD: // Hide OSD display:
					ShowWindow(hwnd, 0);
					return 0;
				case ID_RESET: // try resetting PM
					resetPM();
					return 0;
				case ID_QUIT:					
					SendMessage(hwnd,WM_CLOSE,0,0); // Send "close" to Window
					return 0;
				default:
					break;
			}
		}
		return 0;
	case WM_SHELLNOTIFY: // Checking for right-click of the OSD TrayIcon:	
		if (wParam == ID_TRAY && lParam == WM_RBUTTONUP) {		
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hPopupMenu, TPM_RIGHTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, 0);
		}
		return 0;
	case WM_CLOSE:	// Close Window/Exit OSD application	
		KillTimer(hwnd,1); // Disable timer		
		Shell_NotifyIcon(NIM_DELETE, &ndata);  // Remove tray icon
		// Cleaning up...
		DeleteObject(hFont); 
		DestroyMenu(hPopupMenu); 
		DestroyWindow(hwnd); 
		break;
	case WM_DESTROY:		
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

/////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd,int show)
{
	sprintf(osdTxt, "Waiting...");

	// Init Win32 structures for the OSD "window":
	WNDCLASSEX wc;
	memset(&wc,0,sizeof(wc));
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance=hInst;
	wc.lpfnWndProc=WndProc;
	wc.hCursor=LoadCursor(0,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName="c2osd";
	RegisterClassEx(&wc);

	// Toolwindow (without a title, etc.)
	HWND hWnd=CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_LAYERED  | WS_EX_TOOLWINDOW | WS_EX_TOPMOST  , "c2osd", 0,  WS_POPUP, osd_x,osd_y, osd_width, osd_height, 0, 0, hInst, 0);
	// Set white color as the transparent one:
	SetLayeredWindowAttributes(hWnd, RGB(255,255,255), 0, LWA_COLORKEY);
	// MM: show OSD at the beginning...
	ShowWindow(hWnd, 1);

	//////////////////////////////
	connectPM();
	
	////////////////////////////
	// The main message loop to handle Windows "events":
	MSG msg;
	while(GetMessage(&msg,0,0,0)) {
		TranslateMessage(&msg); // Translates virtual-key messages into character messages, etc.
		DispatchMessage(&msg); // Dispatches a message to a window procedure.
	}	
	return 0;
}

