#include "prism/soundeffect.h"

#include <kos.h>

#include "prism/file.h"
#include "prism/sound.h"

void initSoundEffects() {

}

void setupSoundEffectHandler() {
	
}

void shutdownSoundEffectHandler() {
	snd_sfx_unload_all();
}

int loadSoundEffect(char* tPath) {
    return 0;	
    char fullPath[1024];
	getFullPath(fullPath, tPath);
	return snd_sfx_load(fullPath);
}

int loadSoundEffectFromBuffer(Buffer tBuffer) {
    return 0;
	char tempPath[1024];
	strcpy(tempPath, "$/ram/tempsound.wav");

	bufferToFile(tempPath, tBuffer);
	int ret = loadSoundEffect(tempPath);
	fileUnlink(tempPath);

	return ret;
}

void unloadSoundEffect(int tID) {
	return;
	snd_sfx_unload(tID);
}

int playSoundEffect(int tID) {
    return -1;
	return snd_sfx_play(tID, 255, 128);
}

void stopSoundEffect(int tSFX) {
    return;
	snd_sfx_stop(tSFX);
}

void setSoundEffectVolume(double tVolume) {
	(void) tVolume;
}
