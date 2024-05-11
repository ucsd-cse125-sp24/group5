#include "sound/SoundManager.h"


std::unique_ptr<sound::SoundManager> sound::soundManager;

void sound::initSoundManager() {
	sound::soundManager = std::make_unique<sound::SoundManager>();
}

sound::SoundManager::SoundManager() {


	
	std::cout << "shader value: " << SetupParser::getValue("shader") << std::endl;

	// load BGM sound file into buffer
	if (!bgm_buffer.loadFromFile(bgm_filepath)) {
		std::cout << "Cannot load file: " << bgm_filepath << std::endl;
	}

	bgm.setBuffer(bgm_buffer);
	// make the BGM loop continuously
	bgm.setLoop(true);
	// set the BGM volume to half the original
	bgm.setVolume(50.f);


	// load shooting sound
	if (!shooting_buffer.loadFromFile(shooting_sound_filepath)) {
		std::cout << "Cannot load file: " << shooting_sound_filepath << std::endl;
	}
	shooting_sound.setBuffer(shooting_buffer);

	// load jump sound
	if (!jump_buffer.loadFromFile(jump_sound_filepath)) {
		std::cout << "Cannot load file: " << jump_sound_filepath << std::endl;
	}
	jump_sound.setBuffer(jump_buffer);

	// load explosion sound
	if (!explosion_buffer.loadFromFile(explosion_sound_filepath)) {
		std::cout << "Cannot load file: " << explosion_sound_filepath << std::endl;
	}
	explosion_sound.setBuffer(explosion_buffer);

	std::cout << "Successfully load all sound files" << std::endl;

	// start BGM music
	bgm.play();
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