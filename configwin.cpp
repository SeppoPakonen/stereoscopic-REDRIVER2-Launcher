#include "RED2Launcher.h"
#include "ini.h"
#include "config_ini_writer.h"

const char* LanguageList[] = {
	"English",
	"Italian",
	"German",
	"French",
	"Spanish",
};

const char* StereoModeList[] = {
	"Disabled",
	"Anaglyph Simple",
	"Anaglyph Full-Color",
	"Side-by-Side",
	"Top-and-Bottom",
	"Interlaced Scanlines",
	"Polarized",
	"Checkerboard",
};

ConfigWindow::ConfigWindow()
{
	CtrlLayout(*this, "Configuration");

	for(int i = 0; i < sizeof(LanguageList) / sizeof(char*); i++)
		languageList.Add(LanguageList[i]);

	for(int i = 0; i < sizeof(StereoModeList) / sizeof(char*); i++)
		stereoModeList.Add(StereoModeList[i]);
	
	finishBtn << [=] {
			if(StoreConfig())
				Close();
			else
				Exclamation("Unable to save 'config.ini'!");
		};
		
	imageChooseBtn << [=] {
			if(!fs.ExecuteOpen("Select ISO/BIN image"))
				return;
			String filename = fs;
			imageFilenameEdit <<= filename;
		};
		
	useImageCheck << [=] {
		imageChooseBtn.Enable(useImageCheck.Get());
		imageFilenameEdit.Enable(useImageCheck.Get());
	};
	
	keyboardCtrlsBtn << [=] {
		ctrlWindow.InitButtons(false);
		ctrlWindow.Open(this);
	};
	
	joy1CtrlsBtn << [=] {
		ctrlWindow.InitButtons(true);
		ctrlWindow.Open(this);
	};
	
	fs.ActiveDir(".");
	fs.Type("Compatible ISO 9660 files (.bin, .iso, .ccd)", "*.bin *.iso *.ccd");
	
	useImageCheck.Set(0);
	imageChooseBtn.Enable(0);
	imageFilenameEdit.Enable(0);

	// Initialize stereo controls with defaults
	stereoModeList.SetIndex(0);
	stereoSwapEyesCheck.Set(0);
	convergenceSpin.SetData(0.5);
	separationSpin.SetData(1.0);

	// temporarily disabled
	//keyboardCtrlsBtn.Enable(0);
	//joy1CtrlsBtn.Enable(0);

	// load config.ini
	LoadConfig();
}

void ConfigWindow::LoadConfig()
{
	ini_t* config;

	BestDefaultConfig(&cfgData);

	config = ini_load("config.ini");
	
	if(config)
	{
		const char* cdImageFileName = ini_get(config, "cdfs", "image");
		
		if(cdImageFileName)
		{
			useImageCheck.Set(1);
			imageChooseBtn.Enable(1);
			imageFilenameEdit.Enable(1);
		}
		imageFilenameEdit <<= cdImageFileName;
		
		// configure Psy-X pads
		//ini_sget(config, "pad", "pad1device", "%d", &g_controllerToSlotMapping[0]);
		//ini_sget(config, "pad", "pad2device", "%d", &g_controllerToSlotMapping[1]);

		// configure Psy-X renderer
		//ini_sget(config, "render", "windowWidth", "%d", &windowWidth);
		//ini_sget(config, "render", "windowHeight", "%d", &windowHeight);
		ini_sget(config, "render", "fullscreen", "%d", &cfgData.fullScreen);
		ini_sget(config, "render", "pgxpTextureMapping", "%d", &cfgData.pgxpTextureCorrection);
		ini_sget(config, "render", "pgxpZbuffer", "%d", &cfgData.pgxpZBuffer);
		ini_sget(config, "render", "bilinearFiltering", "%d", &cfgData.bilinearFiltering);

		// configure host game
		//ini_sget(config, "game", "drawDistance", "%d", &gDrawDistance);
		ini_sget(config, "game", "disableChicagoBridges", "%d", &cfgData.disableChicagoBridges);
		ini_sget(config, "game", "fieldOfView", "%d", &cfgData.fieldOfView);
		//ini_sget(config, "game", "freeCamera", "%d", &enableFreecamera);
		//ini_sget(config, "game", "driver1music", "%d", &gDriver1Music);
		ini_sget(config, "game", "widescreenOverlays", "%d", &cfgData.widescreenOverlayAlign);
		ini_sget(config, "game", "fastLoadingScreens", "%d", &cfgData.fastLoadingScreens);
		ini_sget(config, "game", "languageId", "%d", &cfgData.languageId);
		//ini_sget(config, "game", "overrideContent", "%d", &gContentOverride);
		
		ParseKeyboardMappings(config, "kbcontrols_game", cfgData.keyboardCtrls);
		//ParseKeyboardMappings(config, "kbcontrols_menu", g_kbMenuMappings);

		ParseControllerMappings(config, "controls_game", cfgData.controllerCtrls);
		//ParseControllerMappings(config, "controls_menu", g_gcMenuMappings);

		// Load stereo settings
		ini_sget(config, "render", "stereoMode", "%d", &cfgData.stereoMode);
		ini_sget(config, "render", "stereoSwapEyes", "%d", &cfgData.stereoSwapEyes);
		ini_sget(config, "render", "stereoConvergence", "%lf", &cfgData.stereoConvergence);
		ini_sget(config, "render", "stereoSeparation", "%lf", &cfgData.stereoSeparation);

		ini_free(config);
	}

	fullscreenCheck.Set(cfgData.fullScreen);
	pgxpTextureMappingCheck.Set(cfgData.pgxpTextureCorrection);
	pgxpZbufferCheck.Set(cfgData.pgxpZBuffer);
	bilinearFilteringCheck.Set(cfgData.bilinearFiltering);
	disableChicagoBridgesCheck.Set(cfgData.disableChicagoBridges);
	fieldOfViewSpin <<= cfgData.fieldOfView;
	widescreenOverlaysCheck.Set(cfgData.widescreenOverlayAlign);
	fastLoadingScreensCheck.Set(cfgData.fastLoadingScreens);
	languageList.SetIndex(cfgData.languageId);

	// Load stereo settings into UI
	stereoModeList.SetIndex(cfgData.stereoMode);
	stereoSwapEyesCheck.Set(cfgData.stereoSwapEyes);
	convergenceSpin <<= cfgData.stereoConvergence;
	separationSpin <<= cfgData.stereoSeparation;

	// copy controls
	ctrlWindow.keyboardCtrls = cfgData.keyboardCtrls;
	ctrlWindow.controllerCtrls = cfgData.controllerCtrls;
}

bool ConfigWindow::StoreConfig()
{
	// copy controls
	cfgData.keyboardCtrls = ctrlWindow.keyboardCtrls;
	cfgData.controllerCtrls = ctrlWindow.controllerCtrls;

	std::string imageFilenameStr = (~imageFilenameEdit).ToStd();

	cfgData.imageFilename = (char*)imageFilenameStr.c_str();
	cfgData.fullScreen = fullscreenCheck.Get();
	cfgData.pgxpTextureCorrection = pgxpTextureMappingCheck.Get();
	cfgData.pgxpZBuffer = pgxpZbufferCheck.Get();
	cfgData.bilinearFiltering = bilinearFilteringCheck.Get();
	cfgData.disableChicagoBridges = disableChicagoBridgesCheck.Get();
	cfgData.fieldOfView = ~fieldOfViewSpin;
	cfgData.widescreenOverlayAlign = widescreenOverlaysCheck.Get();
	cfgData.fastLoadingScreens = fastLoadingScreensCheck.Get();
	cfgData.languageId = languageList.GetIndex();

	// Store stereo settings
	cfgData.stereoMode = stereoModeList.GetIndex();
	cfgData.stereoSwapEyes = stereoSwapEyesCheck.Get();
	cfgData.stereoConvergence = ~convergenceSpin;
	cfgData.stereoSeparation = ~separationSpin;

	return SaveNewConfigFile(&cfgData);
}

