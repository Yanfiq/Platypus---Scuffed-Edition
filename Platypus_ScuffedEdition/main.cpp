#include <stdbool.h>
#include <iostream>
#include "SFML/Audio.hpp"

#include "player.h"
#include "bullet.h"
#include "enemy.h"
#include "Text.h"
#include "func.h"
#include "bar.h"
#include "textureManager.h"
#include "InputManager.h"
#include "animation.h"

int main() {
	sf::RenderWindow window;
	window.create(sf::VideoMode(1280, 720), "Platypus Scuffed Edition", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(60);

	sf::View view(sf::FloatRect(0.f, 0.f, 1280.f, 720.f));
	window.setView(view);

	sf::Clock clock;
	sf::Clock elapsed;
	sf::Event event;
	sf::Music bgmusic;
	bgmusic.openFromFile("audio/Boooring.ogg");
	bgmusic.play();
	bgmusic.setLoop(true);
	
	//texture initialization
	textRenderer TextRenderer("fonts/Poppins-SemiBold.ttf");
	textureManager::initializeTexture();
	sounds::loadSound();
 
	// enumeration for scene changes
	enum part { start, settings , tutorial, credits, transition, singleMulti, play, pause };

	// variables that'll be used inside the main game
	int level = 0;					// as the name implies, to differentiate levels
	int currentPoint = 0;			// as the name implies, to save the point calculation result
	bool generateEnemy = false;		// decides whether to generate new enemies
	bool endless = false;			// endless mode
	bool cheat = false;				// cheat

	// variables that'll be used on multiple choices scene
	int scene = start;				// decide what scene is being run

	//volumes
	int bgmVolume = 0;
	int sfxVolume = 50;

	//binding keys
	InputManager::Instance()->KBind("Enter", sf::Keyboard::Enter);
	InputManager::Instance()->KBind("Spacebar", sf::Keyboard::Space);
	InputManager::Instance()->KBind("Back", sf::Keyboard::Escape);
	InputManager::Instance()->KBind("Up_1", sf::Keyboard::Up);
	InputManager::Instance()->KBind("Up_2", sf::Keyboard::W);
	InputManager::Instance()->KBind("Down_1", sf::Keyboard::Down);
	InputManager::Instance()->KBind("Down_2", sf::Keyboard::S);
	InputManager::Instance()->KBind("Left_1", sf::Keyboard::Left);
	InputManager::Instance()->KBind("Left_2", sf::Keyboard::A);
	InputManager::Instance()->KBind("Right_1", sf::Keyboard::Right);
	InputManager::Instance()->KBind("Right_2", sf::Keyboard::D);

	// main game loop
	while (window.isOpen()) {
		bgmusic.setVolume(bgmVolume);
		sounds::monitoring();

		// POLL EVENT SECTION ----------------------------------------------------------------------------------------------------------------------------------------------------------
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::Resized:
			{
				int width = (event.size.width < 1280) ? 1280 : event.size.width;
				int height = (event.size.height < 720) ? 720 : event.size.height;
				sf::FloatRect visibleArea(0.f, 0.f, width, height);
				std::cout << width << " , " << height << std::endl;
				window.setSize(sf::Vector2u(width, height));
				window.setView(sf::View(visibleArea));
				break;
			}
			}
			InputManager::Instance()->KRUpdate(event);
		}
		// END OF POLL EVENT SECTION ---------------------------------------------------------------------------------------------------------------------------------------------------

		// RENDER SECTION --------------------------------------------------------------------------------------------------------------------------------------------------------------
		textureManager::displayImage(window, "background_nebula.jpg", window.getSize().x/2, window.getSize().y/2, sf::Vector2f(window.getSize()));
		// delta time between frame
		float dt = clock.restart().asSeconds();
		if (scene == pause) {
			if (!cheat) {
				player::justDrawAllObject(window);
				bullet::justDrawAllObject(window);
			}
			else {
				player::updateNDrawAllObject(dt, window);
				bullet::updateNDrawAllObject(dt, window);
			}

			enemy::justDrawAllObject(window);
		}
		else {
			player::updateNDrawAllObject(dt, window);
			bullet::updateNDrawAllObject(dt, window);
			enemy::updateNDrawAllObject(dt, window);
		}
		currentPoint += processCollision(sfxVolume);
		animate::monitoringAnimation(window);
		// END OF RENDER RECTION -----------------------------------------------------------------------------------------------------------------------------------------------------------

		// SCENES AND VIDEO GAME LOGIC -----------------------------------------------------------------------------------------------------------------------------------------------------
		switch (scene) {
		case start:
		{
			bullet::clearObject();
			enemy::clearObject();
			TextRenderer.displayText(window, "something is happening somewhere", 1, 60, sf::Color::White, window.getSize().x / 2, 100);

			static int choice = 0;
			std::vector<std::string> choices = { "START", "SETTINGS", "EXIT" };
			TextRenderer.displayMultipleChoice(window, choices, choice, 50, sf::Color::Cyan, sf::Color::White, 100, window.getSize().y - 300);

			if (InputManager::Instance()->KeyPress("Up_1"))   choice = (choice == 0) ? 0 : choice - 1;
			if (InputManager::Instance()->KeyPress("Down_1")) choice = (choice == 2) ? 2 : choice + 1;
			if (InputManager::Instance()->KeyPress("Enter")) {
				switch (choice) {
				case 0:scene = singleMulti; break;
				case 1:scene = settings; break;
				case 2: window.close(); break;
				}
				choice = 0;
			}
			break;
		}
		case tutorial:
		{
			static int step = 1;
			displayTutorial(window, TextRenderer.getFont(), step);
			TextRenderer.displayText(window, "Press ENTER to continue", 1, 30, sf::Color::White, window.getSize().x/2, window.getSize().y - 110);
			if (InputManager::Instance()->KeyPress("Enter")) step++;
			if (step > 5) {
				scene = settings;
				step = 1;
			}
			break;
		}

		case settings:
		{
			TextRenderer.displayText(window, "something is happening somewhere", 1, 60, sf::Color::White, window.getSize().x / 2, 100);

			static int choice = 0;
			std::string Cheat = (cheat) ? "Enabled" : "Disabled";
			std::vector<std::string> choices = { "BGM Volume : <" + std::to_string(bgmVolume) + "%>",
												 "SFX Volume : <" + std::to_string(sfxVolume) + "%>",
												 "Cheat : < " + Cheat + " >",
												 "Tutorial",
												 "Credit",
												 "Back to main menu"};
			TextRenderer.displayMultipleChoice(window, choices, choice, 40, sf::Color::Cyan, sf::Color::White, 100, window.getSize().y - 450);


			// ACTION
			// moving between choices
			if (InputManager::Instance()->KeyPress("Up_1")) choice = (choice == 0) ? 0 : choice - 1;
			if (InputManager::Instance()->KeyPress("Down_1")) choice = (choice == 5) ? 5 : choice + 1;

			// Decrease / disable
			if (InputManager::Instance()->KeyPress("Left_1")) {
				switch (choice) {
				case 0:bgmVolume = (bgmVolume == 0) ? 0 : bgmVolume - 5; break; 
				case 1:sfxVolume = (sfxVolume == 0) ? 0 : sfxVolume - 5; sounds::playShootSound(sfxVolume); break;
				case 2:cheat = false; window.create(sf::VideoMode(1280, 720), "Platypus Scuffed Edition", sf::Style::Titlebar | sf::Style::Close); window.setFramerateLimit(60);  break;
				}
			}

			// Increase / enable
			if (InputManager::Instance()->KeyPress("Right_1")) {
				switch (choice) {
				case 0:bgmVolume = (bgmVolume == 100) ? 100 : bgmVolume + 5; break;
				case 1:sfxVolume = (sfxVolume == 100) ? 100 : sfxVolume + 5; sounds::playShootSound(sfxVolume); break;
				case 2:cheat = true; window.create(sf::VideoMode(1280, 720), "Platypus Scuffed Edition", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize); window.setFramerateLimit(60);  break;
				}
			}

			// scene changes
			if (InputManager::Instance()->KeyPress("Enter")) {
				switch (choice) {
				case 3:scene = tutorial; break;
				case 4:scene = credits; break;
				case 5:scene = start; break;
				}
				choice = 0;
			}
			break;
		}

		case credits:
		{
			static float positionY = window.getSize().y;
			sfe::RichText text = displayCredit(TextRenderer.getFont(), 100, positionY);
			window.draw(text);
			positionY -= 5;
			if (positionY < (- 1500)) {
				scene = settings;
				positionY = 800;
			}
			break;
		}

		case singleMulti:
		{
			static int choice;
			std::vector<std::string> choices = { "Singleplayer", "Multiplayer" };
			TextRenderer.displayText(window, "something is happening somewhere", 1, 50, sf::Color::White, window.getSize().x/2, 100);
			TextRenderer.displayText(window, "Choose your gamemode: ", 0, 40, sf::Color::White, 100, window.getSize().y - 220);
			TextRenderer.displayMultipleChoice(window, choices, choice, 40, sf::Color::Cyan, sf::Color::White, 100, window.getSize().y - 180);

			// moving between choices
			if (InputManager::Instance()->KeyPress("Up_1"))   choice = 0;
			if (InputManager::Instance()->KeyPress("Down_1")) choice = 1;
			if (InputManager::Instance()->KeyPress("Enter")){
				if (choice == 0) {
					player* Player_1 = new player(1, "gameplay_player.png", 100, static_cast<float>(window.getSize().y) * 1/2, 0, 0);
				}
				if (choice == 1) {
					player* Player_1 = new player(1, "gameplay_player.png", 100, static_cast<float>(window.getSize().y) * 1/3, 0, 0);
					player* Player_2 = new player(2, "gameplay_player.png", 100, static_cast<float>(window.getSize().y) * 2/3, 0, 0);
				}
				scene = play;
				choice = 0;
			}
			break;
		}

		case transition:
		{
			if(level == -1){ //lOSE
				TextRenderer.displayText(window, "YOU LOSE :(\nBETTER LUCK NEXT TIME", 0, 40, sf::Color::White, 100, 100);
				TextRenderer.displayText(window, "Your last score is " + std::to_string(currentPoint), 0, 40, sf::Color::White, 100, 200);
				TextRenderer.displayText(window, "Press Enter to back to main menu", 1, 40, sf::Color::White, window.getSize().x/2, window.getSize().y - 60);
				if (InputManager::Instance()->KeyPress("Enter")) {
					player::clearObject();
					enemy::clearObject();
					level = 0;
					currentPoint = 0;
					scene = start;
					endless = false;
				}
			}
			else if (level > 2) { //WIN
				static int choice = 0;
				TextRenderer.displayText(window, "CONGRATULATION :)\nYOU'RE THE WINNER", 0, 40, sf::Color::White, 100, 120);
				TextRenderer.displayText(window, "Your final score is " + std::to_string(currentPoint), 0, 40, sf::Color::White, 100, 210);
				std::vector<std::string>choices = {"ENDLESS MODE", "Main menu"};
				TextRenderer.displayMultipleChoice(window, choices, choice, 40, sf::Color::Cyan, sf::Color::White, 100, window.getSize().y - 180);

				if (InputManager::Instance()->KeyPress("Up_1"))   choice = 0;
				if (InputManager::Instance()->KeyPress("Down_1")) choice = 1;
				if (InputManager::Instance()->KeyPress("Enter")) {
					if (choice == 0) {
						endless = true;
						scene = play;
					}
					else if (choice == 1) {
						player::clearObject();
						level = 0;
						currentPoint = 0;
						scene = start;
					}
				}
			}
			else { //NEXT LEVEL
				TextRenderer.displayText(window, "LEVEL " + std::to_string(level + 1), 0, 40, sf::Color::Cyan, 100, 100);
				TextRenderer.displayText(window, "Press ENTER to continue", 1, 30, sf::Color::White, window.getSize().x/2, window.getSize().y - 60);
				if (InputManager::Instance()->KeyPress("Enter")) {
					level++;
					generateEnemy = true;
					scene = play;
				}
			}
			break;
		}

		case pause:
		{
			static int choice = 0;
			TextRenderer.displayText(window, "PAUSE", 0, 40, sf::Color::White, 100, 100);
			std::vector<std::string> choices = { "Resume", "Rage quit" };
			TextRenderer.displayMultipleChoice(window, choices, choice, 40, sf::Color::Cyan, sf::Color::White, 100, window.getSize().y - 160);

			if (InputManager::Instance()->KeyPress("Up_1"))   choice = 0;
			if (InputManager::Instance()->KeyPress("Down_1")) choice = 1;
			if (InputManager::Instance()->KeyPress("Enter")) {
				switch (choice) {
				case 0:
					scene = play;  
					break;
				case 1:
					scene = start; 
					level = 0;
					player::clearObject();
					break;
				}
			}
			break;
		}

		case play:
		{
			//pause the game if the window lost its focus
			if (!window.hasFocus()) {
				scene = pause;
			}

			if (endless) {
				static float difficulty = 3;
				static int counts = 1; // just for id
				if (elapsed.getElapsedTime().asSeconds() > 10 || enemy::getEnemyMap()->size() < 3) {
					//generate enemy
					int counter = 0;
					for (int i = counts; i < counts + ((getRandomFloat(window.getSize().x / 288, window.getSize().y / 176)) * difficulty); i++) {
						enemy* Enemy = new enemy(i, "gameplay_enemy.png", getRandomFloat(400, window.getSize().x), getRandomFloat(0, window.getSize().y), getRandomFloat(-1000, 1000), getRandomFloat(-1000, 1000));
						animate::play("gameplay_spawn.png", 4, 4, sf::Vector2f(Enemy->getPosition().x, Enemy->getPosition().y));
						counter++;
					}
					counts += counter;
					if (counts == 999)counts = 1;
					difficulty += 0.5;
					elapsed.restart();
				}
			}


			//level mechanics and enemy object creation
			if (generateEnemy) {
				bullet::clearObject();
				switch (level) {
				case 1:
				{
					for (int i = 1; i < getRandomFloat(window.getSize().x / 288, window.getSize().y / 176); i++) {
						enemy* Enemy = new enemy(i, "gameplay_enemy.png", getRandomFloat(400, window.getSize().x), getRandomFloat(0, window.getSize().y), getRandomFloat(-500, 500), getRandomFloat(-500, 500));
						animate::play("gameplay_spawn.png", 4, 4, sf::Vector2f(Enemy->getPosition().x, Enemy->getPosition().y));
					}
					break;
				}
				case 2:
				{
					for (int i = 1; i < getRandomFloat((window.getSize().x / 288), (window.getSize().y / 176)) * 2; i++) {
						enemy* Enemy = new enemy(i, "gameplay_enemy.png", getRandomFloat(400, window.getSize().x), getRandomFloat(0, window.getSize().y), getRandomFloat(-750, 750), getRandomFloat(-750, 750));
						animate::play("gameplay_spawn.png", 4, 4, sf::Vector2f(Enemy->getPosition().x, Enemy->getPosition().y));
					}
					break;
				}
				case 3:
				{
					for (int i = 1; i < getRandomFloat((window.getSize().x / 288), (window.getSize().y / 176)) * 3; i++) {
						enemy* Enemy = new enemy(i, "gameplay_enemy.png", getRandomFloat(400, window.getSize().x), getRandomFloat(0, window.getSize().y), getRandomFloat(-1000, 1000), getRandomFloat(-1000, 1000));
						animate::play("gameplay_spawn.png", 4, 4, sf::Vector2f(Enemy->getPosition().x, Enemy->getPosition().y));
					}
					break;
				}
				}
				generateEnemy = false;
				break;
			}

			//level up when the enemy is 0
			if (enemy::getEnemyMap()->empty() && !endless) {
				scene = transition;
				break;
			}

			//lose when the player is 0
			if (player::getPlayerMap()->empty()) {
				level = -1;
				scene = transition;
				break;
			}

			if (InputManager::Instance()->KeyPress("Spacebar")) {
				if (player::getObjectPtr(object::Type::player_obj + 1) != NULL) player::getObjectPtr(object::Type::player_obj + 1)->setVelocity(0, 0);
				if (player::getObjectPtr(object::Type::player_obj + 2) != NULL) player::getObjectPtr(object::Type::player_obj + 2)->setVelocity(0, 0);
				scene = pause;
			}

			//enemy's attack algorithm
			std::unordered_map<int, enemy*>* enemyMap = enemy::getEnemyMap();
			std::unordered_map<int, player*>* playerMap = player::getPlayerMap();
			for (auto enemy_object = enemyMap->begin(); enemy_object != enemyMap->end(); enemy_object++) {
				enemy* Enemy = enemy_object->second;
				for (auto player_object = playerMap->begin(); player_object != playerMap->end(); player_object++) {
					player* Player = player_object->second;
					if ((Enemy->getPosition().y < Player->getPosition().y + 5) &&
						(Enemy->getPosition().y > Player->getPosition().y - 5)) {
						Enemy->shoot(sfxVolume);
					}
				}
			}

			TextRenderer.displayText(window, "Score : " + std::to_string(currentPoint), 0, 40, sf::Color::White, 30, 30);
			break;
		}
		}

		/*VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV PLAYER'S CONTROL VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV*/
		if ((scene != pause) || (scene == pause && cheat)) {
			//FIRST PLAYER
			if (player::getObjectPtr(object::Type::player_obj + 1) != NULL) {
				static bool cooldown = false;
				player* player_1 = player::getObjectPtr(object::Type::player_obj + 1);
				player_1->bulletBar.draw(window, player_1->getBulletRemain(), sf::Vector2f(150, window.getSize().y - 50));
				TextRenderer.displayText(window, "player_1's bullet", 1, 20, sf::Color::Black, 150, window.getSize().y - 50);
				if (InputManager::Instance()->KeyDown("Up_1"))
					player_1->thrustUp();
				if (InputManager::Instance()->KeyDown("Down_1"))
					player_1->thrustDown();
				if (InputManager::Instance()->KeyDown("Right_1") && player_1->getBulletRemain() >= 1 && !cooldown)
					player_1->shoot(sfxVolume);
				if (InputManager::Instance()->KeyPress("Left_1")) 
					cooldown = true;
				if (cooldown) {
					if (player_1->getBulletRemain() >= MAX_PLAYER_BULLET) {
						cooldown = false;
					}
					else
						player_1->reloadBullet(1);
				}
			}

			//SECOND PLAYER
			if (player::getObjectPtr(object::Type::player_obj + 2) != NULL) {
				bool cooldown = false;
				player* player_2 = player::getObjectPtr(object::Type::player_obj + 2);
				player_2->bulletBar.draw(window, player_2->getBulletRemain(), sf::Vector2f(window.getSize().x - 150, window.getSize().y - 50));
				TextRenderer.displayText(window, "player_2's bullet", 1, 20, sf::Color::Black, window.getSize().x - 150, window.getSize().y - 50);
				if (InputManager::Instance()->KeyDown("Up_2"))
					player_2->thrustUp();
				if (InputManager::Instance()->KeyDown("Down_2"))
					player_2->thrustDown();
				if (InputManager::Instance()->KeyDown("Right_2") && player_2->getBulletRemain() >= 1 && !cooldown)
					player_2->shoot(sfxVolume);
				if (InputManager::Instance()->KeyPress("Left_2"))
					cooldown = true;
				if (cooldown) {
					if (player_2->getBulletRemain() >= MAX_PLAYER_BULLET) {
						cooldown = false;
					}
					else {
						player_2->reloadBullet(1);
					}
				}
			}
		}
		/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ PLAYER'S CONTROL ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		window.display();
		window.clear();
		InputManager::Instance()->Update();
	}
	return 0;
}