/* SPU2-X, A plugin for Emulating the Sound Processing Unit of the Playstation 2
 * Developed and maintained by the Pcsx2 Development Team.
 *
 * Original portions from SPU2ghz are (c) 2008 by David Quintana [gigaherz]
 *
 * SPU2-X is free software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free Software Found-
 * ation, either version 3 of the License, or (at your option) any later version.
 *
 * SPU2-X is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with SPU2-X.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Global.h"
#include "Linux/Dialogs.h"
#include "Config.h"

#ifdef PCSX2_DEVBUILD
static const int LATENCY_MAX = 3000;
#else
static const int LATENCY_MAX = 750;
#endif

static const int LATENCY_MIN = 40;

int AutoDMAPlayRate[2] = {0,0};

// Default settings.

// MIXING
int Interpolation = 4;
/* values:
		0: no interpolation (use nearest)
		1. linear interpolation
		2. cubic interpolation
		3. hermite interpolation
		4. catmull-rom interpolation
*/

bool EffectsDisabled = false;
float FinalVolume; // global
bool AdvancedVolumeControl;
float VolumeAdjustFLdb; // decibels settings, cos audiophiles love that
float VolumeAdjustCdb;
float VolumeAdjustFRdb;
float VolumeAdjustBLdb;
float VolumeAdjustBRdb;
float VolumeAdjustSLdb;
float VolumeAdjustSRdb;
float VolumeAdjustLFEdb;
float VolumeAdjustFL;   // linear coefs calcualted from decibels,
float VolumeAdjustC;
float VolumeAdjustFR;
float VolumeAdjustBL;
float VolumeAdjustBR;
float VolumeAdjustSL;
float VolumeAdjustSR;
float VolumeAdjustLFE;

bool postprocess_filter_enabled = true;
bool postprocess_filter_dealias = false;
bool _visual_debug_enabled = false; // windows only feature

// OUTPUT
u32 OutputModule = 0;
int SndOutLatencyMS = 300;
int SynchMode = 0; // Time Stretch, Async or Disabled
static u32 OutputAPI = 0;

int numSpeakers = 0;
int dplLevel = 0;
/*****************************************************************************/

void ReadSettings()
{
	// For some reason this can be called before we know what ini file we're writing to.
	// Lets not try to read it if that happens.
	if (!pathSet) 
	{
		FileLog("Read called without the path set.\n");
		return;
	}
	
	Interpolation = CfgReadInt( L"MIXING",L"Interpolation", 4 );
	EffectsDisabled = CfgReadBool( L"MIXING", L"Disable_Effects", false );
	postprocess_filter_dealias = CfgReadBool( L"MIXING", L"DealiasFilter", false );
	FinalVolume = ((float)CfgReadInt( L"MIXING", L"FinalVolume", 100 )) / 100;
	if ( FinalVolume > 1.0f) FinalVolume = 1.0f;

	AdvancedVolumeControl = CfgReadBool(L"MIXING", L"AdvancedVolumeControl", false);
	VolumeAdjustCdb = CfgReadFloat(L"MIXING", L"VolumeAdjustC(dB)", 0);
	VolumeAdjustFLdb = CfgReadFloat(L"MIXING", L"VolumeAdjustFL(dB)", 0);
	VolumeAdjustFRdb = CfgReadFloat(L"MIXING", L"VolumeAdjustFR(dB)", 0);
	VolumeAdjustBLdb = CfgReadFloat(L"MIXING", L"VolumeAdjustBL(dB)", 0);
	VolumeAdjustBRdb = CfgReadFloat(L"MIXING", L"VolumeAdjustBR(dB)", 0);
	VolumeAdjustSLdb = CfgReadFloat(L"MIXING", L"VolumeAdjustSL(dB)", 0);
	VolumeAdjustSRdb = CfgReadFloat(L"MIXING", L"VolumeAdjustSR(dB)", 0);
	VolumeAdjustLFEdb = CfgReadFloat(L"MIXING", L"VolumeAdjustLFE(dB)", 0);
	VolumeAdjustC = powf(10, VolumeAdjustCdb / 10);
	VolumeAdjustFL = powf(10, VolumeAdjustFLdb / 10);
	VolumeAdjustFR = powf(10, VolumeAdjustFRdb / 10);
	VolumeAdjustBL = powf(10, VolumeAdjustBLdb / 10);
	VolumeAdjustBR = powf(10, VolumeAdjustBRdb / 10);
	VolumeAdjustSL = powf(10, VolumeAdjustSLdb / 10);
	VolumeAdjustSR = powf(10, VolumeAdjustSRdb / 10);
	VolumeAdjustLFE = powf(10, VolumeAdjustLFEdb / 10);


	wxString temp;
	CfgReadStr( L"OUTPUT", L"Output_Module", temp, PortaudioOut->GetIdent() );
	OutputModule = FindOutputModuleById( temp.c_str() );// find the driver index of this module

	// find current API
	CfgReadStr( L"PORTAUDIO", L"HostApi", temp, L"ALSA" );
	OutputAPI = -1;
	if (temp == L"ALSA") OutputAPI = 0;
	if (temp == L"OSS")  OutputAPI = 1;
	if (temp == L"JACK") OutputAPI = 2;

	SndOutLatencyMS = CfgReadInt(L"OUTPUT",L"Latency", 300);
	SynchMode = CfgReadInt( L"OUTPUT", L"Synch_Mode", 0);

	PortaudioOut->ReadSettings();
	SoundtouchCfg::ReadSettings();
	DebugConfig::ReadSettings();

	// Sanity Checks
	// -------------

	Clampify( SndOutLatencyMS, LATENCY_MIN, LATENCY_MAX );

	WriteSettings();
	spuConfig->Flush();
}

/*****************************************************************************/

void WriteSettings()
{
	if (!pathSet) 
	{
		FileLog("Write called without the path set.\n");
		return;
	}

	CfgWriteInt(L"MIXING",L"Interpolation",Interpolation);
	CfgWriteBool(L"MIXING",L"Disable_Effects",EffectsDisabled);
	CfgWriteBool(L"MIXING",L"DealiasFilter",postprocess_filter_dealias);
	CfgWriteInt(L"MIXING",L"FinalVolume",(int)(FinalVolume * 100 +0.5f));

	CfgWriteBool(L"MIXING", L"AdvancedVolumeControl", AdvancedVolumeControl);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustC(dB)", VolumeAdjustCdb);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustFL(dB)", VolumeAdjustFLdb);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustFR(dB)", VolumeAdjustFRdb);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustBL(dB)", VolumeAdjustBLdb);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustBR(dB)", VolumeAdjustBRdb);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustSL(dB)", VolumeAdjustSLdb);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustSR(dB)", VolumeAdjustSRdb);
	CfgWriteFloat(L"MIXING", L"VolumeAdjustLFE(dB)", VolumeAdjustLFEdb);

	CfgWriteStr(L"OUTPUT",L"Output_Module", mods[OutputModule]->GetIdent() );
	CfgWriteInt(L"OUTPUT",L"Latency", SndOutLatencyMS);
	CfgWriteInt(L"OUTPUT",L"Synch_Mode", SynchMode);

	PortaudioOut->WriteSettings();
	SoundtouchCfg::WriteSettings();
	DebugConfig::WriteSettings();
}

void advanced_dialog()
{
	SoundtouchCfg::DisplayDialog();
}

void debug_dialog()
{
	DebugConfig::DisplayDialog();
}

void DisplayDialog()
{

}

void configure()
{
	initIni();
	ReadSettings();
	DisplayDialog();
	WriteSettings();
	delete spuConfig;
	spuConfig = NULL;
}
