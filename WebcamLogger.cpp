/////////////////////////////////////////////////////////////////////////////
// $Id: WebcamLogger.cpp 5830 2018-11-29 19:09:18Z abelsten $
// Authors: adam.wilson@uc.edu and Alexander Belsten (belsten@neurotechcenter.org)
// Description: The WebcamLogger records webcam video to an AVI file
// and sets the frame number as a state value
//
// Parameterization:
//   Webcam logging is enabled from the command line adding
//     --LogWebcam=1
//   As a command line option.
//
// Event Variables:
//   WebcamFrame<n> - The current frame number for camera index n 
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2021: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
/////////////////////////////////////////////////////////////////////////////
#include "WebcamLogger.h"

#define NUM_OF_WEBCAM_EVENTS   4

#define PARM_CAMERAINDEX_IDX   0
#define PARM_WIDTH_IDX         1
#define PARM_HEIGHT_IDX        2
#define PARM_DECIMATION_IDX    3
#define PARM_DISPLAYSTREAM_IDX 4
#define PARM_FOURCC_IDX        5

Extension( WebcamLogger );

void PrintAvailableCameras (bool _useDirectShow);

WebcamLogger::WebcamLogger() :
	mWebcamEnable( false )
{
	
}

WebcamLogger::~WebcamLogger()
{
	Halt();
}

void WebcamLogger::Publish()
{
  /*
	bool webcamEnable = false;
	webcamEnable = ( (int)OptionalParameter("LogWebcam", 0) != 0 );

	if( !webcamEnable ) 
	{
		if (mpWebcamThread) delete mpWebcamThread;
		return;
	}
  */
  if (OptionalParameter ("LogWebcam", 0) == 0) return;

	BEGIN_PARAMETER_DEFINITIONS
		"Source:WebcamLogger int LogWebcam= 1 0 0 1"
			" // Allow logging from webcam (boolean)",

    "Source:WebcamLogger int UseDirectShow= 1 0 0 1"
      " // Use the DirectShow API (boolean)",

		"Source:WebcamLogger int DateTimeLocation= 0 0 0 4"
			" // Date/time text location in saved video: "
				" 0: none,"
				" 1: UpperRight,"
				" 2: UpperLeft,"
				" 3: LowerRight,"
				" 4: LowerLeft"
					" (enumeration)",

    "Source:WebcamLogger matrix Connections= "
      "{ CameraIndex Width Height Decimation DisplayStream FOURCC} " // row labels
      "{ Camera0 } "                                                 // column labels
      "0 "                                      // Camera Index
      "1920 "                                   // Width
      "1080 "                                   // Height
      "1 "                                      // Decimation
      "1 "                                      // Display Stream
      "H264 "                                   // FOURCC
	END_PARAMETER_DEFINITIONS

	// declare NUM_OF_WEBCAM_EVENTS event states
	for (int i = 0; i < NUM_OF_WEBCAM_EVENTS; i++) 
	{
		std::stringstream EventStrm;
		EventStrm << "WebcamFrame" << i << " 24 0 0 0";
		std::string EventStr = EventStrm.str();
		BEGIN_EVENT_DEFINITIONS
			EventStr.c_str(),
		END_EVENT_DEFINITIONS
	}
}

void WebcamLogger::AutoConfig ()
{
  if (OptionalParameter ("LogWebcam", 0) == 0) return;
}

void WebcamLogger::Preflight() const
{
	if (OptionalParameter("LogWebcam", 0) == 0) return;
  /*
	if (!(bool)Parameter("RecordAll"))
	{
    cv::VideoCapture capture = cv::VideoCapture ((int)Parameter ("RecordAll"));
		if (!capture.isOpened())
		{
			bcierr << "Cannot initialize webcam with index " << Parameter("CameraNumber") 
						 << ". Please make sure that the webcam is connected, and that the camera number is appropriate.";
			return;
		}
		capture.release();
	}
  Parameter ("RecordAll");
  Parameter ("SelectCodec");
  Parameter ("StartIndex");
  */
	Parameter ("DateTimeLocation");
  Parameter ("Connections");

	Parameter ("DataDirectory");
	Parameter ("SubjectName");
	Parameter ("SubjectSession");
	Parameter ("SubjectRun");

  PrintAvailableCameras (Parameter("UseDirectShow"));
  
  if (Parameter ("Connections")->NumRows () != 6)
  {
    bcierr << "WebcamLogger Error: There must be 5 rows in Connections parameter. "
           << "See https://www.bci2000.org/mediawiki/index.php/Contributions:WebcamLogger "
           << "for more info" << std::endl;
    return;
  }
  
  // check each connection for valid data
  for (int i = 0; i < Parameter("Connections")->NumColumns(); i ++)
  {
    // check for a valid index
    if ((int)Parameter ("Connections")(PARM_CAMERAINDEX_IDX, i) < 0)
      bcierr << "WebcamLogger Error: CameraIndex in Connections parameter must be greater than zero." << std::endl;

    // check for valid width
    if ((int)Parameter ("Connections")(PARM_WIDTH_IDX, i) < 1)
      bcierr << "WebcamLogger Error: Width in Connections parameter must be greater than zero." << std::endl;

    // check for valid height
    if ((int)Parameter ("Connections")(PARM_HEIGHT_IDX, i) < 1)
      bcierr << "WebcamLogger Error: Height in Connections parameter must be greater than zero." << std::endl;

    // check for valid decimation
    if ((int)Parameter ("Connections")(PARM_DECIMATION_IDX, i) < 0)
      bcierr << "WebcamLogger Error: Decimation in Connections parameter must be positive." << std::endl;

    // check for valid decimation
    int displystream = (int)Parameter ("Connections")(PARM_DISPLAYSTREAM_IDX, i);
    if (displystream != 0 && displystream != 1)
      bcierr << "WebcamLogger Error: DisplayStream in Connections parameter must be zero or one." << std::endl;

    // check for a valid fourcc length (we doen't know if it is a valid fourcc yet)
    std::string FOURCC = (std::string)Parameter ("Connections")(PARM_FOURCC_IDX, i);
    if (FOURCC.length () > 4)
    {
      bcierr << "WebcamLogger Error: FOURCC must have four characters or less";
    }
  }
}

void WebcamLogger::Initialize()
{
	mWebcamEnable = ( (int)OptionalParameter( "LogWebcam", 0) != 0 );
  if (!mWebcamEnable) return;
  /*
  this->Halt ();
	mCamNum        =  (int)Parameter("CameraNumber");
	mRecordAllCams = (bool)Parameter("RecordAll");
	mOpenWindow    = (bool)Parameter("DisplayStream");
	mCodecType     =  (int)Parameter("SelectCodec");
	mStartIndex    =  (int)Parameter("StartIndex");
	mEndIndex      =  (int)Parameter("EndIndex");
  mDecimation    =  (int)Parameter("Decimation");
  mTextLocation  =  (int)Parameter("DateTimeLocation");
	mStrFOURCC   = (std::string)Parameter("CodecFOURCC");
	mStrFOURCC.resize(4, ' ');
	std::transform(mStrFOURCC.begin(), mStrFOURCC.end(), mStrFOURCC.begin(), ::toupper);
		
	if (!mRecordAllCams)
	{
		// only recording from one camera

		if (mCamNum > NUM_OF_WEBCAM_EVENTS)
		{
			bcierr << "The max webcam index supported by this version of BCI2000 is " << NUM_OF_WEBCAM_EVENTS
							<< ". To record from more webcams or webcams of higher index, change NUM_OF_WEBCAM_EVENTS macro in WebcamLogger.cpp to the "
							<< "max index or number of webcams you want to record from and recompile.";
			return;
		}

		// start thread
		if (!mpWebcamThread)
		{
			mpWebcamThread = new WebcamThread(this, mCamNum);
		}
		else
		{
      this->Halt ();
      mpWebcamThread = new WebcamThread (this, mCamNum);
		}
	}
	else {
		//record from all cameras on system
		if (mEndIndex > NUM_OF_WEBCAM_EVENTS)
		{
			bcierr << "The max number of webcams supported by this version of BCI2000 is " << NUM_OF_WEBCAM_EVENTS 
					    << ". To record from more webcams, change NUM_OF_WEBCAM_EVENTS macro in WebcamLogger.cpp to the "
							<< "number of webcams you want to record from and recompile.";
			return;
		}

		if (mStartIndex >= mEndIndex)
		{
			bcierr << "Start index " << mStartIndex << " is greater than or equal to end index " 
							<< mEndIndex << ". No cameras will be connected";
			return;
		}

		// close connections with other cameras
		this->Halt();

		// try to connect to specified cameras
		for (int i = mStartIndex; i < mEndIndex+1; i++)
		{
			WebcamThread* temp = new WebcamThread(this, i);
			if (temp->Connected())
			{
				mWebcamThreads.push_back(temp);
			}
			else
			{
				temp->StopThread();
				delete temp;
				temp = NULL;
			}
		}
		bciout << "Connected to " << mWebcamThreads.size() << " Camera(s)";
	}
  */
  
  // disconnect and deallocate any old threads
  Halt ();

  // make new threads
  for (int i = 0; i < Parameter ("Connections")->NumColumns (); i++)
  {
    WebcamThread* temp_camera = new WebcamThread (
      Parameter ("Connections")(PARM_CAMERAINDEX_IDX,   i),
      Parameter ("Connections")(PARM_WIDTH_IDX,         i),
      Parameter ("Connections")(PARM_HEIGHT_IDX,        i),
      Parameter ("Connections")(PARM_DECIMATION_IDX,    i),
      Parameter ("Connections")(PARM_DISPLAYSTREAM_IDX, i),
      Parameter ("DateTimeLocation"                      ),
      Parameter ("UseDirectShow"                         ),
      Parameter ("Connections")(PARM_FOURCC_IDX,        i)
    );

    bool connected = temp_camera->Initalize ();
    if (connected)
    {
      mWebcamThreads.push_back (temp_camera);
    }
  }
}


void WebcamLogger::StartRun()
{
  if (!mWebcamEnable) return;
	/*
	// update output file name
	stdLLstring output = CurrentRun();
  mOutputFile = FileUtils::ExtractDirectory( mOutputFile ) + FileUtils::ExtractBase( mOutputFile );
		
	// start recording from camera(s)
	if (mRecordAllCams)
	{
		for (int i = 0; i < mWebcamThreads.size(); i++)
		{
			mWebcamThreads[i]->StartRecording();
		}
	}
	else
	{			
		mpWebcamThread->StartRecording();
	}
	mRecord = true;
	*/
  std::string output_file_prefix = CurrentRun ();
  output_file_prefix = FileUtils::ExtractDirectory (output_file_prefix) + FileUtils::ExtractBase (output_file_prefix);
  for (int i = 0; i < mWebcamThreads.size (); i++)
  {
    mWebcamThreads[i]->StartRecording (output_file_prefix);
  }
}

void WebcamLogger::StopRun()
{
  /*
	if (mWebcamEnable)
	{
		mRecord = false;

		// stop recording from camera(s)
		if (mRecordAllCams)
		{
			for (int i = 0; i < mWebcamThreads.size(); i++)
			{
				mWebcamThreads[i]->StopRecording();
			}
		}
		else
		{
			mpWebcamThread->StopRecording();
		}
	}
  */
  for (int i = 0; i < mWebcamThreads.size (); i++)
  {
    mWebcamThreads[i]->StopRecording ();
  }
}

void WebcamLogger::Halt()
{
  /*
	// stop all camera threads and delete threads 
	if (mRecordAllCams)
	{
		for (int i = 0; i < mWebcamThreads.size(); i++)
		{
			mWebcamThreads[i]->StopThread();
			delete mWebcamThreads[i];
			mWebcamThreads[i] = NULL;
		}
		mWebcamThreads.clear();
	}
	else
	{
		if (mpWebcamThread)
		{
			mpWebcamThread->StopThread();
			delete mpWebcamThread;
			mpWebcamThread = NULL;
		}
	}
  */
  if (mWebcamThreads.size () != 0)
  {
    for (int i = 0; i < mWebcamThreads.size (); i++)
    {
      mWebcamThreads[i]->StopStream ();
      delete mWebcamThreads[i];
      mWebcamThreads[i] = NULL;
    }
    mWebcamThreads.clear ();
  }
}
 