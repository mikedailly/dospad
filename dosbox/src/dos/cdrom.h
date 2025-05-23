/*
 *  Copyright (C) 2002-2021  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef __CDROM_INTERFACE__
#define __CDROM_INTERFACE__

#define MAX_ASPI_CDROM	5

#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "dosbox.h"
#include "mem.h"
#include "mixer.h"
#include "SDL.h"
#include "SDL_thread.h"

#if defined(C_SDL_SOUND)
#include "SDL_sound.h"
#endif

#ifdef IPHONEOS

/***** BEGIN COPY OF SDL_cdrom.h -- absent in SDL1.3 *****/

/** The maximum number of CD-ROM tracks on a disk */
#define SDL_MAX_TRACKS	99

/** @name Track Types
 *  The types of CD-ROM track possible
 */
/*@{*/
#define SDL_AUDIO_TRACK	0x00
#define SDL_DATA_TRACK	0x04
/*@}*/

/** The possible states which a CD-ROM drive can be in. */
typedef enum {
	CD_TRAYEMPTY,
	CD_STOPPED,
	CD_PLAYING,
	CD_PAUSED,
	CD_ERROR = -1
} CDstatus;

/** Given a status, returns true if there's a disk in the drive */
#define CD_INDRIVE(status)	((int)(status) > 0)

typedef struct SDL_CDtrack {
	Uint8 id;		/**< Track number */
	Uint8 type;		/**< Data or audio track */
	Uint16 unused;
	Uint32 length;		/**< Length, in frames, of this track */
	Uint32 offset;		/**< Offset, in frames, from start of disk */
} SDL_CDtrack;

/** This structure is only current as of the last call to SDL_CDStatus() */
typedef struct SDL_CD {
	int id;			/**< Private drive identifier */
	CDstatus status;	/**< Current drive status */

	/** The rest of this structure is only valid if there's a CD in drive */
        /*@{*/
	int numtracks;		/**< Number of tracks on disk */
	int cur_track;		/**< Current track position */
	int cur_frame;		/**< Current frame offset within current track */
	SDL_CDtrack track[SDL_MAX_TRACKS+1];
        /*@}*/
} SDL_CD;

/** @name Frames / MSF Conversion Functions
 *  Conversion functions from frames to Minute/Second/Frames and vice versa
 */
/*@{*/
#define CD_FPS	75
#define FRAMES_TO_MSF(f, M,S,F)	{					\
	int value = f;							\
	*(F) = value%CD_FPS;						\
	value /= CD_FPS;						\
	*(S) = value%60;						\
	value /= 60;							\
	*(M) = value;							\
}
#define MSF_TO_FRAMES(M, S, F)	((M)*60*CD_FPS+(S)*CD_FPS+(F))
/***** END COPY OF SDL_cdrom.h *****/

#endif

#define RAW_SECTOR_SIZE		2352
#define COOKED_SECTOR_SIZE	2048

enum { CDROM_USE_SDL, CDROM_USE_ASPI, CDROM_USE_IOCTL_DIO, CDROM_USE_IOCTL_DX, CDROM_USE_IOCTL_MCI };

typedef struct SMSF {
	unsigned char min;
	unsigned char sec;
	unsigned char fr;
} TMSF;

typedef struct SCtrl {
	Bit8u	out[4];			// output channel
	Bit8u	vol[4];			// channel volume
} TCtrl;

extern int CDROM_GetMountType(char* path, int force);

class CDROM_Interface
{
public:
//	CDROM_Interface						(void);
	virtual ~CDROM_Interface			(void) {};

	virtual bool	SetDevice			(char* path, int forceCD) = 0;

	virtual bool	GetUPC				(unsigned char& attr, char* upc) = 0;

	virtual bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut) = 0;
	virtual bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr) = 0;
	virtual bool	GetAudioSub			(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos) = 0;
	virtual bool	GetAudioStatus		(bool& playing, bool& pause) = 0;
	virtual bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen) = 0;

	virtual bool	PlayAudioSector		(unsigned long start,unsigned long len) = 0;
	virtual bool	PauseAudio			(bool resume) = 0;
	virtual bool	StopAudio			(void) = 0;
	virtual void	ChannelControl		(TCtrl ctrl) = 0;
	
	virtual bool	ReadSectors			(PhysPt buffer, bool raw, unsigned long sector, unsigned long num) = 0;

	virtual bool	LoadUnloadMedia		(bool unload) = 0;
	
	virtual void	InitNewMedia		(void) {};
};	

class CDROM_Interface_SDL : public CDROM_Interface
{
public:
	CDROM_Interface_SDL			(void);
	virtual ~CDROM_Interface_SDL(void);

	virtual bool	SetDevice			(char* path, int forceCD);
	virtual bool	GetUPC				(unsigned char& attr, char* upc) { attr = 0; strcpy(upc,"UPC"); return true; };
	virtual bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut);
	virtual bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr);
	virtual bool	GetAudioSub			(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos);
	virtual bool	GetAudioStatus		(bool& playing, bool& pause);
	virtual bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen);
	virtual bool	PlayAudioSector		(unsigned long start,unsigned long len);
	virtual bool	PauseAudio			(bool resume);
	virtual bool	StopAudio			(void);
	virtual void	ChannelControl		(TCtrl /*ctrl*/) { return; };
	virtual bool	ReadSectors			(PhysPt /*buffer*/, bool /*raw*/, unsigned long /*sector*/, unsigned long /*num*/) { return false; };
	virtual bool	LoadUnloadMedia		(bool unload);

private:
	bool	Open				(void);
	void	Close				(void);

	SDL_CD*	cd;
	int		driveID;
	Uint32	oldLeadOut;
};

class CDROM_Interface_Fake : public CDROM_Interface
{
public:
	bool	SetDevice			(char* /*path*/, int /*forceCD*/) { return true; };
	bool	GetUPC				(unsigned char& attr, char* upc) { attr = 0; strcpy(upc,"UPC"); return true; };
	bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut);
	bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr);
	bool	GetAudioSub			(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos);
	bool	GetAudioStatus		(bool& playing, bool& pause);
	bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen);
	bool	PlayAudioSector		(unsigned long /*start*/,unsigned long /*len*/) { return true; };
	bool	PauseAudio			(bool /*resume*/) { return true; };
	bool	StopAudio			(void) { return true; };
	void	ChannelControl		(TCtrl /*ctrl*/) { return; };
	bool	ReadSectors			(PhysPt /*buffer*/, bool /*raw*/, unsigned long /*sector*/, unsigned long /*num*/) { return true; };
	bool	LoadUnloadMedia		(bool /*unload*/) { return true; };
};	

class CDROM_Interface_Image : public CDROM_Interface
{
private:
	class TrackFile {
	public:
		virtual bool read(Bit8u *buffer, int seek, int count) = 0;
		virtual int getLength() = 0;
		virtual ~TrackFile() { };
	};
	
	class BinaryFile : public TrackFile {
	public:
		BinaryFile(const char *filename, bool &error);
		~BinaryFile();
		bool read(Bit8u *buffer, int seek, int count);
		int getLength();
	private:
		BinaryFile();
		std::ifstream *file;
	};
	
	#if defined(C_SDL_SOUND)
	class AudioFile : public TrackFile {
	public:
		AudioFile(const char *filename, bool &error);
		~AudioFile();
		bool read(Bit8u *buffer, int seek, int count);
		int getLength();
	private:
		AudioFile();
		Sound_Sample *sample;
		int lastCount;
		int lastSeek;
#ifdef IPHONEOS
		int remainingBytes;
#endif
	};
	#endif
	
	struct Track {
		int number;
		int attr;
		int start;
		int length;
		int skip;
		int sectorSize;
		bool mode2;
		TrackFile *file;
	};
	
public:
	CDROM_Interface_Image		(Bit8u subUnit);
	virtual ~CDROM_Interface_Image	(void);
	void	InitNewMedia		(void);
	bool	SetDevice		(char* path, int forceCD);
	bool	GetUPC			(unsigned char& attr, char* upc);
	bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut);
	bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr);
	bool	GetAudioSub		(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos);
	bool	GetAudioStatus		(bool& playing, bool& pause);
	bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen);
	bool	PlayAudioSector		(unsigned long start,unsigned long len);
	bool	PauseAudio		(bool resume);
	bool	StopAudio		(void);
	void	ChannelControl		(TCtrl ctrl);
	bool	ReadSectors		(PhysPt buffer, bool raw, unsigned long sector, unsigned long num);
	bool	LoadUnloadMedia		(bool unload);
	bool	ReadSector		(Bit8u *buffer, bool raw, unsigned long sector);
	bool	HasDataTrack		(void);
	
static	CDROM_Interface_Image* images[26];

private:
	// player
static	void	CDAudioCallBack(Bitu len);
	int	GetTrack(int sector);

static  struct imagePlayer {
		CDROM_Interface_Image *cd;
		MixerChannel   *channel;
		SDL_mutex 	*mutex;
		Bit8u   buffer[8192];
		int     bufLen;
		int     currFrame;	
		int     targetFrame;
		bool    isPlaying;
		bool    isPaused;
		bool    ctrlUsed;
		TCtrl   ctrlData;
	} player;
	
	void 	ClearTracks();
	bool	LoadIsoFile(char *filename);
	bool	CanReadPVD(TrackFile *file, int sectorSize, bool mode2);
	// cue sheet processing
	bool	LoadCueSheet(char *cuefile);
	bool	GetRealFileName(std::string& filename, std::string& pathname);
	bool	GetCueKeyword(std::string &keyword, std::istream &in);
	bool	GetCueFrame(int &frames, std::istream &in);
	bool	GetCueString(std::string &str, std::istream &in);
	bool	AddTrack(Track &curr, int &shift, int prestart, int &totalPregap, int currPregap);

static	int	refCount;
	std::vector<Track>	tracks;
typedef	std::vector<Track>::iterator	track_it;
	std::string	mcn;
	Bit8u	subUnit;
};

#if defined (WIN32)	/* Win 32 */

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include "wnaspi32.h"			// Aspi stuff 

class CDROM_Interface_Aspi : public CDROM_Interface
{
public:
	CDROM_Interface_Aspi		(void);
	virtual ~CDROM_Interface_Aspi(void);

	bool	SetDevice			(char* path, int forceCD);

	bool	GetUPC				(unsigned char& attr, char* upc);

	bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut);
	bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr);
	bool	GetAudioSub			(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos);
	bool	GetAudioStatus		(bool& playing, bool& pause);
	bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen);

	bool	PlayAudioSector		(unsigned long start,unsigned long len);
	bool	PauseAudio			(bool resume);
	bool	StopAudio			(void);
	void	ChannelControl		(TCtrl /*ctrl*/) { return; };
	
	bool	ReadSectors			(PhysPt buffer, bool raw, unsigned long sector, unsigned long num);

	bool	LoadUnloadMedia		(bool unload);
	
private:
	DWORD	GetTOC				(LPTOC toc);
	HANDLE	OpenIOCTLFile		(char cLetter, BOOL bAsync);
	void	GetIOCTLAdapter		(HANDLE hF,int * iDA,int * iDT,int * iDL);
	bool	ScanRegistryFindKey	(HKEY& hKeyBase);
	bool	ScanRegistry		(HKEY& hKeyBase);
	BYTE	GetHostAdapter		(char* hardwareID);
	bool	GetVendor			(BYTE HA_num, BYTE SCSI_Id, BYTE SCSI_Lun, char* szBuffer);
		
	// ASPI stuff
	BYTE	haId;
	BYTE	target;
	BYTE	lun;
	char	letter;

	// Windows stuff
	HINSTANCE	hASPI;
	HANDLE		hEvent;											// global event
	DWORD		(*pGetASPI32SupportInfo)	(void);             // ptrs to aspi funcs
	DWORD		(*pSendASPI32Command)		(LPSRB);
	TMSF		oldLeadOut;
};

class CDROM_Interface_Ioctl : public CDROM_Interface
{
public:
	enum cdioctl_cdatype { CDIOCTL_CDA_DIO, CDIOCTL_CDA_MCI, CDIOCTL_CDA_DX };
	cdioctl_cdatype cdioctl_cda_selected;

	CDROM_Interface_Ioctl		(cdioctl_cdatype ioctl_cda);
	virtual ~CDROM_Interface_Ioctl(void);

	bool	SetDevice			(char* path, int forceCD);

	bool	GetUPC				(unsigned char& attr, char* upc);

	bool	GetAudioTracks		(int& stTrack, int& end, TMSF& leadOut);
	bool	GetAudioTrackInfo	(int track, TMSF& start, unsigned char& attr);
	bool	GetAudioSub			(unsigned char& attr, unsigned char& track, unsigned char& index, TMSF& relPos, TMSF& absPos);
	bool	GetAudioStatus		(bool& playing, bool& pause);
	bool	GetMediaTrayStatus	(bool& mediaPresent, bool& mediaChanged, bool& trayOpen);

	bool	PlayAudioSector		(unsigned long start,unsigned long len);
	bool	PauseAudio			(bool resume);
	bool	StopAudio			(void);
	void	ChannelControl		(TCtrl ctrl);
	
	bool	ReadSector			(Bit8u *buffer, bool raw, unsigned long sector);
	bool	ReadSectors			(PhysPt buffer, bool raw, unsigned long sector, unsigned long num);

	bool	LoadUnloadMedia		(bool unload);

	void	InitNewMedia		(void) { Close(); Open(); };
private:

	bool	Open				(void);
	void	Close				(void);

	char	pathname[32];
	HANDLE	hIOCTL;
	TMSF	oldLeadOut;


	/* track start/length data */
	bool	track_start_valid;
	int		track_start_first,track_start_last;
	int		track_start[128];

	bool	GetAudioTracksAll	(void);


	/* mci audio cd interface */
	bool	use_mciplay;
	int		mci_devid;

	bool	mci_CDioctl				(UINT msg, DWORD flags, void *arg);
	bool	mci_CDOpen				(char drive);
	bool	mci_CDClose				(void);
	bool	mci_CDPlay				(int start, int length);
	bool	mci_CDPause				(void);
	bool	mci_CDResume			(void);
	bool	mci_CDStop				(void);
	int		mci_CDStatus			(void);
	bool	mci_CDPosition			(int *position);


	/* digital audio extraction cd interface */
	static void dx_CDAudioCallBack(Bitu len);

	bool	use_dxplay;
	static  struct dxPlayer {
		CDROM_Interface_Ioctl *cd;
		MixerChannel	*channel;
		SDL_mutex		*mutex;
		Bit8u   buffer[8192];
		int     bufLen;
		int     currFrame;	
		int     targetFrame;
		bool    isPlaying;
		bool    isPaused;
		bool    ctrlUsed;
		TCtrl   ctrlData;
	} player;

};

#endif /* WIN 32 */

#if defined (LINUX) || defined(OS2)

class CDROM_Interface_Ioctl : public CDROM_Interface_SDL
{
public:
	CDROM_Interface_Ioctl		(void);

	bool	SetDevice		(char* path, int forceCD);
	bool	GetUPC			(unsigned char& attr, char* upc);
	bool	ReadSectors		(PhysPt buffer, bool raw, unsigned long sector, unsigned long num);

private:
	char	device_name[512];
};

#endif /* LINUX */

#endif /* __CDROM_INTERFACE__ */
