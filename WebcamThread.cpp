/////////////////////////////////////////////////////////////////////////////
// $Id: WebcamLogger.cpp 5817 2018-11-08 16:56:17Z mellinger $
// Authors: adam.wilson@uc.edu & Alexander Belsten belsten@neurotechcenter.org
//
// Description: The WebcamThread records webcam video to an AVI file
// and sets the frame number as a state value
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

#include "WebcamThread.h"

#define OPENCV_API cv::CAP_DSHOW

static time_t Now()
{ return ::time( 0 ); }

static std::string TimeToString( time_t inTime )
{ // format is "MM/dd/yy hh:mm:ss"
  struct ::tm t = { 0 },
             *p = ::localtime( &inTime );
  if( !p )
    return "<n/a>";

  t = *p;
  std::ostringstream oss;
  oss << std::setfill( '0' )
      << std::setw( 2 ) << t.tm_mon + 1 << '/'
      << std::setw( 2 ) << t.tm_mday << '/'
      << std::setw( 2 ) << t.tm_year % 100 << ' '
      << std::setw( 2 ) << t.tm_hour << ':'
      << std::setw( 2 ) << t.tm_min << ':'
      << std::setw( 2 ) << t.tm_sec;
  return oss.str();
}

// Notify the user what cameras are connected to the system and what index they are at.
// This is useful becuase they won't have to search for valid indexes through trial and
// error, but they won't be able to discern one camera from another. They'll have to figure
// that part out through trial and error. I see this as an OpenCV problem.
void PrintAvailableCameras (bool _useDirectShow)
{
  bciout << cv::getBuildInformation () << std::endl;


  int max_n_cameras = 10;
  int total_cameras = 0;
  std::stringstream camera_info_stream;
  camera_info_stream << "WebcamLogger: Enumeration of cameras connected to system:\n";
  for (int i = 0; i < max_n_cameras; i++)
  {
    cv::VideoCapture temp_camera;
    if (_useDirectShow) temp_camera.open (i, OPENCV_API);
    else                temp_camera.open (i);

    if (temp_camera.isOpened ())
    {
      camera_info_stream << "  Camera detected at index " << i << "\n";
      temp_camera.release ();
      total_cameras += 1;
    }
  }
  if (total_cameras == 0)
    camera_info_stream << "  No cameras were detected\n";
  bciout << camera_info_stream.str () << std::endl;
}

//==========================================================================================
// WebcamThread member function implementaion
//==========================================================================================
WebcamThread::WebcamThread ( int         _camIndex,
                             int         _width, 
                             int         _height, 
                             int         _decimation, 
                             bool        _displayStream, 
                             int         _dateLocation,
                             bool        _useDirectShow,
                             std::string _fourcc ):
  mCameraIndex    (_camIndex),
	mSourceWidth    (_width),
	mSourceHeight   (_height),
  mDecimation     (_decimation),
  mDisplayStream  (_displayStream),
  mDateLocation   (_dateLocation),
  mUseDirectShow  (_useDirectShow),
  mRecording      (false),
  mAddDate        (false),
  mFrameNum       (0),
  mCount          (0),
  mTargetFps      (30),
  mWinName        (""){
  _fourcc.resize  (4, ' ');
  std::transform  (_fourcc.begin (), _fourcc.end (), _fourcc.begin (), ::toupper);
  mFourcc         = cv::VideoWriter::fourcc (_fourcc[0], _fourcc[1], _fourcc[2], _fourcc[3]);
}


bool WebcamThread::Initalize()
{
	// start mutex so existing thread doesn't attempt to use camera while initalizing and getting FPS
	mMutex.Acquire();

	// open the webcam
  if (mUseDirectShow) mVCapture.open (mCameraIndex, OPENCV_API);
  else                mVCapture.open (mCameraIndex);

  if (!mVCapture.isOpened ())
  {
    mMutex.Release ();
    return false;
  }

  mVCapture.set (cv::CAP_PROP_FOURCC,       mFourcc);
  mVCapture.set (cv::CAP_PROP_FRAME_WIDTH,  mSourceWidth);
  mVCapture.set (cv::CAP_PROP_FRAME_HEIGHT, mSourceHeight);

  int actualFOURCC = mVCapture.get (cv::CAP_PROP_FOURCC);
  int actualWidth  = mVCapture.get (cv::CAP_PROP_FRAME_WIDTH);
  int actualHeight = mVCapture.get (cv::CAP_PROP_FRAME_HEIGHT);

  if (actualFOURCC != mFourcc)
  {
    bciwarn << "WebcamLogger: Invalid FOURCC for camera at index " << mCameraIndex << std::endl;
    mFourcc = actualFOURCC;
  }

  // Heights and widths are not guaranteed - report to the user if their requested size
  //   isn't used
  if (actualWidth != mSourceWidth)
  {
    bciwarn << "WebcamLogger: Requested width for camera " << mCameraIndex 
            << " not valid. Using " << actualWidth << " instead." << std::endl;
    mSourceWidth = actualWidth;
  }
  if (actualHeight != mSourceHeight)
  {
    bciwarn << "WebcamLogger: Requested height for camera " << mCameraIndex
            << " not valid. Using " << actualHeight << " instead." << std::endl;
    mSourceHeight = actualHeight;
  }

	// get FPS over 60 frames
	int nFrames = 60;
	PrecisionTime t1 = PrecisionTime::Now();
	for (int i = 0; i < nFrames; i++)
	{
		cv::Mat Frame;
		mVCapture >> Frame;
	}
	PrecisionTime t2 = PrecisionTime::UnsignedDiff(PrecisionTime::Now(), t1);
	int fps = 1000.0f / (float(t2) / nFrames);
	mTargetFps = fps / mDecimation;
	bciout << "Camera " << mCameraIndex << " Target FPS: " << mTargetFps;

	mMutex.Release();
		
	mWinName = "Camera " + std::to_string(mCameraIndex);


  this->InitalizeText();

  // everthing has been successful up to this point, so we can start the thread
  this->Start ();

	return true;
}

WebcamThread::~WebcamThread()
{
	cv::destroyWindow (mWinName);
  StopStream ();
}

void WebcamThread::StartRecording(std::string _outputFile)
{
  this->StartIfNotRunning ();

	// open video recorder
	std::string outputFileName = _outputFile + "_" + std::to_string(mCameraIndex) + "_vid.mp4";

	mVideoWriter.open(outputFileName, mFourcc, mTargetFps, cv::Size(mSourceWidth, mSourceHeight), true);

	if (!mVideoWriter.isOpened())
	{
		bciwarn << "WebcamLogger Error: Could not open file for recording camera " << mCameraIndex << " video." 
						<< " Trying a different FOURCC codec may resolve this issue";
		return;
	}

	bciout << "Started Recording Camera " << mCameraIndex;

	mFrameNum  = 0;
	mCount     = 0;
  mRecording = true;
}

void WebcamThread::StopRecording()
{
  mRecording = false;
	if (mVideoWriter.isOpened())
	{
		bciout << "Stopped Recording Camera " << mCameraIndex;
	}
	mVideoWriter.release();
}

void WebcamThread::InitalizeText()
{
	std::string currentTime = TimeToString(Now());
	int baseline;

	// get the size of bounding box of text
	cv::Size textSize = cv::getTextSize(currentTime, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

	// find appropriate point to put the text
  mAddDate = true;
	switch (mDateLocation) {
	case 1: // Upper Right
		mDatePoint = cv::Point(mSourceWidth - 10 - textSize.width, 10 + textSize.height);
		break;
	case 2: // Upper Left
    mDatePoint = cv::Point(10, 10 + textSize.height);
		break;
	case 3: // Lower Right
    mDatePoint = cv::Point(mSourceWidth - 10 - textSize.width, mSourceHeight - 10 - baseline);
		break;
	case 4: // Lower Left
    mDatePoint = cv::Point(10, mSourceHeight - 10 - baseline);
		break;
	default:
    mAddDate = false;
    mDatePoint = cv::Point(10, 10);
		break;
	}
}

void WebcamThread::GetFrame()
{
	mCount++;
	if ((mCount % mDecimation) == 0)
	{
		// get new frame
		cv::Mat Frame;
		mVCapture >> Frame;

		if (mAddDate)
		{
			// add text to the image
			cv::putText(Frame, TimeToString(Now()), mDatePoint, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
		}

		if (mDisplayStream)
		{
			// display image to window
			cv::imshow(mWinName, Frame);
			cv::waitKey(5);
		}
	
		if (mRecording)
		{
			// write image to file
			mVideoWriter << Frame;
			bcievent << "WebcamFrame" + std::to_string(mCameraIndex) + " " << ++mFrameNum;
		}
	}
	else
	{
		if (mRecording)
			bcievent << "WebcamFrame" + std::to_string(mCameraIndex) + " " << 0;
	}
}

int WebcamThread::OnExecute()
{
	mFrameNum = 0;
	bciout << "Camera " << mCameraIndex << " thread started";
	while (!this->Terminating())
	{
		if (mVCapture.isOpened())
			this->GetFrame();
	}
	bciout << "Camera " << mCameraIndex << " thread ended";
	return 0;
}

void WebcamThread::StopStream ()
{ 
  this->TerminateAndWait ();
  if (mVCapture.isOpened())
    mVCapture.release ();
}
