#ifndef _RED2Launcher_config_ini_writer_h_
#define _RED2Launcher_config_ini_writer_h_

#include "psyx_control_mapping.h"

struct config_data_t
{
	// settings
	char* imageFilename;
	int fullScreen;
	int pgxpTextureCorrection;
	int pgxpZBuffer;
	int bilinearFiltering;
	int disableChicagoBridges;
	int fieldOfView;
	int widescreenOverlayAlign;
	int fastLoadingScreens;
	int languageId;
	PsyXKeyboardMapping keyboardCtrls;
	PsyXControllerMapping controllerCtrls;

	// stereo settings
	int stereoMode { 0 };
	int stereoSwapEyes { 0 };
	double stereoConvergence { 0.0 };
	double stereoSeparation { 1.0 };
};

void BestDefaultConfig(config_data_t* data);

bool SaveNewConfigFile(config_data_t* data);

#endif
