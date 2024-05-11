#include <SFML/Audio.hpp>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

namespace sound {
	

	class SoundManager {
	public:
		SoundManager();
		~SoundManager();

		void shootingSound();
		void explosionSound();
		void jumpSound();

	private:
		sf::SoundBuffer bgm_buffer;
		sf::Sound bgm;
		sf::SoundBuffer shooting_buffer;
		sf::Sound shooting_sound;
		sf::SoundBuffer explosion_buffer;
		sf::Sound explosion_sound;
		sf::SoundBuffer jump_buffer;
		sf::Sound jump_sound;


		std::string prefix = "D:/UCSD/2024/Spring/125/group5/client/audios/";
		std::string bgm_filepath = prefix + "bgm.wav";
		std::string shooting_sound_filepath = prefix + "gun_shot.wav";
		std::string explosion_sound_filepath = prefix + "explosion.wav";
		std::string jump_sound_filepath = prefix + "cartoon_jump.wav";

	};

	extern std::unique_ptr<SoundManager> soundManager;
	void initSoundManager();
}