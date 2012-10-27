#include "filetypes.h"
#include <string.h>
#include <vector>

struct FilePos {
	unsigned char trk;
	unsigned char sec;
	unsigned char slen;
};

int loadSCL(Floppy* flp,const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	unsigned char* buf = new unsigned char[0x1000];		// track 0 image
	unsigned char* bptr;
	unsigned char fcnt;
	int tmpa;
	int scnt;
	unsigned int i;
	file.read((char*)buf,9);
	if (strncmp((char*)buf,"SINCLAIR",8) != 0) return ERR_SCL_SIGN;
	if (buf[8] > 0x80) return ERR_SCL_MANY;
	flpFormat(flp);
	fcnt = buf[8];		// files total
	for (i = 0; i < 0x1000; i++) buf[i]=0x00;	// TRK0 in buf (clear)
	scnt = 0x10;
	bptr = buf;		// start of TRK0
	for (i = 0; i < fcnt; i++) {		// make catalog
		file.read((char*)bptr,14);		// file dsc
		bptr[14] = scnt & 0x0f;			// sector
		bptr[15] = ((scnt & 0xff0) >> 4);	// track
		scnt += bptr[13];			// +sectors size
		bptr += 16;				// next file
	}
	bptr[0] = 0;				// mark last file
	buf[0x800] = 0;
	buf[0x8e1] = scnt & 0x0f;		// free sector
	buf[0x8e2] = ((scnt & 0xff0) >> 4);	// free track
	buf[0x8e3] = 0x16;			// 80DS
	buf[0x8e4] = fcnt;			// files total
	tmpa = 0x9f0 - scnt;			// sectors free (0x9f0)	// FIXED: not 0xa00!
	buf[0x8e5] = (tmpa & 0xff);
	buf[0x8e6] = ((tmpa & 0xff00) >> 8);
	buf[0x8e7] = 0x10;			// trdos code
	flpFormTRDTrack(flp,0,buf);
	i = 1;
	while (!file.eof()) {
		file.read((char*)buf,0x1000);
		flpFormTRDTrack(flp,i,buf);
		i++;
	}
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->flag |= FLP_INSERT;
	loadBoot(flp);
	flp->flag &= ~FLP_CHANGED;
	return ERR_OK;
}

int saveSCL(Floppy* flp,const char* name) {
	const char* sign = "SINCLAIR";
	unsigned char* img = new unsigned char[0xA0000];
	unsigned char* buf = new unsigned char[256];
	unsigned char* dptr = img;
	unsigned char* bptr;
	FilePos newfp;
	std::vector<FilePos> fplist;
	int i,j;
	unsigned char tr,sc;

	memcpy(img,sign,8);
	img[8] = 0;
	dptr = img + 9;
	for (i = 1; i < 9; i++) {
		flpGetSectorData(flp,0,i,buf,256);
		bptr = buf;
		for (j = 0; j < 16; j++) {
			if (*bptr == 0) {
				i=20;
				j=20;
			} else {
				if (*bptr != 1) {
					memcpy(dptr,bptr,14);
					newfp.trk = *(bptr + 15);
					newfp.sec = *(bptr + 14);
					newfp.slen = *(bptr + 13);
					fplist.push_back(newfp);
					dptr += 14;
					img[8]++;
				}
				bptr += 16;
			}
		}
	}
	for (i = 0; i < (int)fplist.size(); i++) {
		tr = fplist[i].trk;
		sc = fplist[i].sec;
		for(j = 0; j < fplist[i].slen; j++) {
			flpGetSectorData(flp,tr, sc + 1, dptr, 256);
			dptr += 256;
			sc++;
			if (sc > 15) {
				tr++;
				sc=0;
			}
		}
	}
	j=0;
	for(i = 0; i < dptr - img; i++) {
		j += *(img + i);
	}
	putint(dptr, j);
	dptr += 4;

	std::ofstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	file.write((char*)img,dptr-img);
	file.close();

	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	flp->flag &= ~FLP_CHANGED;

	return ERR_OK;
}
