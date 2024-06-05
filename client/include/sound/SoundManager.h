#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <memory>
#include <iostream>
#include "SetupParser.h"
#include "GameConstants.h"

namespace sound {


	class SoundManager {
	public:
		SoundManager();
		~SoundManager();

		void shootingSound();
		void explosionSound();
		void jumpSound();

		void playCharacterTheme(int characterSeason);
		void stopCharacterTheme(int characterSeason);
		// stop all characters' theme music
		// do this when we are out of lobby
		void stopAllLobbyMusic();

		void muteBgmToggle();

	private:
		sf::SoundBuffer bgm_buffer;
		sf::Sound bgm;
		sf::SoundBuffer shooting_buffer;
		sf::Sound shooting_sound;
		sf::SoundBuffer explosion_buffer;
		sf::Sound explosion_sound;
		sf::SoundBuffer jump_buffer;
		sf::Sound jump_sound;

		sf::SoundBuffer character_themes_buffer[4];
		sf::Sound character_themes[4];


		std::string prefix = std::string(PROJECT_PATH) + "/client/audios/";
		// std::string bgm_filepath = prefix + "bgm.wav";
		std::string bgm_filepath = prefix + "four_seasons/four-seasons-inorder-traversal.wav";
		// std::string shooting_sound_filepath = prefix + "gun_shot.wav";
		std::string shooting_sound_filepath = prefix + "laser-gun.mp3";
		std::string explosion_sound_filepath = prefix + "explosion.wav";
		std::string jump_sound_filepath = prefix + "cartoon_jump.wav";

		std::string character_themes_filepath[4] = {
			prefix + "four_seasons/spring-intro.wav",
			prefix + "four_seasons/summer-intro.wav",
			prefix + "four_seasons/Autumn3.mp3",  // todo
			prefix + "four_seasons/Autumn3.mp3"  // TODO: change to winter
		};

	};

	extern std::unique_ptr<SoundManager> soundManager;
	void initSoundManager();
}