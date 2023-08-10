#pragma once
#include "objects.h"
#include "bullet.h"

class player : public object {
protected:
	static std::unordered_map<int, player*> player_map;
	static int allBullet;
	int bulletFired = 0;
	float hp = 100;

public:
	player(int _object_id, std::string textureDir, float _positionX, float _positionY, float _velocityX, float _velocityY, float _gravity);
	static player* getObjectPtr(int id);
	void shoot();
	void resetBulletCount();
	int getBulletCount();
	void thrust();
	float getPlayerHp() const;
	void setPlayerHp(float _hp);
	void reducePlayerHp(float damage);
	
	static void deleteObject(int id);
	static void clearObject();
	static std::unordered_map<int, player*>* getPlayerMap();
	static void updateNDrawAllObject(double dt, sf::RenderWindow& window);
};