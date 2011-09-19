#ifndef _EMULWIN_H
#define _EMULWIN_H

#include <vector>
#include <QIcon>
#include <QTimer>
#include <stdint.h>

#ifndef WIN32
	#include <QX11EmbedContainer>
#else
	#include <QDialog>
	#include <SDL_syswm.h>
#endif

#include "spectrum.h"

// pause reasons
#define	PR_MENU		1
#define	PR_FILE		(1<<1)
#define	PR_OPTS		(1<<2)
#define	PR_DEBUG	(1<<3)
#define	PR_QUIT		(1<<4)
#define	PR_PAUSE	(1<<5)
#define	PR_EXTRA	(1<<6)

// flags - emul mode / events
#define	FL_GRAB		1L
#define	FL_RZX		(1L<<1)
#define	FL_SHOT		(1L<<2)
#define	FL_RESET	(1L<<3)
#define	FL_FAST		(1L<<4)

struct XBookmark {
	std::string name;
	std::string path;
};

struct XProfile {
	std::string name;
	std::string file;
	ZXComp* zx;
};

// TODO: kill EmulWin class?

class EmulWin : public QObject {
	Q_OBJECT
	public:
		EmulWin();
	private:
		QTimer *timer;
	private slots:
		void bookmarkSelected(QAction*);
		void profileSelected(QAction*);
	public slots:
		void SDLEventHandler();
};

class MainWin : public QX11EmbedContainer {
	Q_OBJECT
	public:
		MainWin();
	signals:
		void mwsOptions();
	protected:
		void closeEvent(QCloseEvent*);
};

// main
void emulInit();
void emulShow();
void emulUpdateWindow();
void emulSetIcon(const char*);
void emulPause(bool, int);
int emulGetFlags();
void emulSetFlag(int,bool);
bool emulSaveChanged();
void emulExec();
void emulStartTimer(int);
void emulStopTimer();
QWidget* emulWidget();

// USER MENU
void initUserMenu(QWidget*);
// bookmarks
void fillBookmarkMenu();
void addBookmark(std::string,std::string);
void setBookmark(int,std::string,std::string);
void delBookmark(int);
void clearBookmarks();
void swapBookmarks(int,int);
std::vector<XBookmark> getBookmarkList();
int getBookmarksCount();
// profiles
void fillProfileMenu();
void addProfile(std::string,std::string);
bool setProfile(std::string);
void clearProfiles();
std::vector<XProfile> getProfileList();
XProfile* getCurrentProfile();
// hardware
void initHardware();
void setHardware(ZXComp*, std::string);
std::vector<std::string> getHardwareNames();
std::vector<HardWare> getHardwareList();
// romset
void addRomset(RomSet);
void setRomset(ZXComp*, std::string);
std::vector<RomSet> getRomsetList();
#endif
