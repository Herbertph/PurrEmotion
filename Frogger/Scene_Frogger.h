// Scene_Frogger.h
// Updated by Herbert Parasio on 2023-12-14

#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "Scene.h"
#include "GameEngine.h"
#include "Entity.h"

class Scene_Frogger : public Scene {
private:
	sf::Vector2f velocity;
	bool onSafeEntity;


	sPtrEntt m_player{ nullptr };
	sf::View m_worldView;
	sf::FloatRect m_worldBounds;


	sf::Text m_scoreText;
	sf::Text m_livesText;
	sf::Font m_font;


	sf::Clock gapTimer;
	sf::Clock bugIconTimer;
	sf::Time bugIconDuration = sf::seconds(5.0f);


	bool m_drawTextures{ true };
	bool m_drawAABB{ false };
	bool m_drawGrid{ false };


	int m_score{ 0 };
	int m_lillyPadsOccupied = 0;
	std::unordered_map<std::string, bool> m_laneCrossed;


	void sMovement(sf::Time dt);
	void sCollisions(sf::Time dt);
	void sUpdate(sf::Time dt);
	void sAnimation(sf::Time dt);
	void applyGravity(sf::Time dt);
	void onEnd();


	void playerMovement();
	void adjustPlayerPosition();
	
	
	void spawnPlayer(sf::Vector2f pos);

	bool checkCollision(Entity& entity1, Entity& entity2);
	

	void drawUI();
	
	void drawBackground();
	void drawEntities();
	void drawBoundingBox(std::shared_ptr<Entity>);
	bool isOnGround() const;
	void checkGroundCollision();

	void registerActions();

	void init(const std::string& path);
	void loadLevel(const std::string& path);
	sf::FloatRect getViewBounds();

public:
	Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath);
	void update(sf::Time dt) override;
	void sDoAction(const Command& command) override;
	void sRender() override;
};

#endif //BREAKOUT_SCENE_BREAKOUT_H
