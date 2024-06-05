#include "sound/SoundManager.h"


std::unique_ptr<sound::SoundManager> sound::soundManager;

void sound::initSoundManager() {
	sound::soundManager = std::make_unique<sound::SoundManager>();
}

sound::SoundManager::SoundManager() {


	
	// std::cout << "shader value: " << SetupParser::getValue("shader") << std::endl;

	// load BGM sound file into buffer
	if (!bgm_buffer.loadFromFile(bgm_filepath)) {
		std::cout << "Cannot load file: " << bgm_filepath << std::endl;
	}

	bgm.setBuffer(bgm_buffer);
	// make the BGM loop continuously
	bgm.setLoop(true);
	// set the BGM volume to half the original
	bgm.setVolume(100.0f);


	// load shooting sound
	if (!shooting_buffer.loadFromFile(shooting_sound_filepath)) {
		std::cout << "Cannot load file: " << shooting_sound_filepath << std::endl;
	}
	shooting_sound.setBuffer(shooting_buffer);
	shooting_sound.setVolume(15);

	// load jump sound
	if (!jump_buffer.loadFromFile(jump_sound_filepath)) {
		std::cout << "Cannot load file: " << jump_sound_filepath << std::endl;
	}
	jump_sound.setBuffer(jump_buffer);
	jump_sound.setVolume(50);

	// load explosion sound
	if (!explosion_buffer.loadFromFile(explosion_sound_filepath)) {
		std::cout << "Cannot load file: " << explosion_sound_filepath << std::endl;
	}
	explosion_sound.setBuffer(explosion_buffer);
	explosion_sound.setVolume(30);

	// load character theme songs
	for (int i = 0; i < 4; i++) {
		if (!character_themes_buffer[i].loadFromFile(character_themes_filepath[i])) {
			std::cout << "Cannot load character theme song file: " << character_themes_filepath[i] << std::endl;
		}
		character_themes[i].setBuffer(character_themes_buffer[i]);
		character_themes[i].setLoop(true);
		character_themes[i].setVolume(50);
	}

	std::cout << "Successfully load all sound files" << std::endl;

	// start BGM music for default selected character
	playCharacterTheme(0);

	// bgm.play(); // TODO: move to Client gameloop, refactor

}

sound::SoundManager::~SoundManager() {
	std::cout << "Sound Manager get deleted" << std::endl;
}

void sound::SoundManager::shootingSound() {
	bgm.pause();
	shooting_sound.play();
	bgm.play();
}


void sound::SoundManager::explosionSound() {
	bgm.pause();
	explosion_sound.play();
	bgm.play();
}

void sound::SoundManager::jumpSound() {
	bgm.pause();
	jump_sound.play();
	bgm.play();
}

void sound::SoundManager::playCharacterTheme(int characterSeason) {
	// there should be no "game bgm" in the character selection stage -- you just hear the character's theme songs
	//std::printf("playing character theme song %d\n", characterSeason);
	assert(characterSeason < 4);
	character_themes[characterSeason].play();
}

void sound::SoundManager::stopCharacterTheme(int characterSeason) {
	// there should be no "game bgm" in the character selection stage -- you just hear the character's theme songs
	assert(characterSeason < 4);
	//std::printf("pausing character theme song %d\n", characterSeason);
	character_themes[characterSeason].stop();
}

void sound::SoundManager::stopAllLobbyMusic() {
	// assuming that we have one theme for each character
	for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
		character_themes[i].stop();
	}

	// start bgm theme
	bgm.play();
}

void sound::SoundManager::muteBgmToggle() {
	if (bgm.getVolume() == 0) {
		bgm.setVolume(100.0f);;
	}
	else {
		bgm.setVolume(0.0f);;
	}
}