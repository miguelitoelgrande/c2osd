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
