/////////////////////////////////////////////////////////////////////////////
// $Id: WebcamLogger.h 2118 6/30/2010
// Authors: adam.wilson@uc.edu and Alexander Belsten (belsten@neurotechcenter.org)
// Description: The Webcam Logger logs video data from a standard webcams,
// saves the videos in a compressed video format, and stores the frame numbers
// as states
//
// Parameterization:
//   Webcam logging is enabled from the command line adding
//     --LogWebcam=1
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
#ifndef WEBCAMLOGGER_H
#define WEBCAMLOGGER_H

#include <string>
#include <iostream>
#include <cstring>
#include <ctime>
#include <iomanip>

#include "WebcamThread.h"
#include "Environment.h"
#include "GenericVisualization.h"
#include "FileUtils.h"

class WebcamLogger : public EnvironmentExtension
{
public:

	friend class WebcamThread;

	//Constructors and virtual interface
	WebcamLogger();
	~WebcamLogger();
	void Publish() override;
  void AutoConfig () override;
	void Preflight() const override;
	void Initialize() override;
	void StartRun() override;
	void StopRun() override;
	void Halt() override;

private:
  bool							         mWebcamEnable;
	std::vector<WebcamThread*> mWebcamThreads;
};

#endif // WEBCAM_LOGGER_H





