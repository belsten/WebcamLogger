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
#ifndef WEBCAMTHREAD_H
#define WEBCAMTHREAD_H

#include <opencv2/opencv.hpp>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "WebcamLogger.h"
#include "Thread.h"
#include "Mutex.h"
#include "PrecisionTime.h"
#include "BCIEvent.h"

class WebcamLogger;

class WebcamThread : public Thread
{
public:
  WebcamThread ( int         _camIndex, 
                 int         _width, 
                 int         _height, 
                 int         _decimation, 
                 bool        _displayStream, 
                 int         _dateLocation,
                 bool        _useDirectShow,
                 std::string _fourcc
  );

	~WebcamThread      ();
	int  OnExecute     () override;
	void StartRecording(std::string _outputFile);
	void StopRecording ();
	bool Initalize     ();
	bool Connected     () const { return mVCapture.isOpened(); }
  void StopStream    ();

private:
	void InitalizeText();
  void GetFrame     ();

	Tiny::Mutex			   mMutex;
	
  bool               mUseDirectShow;
  cv::VideoCapture   mVCapture;
	cv::VideoWriter    mVideoWriter;
  std::string			   mWinName;

  bool						   mDisplayStream;
  bool               mAddDate;
	cv::Point				   mDatePoint;
  int                mDateLocation;

  unsigned long		   mFrameNum;
  unsigned long		   mCount;

  int 						   mSourceWidth;
  int                mSourceHeight;

  int                mCameraIndex;
  int                mDecimation;
  int                mFourcc;
	float						   mTargetFps;

  Synchronized<bool> mRecording;
};

#endif // WEBCAMTHREAD_H





