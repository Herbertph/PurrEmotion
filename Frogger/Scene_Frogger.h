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
#include <string>
#include <vector>

struct TimedText {
	std::string text;
	sf::Time startTime;
	sf::Time endTime;
};

class Scene_Frogger : public Scene {
private:
	std::vector<TimedText> timedTexts; // Lista de textos com tempos
	sf::Text displayText; // O texto SFML que será usado para renderização
	sf::Font font; // A fonte do texto
	size_t currentTextIndex = 0; // Índice do texto atual
	size_t currentCharIndex = 0; // Índice do caractere atual no efeito de máquina de escrever
	sf::Time textEffectTime = sf::seconds(0.05); // Intervalo de tempo para o efeito de máquina de escrever
	sf::Time sinceLastUpdate = sf::Time::Zero; // Controle de tempo para o efeito de máquina de escrever
	size_t charIndex = 0; // Índice do próximo caractere a ser exibido
	sf::Time lastCharUpdateTime; // Quando o último caractere foi adicionado ao texto visível
	sf::Time textStartTime;


	sf::Vector2f velocity;
	bool onSafeEntity;


	sPtrEntt m_player{ nullptr };
	sPtrEntt m_invisibleCollisionBox{ nullptr };
	sPtrEntt m_interactiveBox{ nullptr };
	sf::View m_worldView;
	sf::FloatRect m_worldBounds;
	sf::Time m_elapsedTime = sf::Time::Zero;
	sf::Font m_font;
	sf::RectangleShape textBackground;



	bool m_drawTextures{ true };
	bool m_drawAABB{ false };
	bool m_drawGrid{ false };
	bool m_boxCreated[3] = { false, false, false };

	int m_score{ 0 };
	std::unordered_map<std::string, bool> m_laneCrossed;
	std::unordered_map<int, sf::Time> m_boxLifeTime;
	std::vector<std::shared_ptr<Entity>> m_interactiveBoxes;


	void sMovement(sf::Time dt);
	void sCollisions(sf::Time dt);
	void sUpdate(sf::Time dt);
	void sAnimation(sf::Time dt);
	void applyGravity(sf::Time dt);
	void onEnd();
	
	


	void playerMovement();
	void adjustPlayerPosition();
	void spawnInvisibleCollisionBox();
	void checkPlayerGroundStatus();
	float getGroundLevelAt(float x);
	bool isOnInvisibleBox() const;
	void spawnInteractiveBoxes(int boxIndex);
	void removeInteractiveBoxes(int boxIndex);
	void initTexts();
	void secondText();
	void thirdText();
	void finishText();
	bool conditionalTextAdded = false;
	bool conditionalTextAddedSecond = false;
	bool conditionalTextAddedThird = false;
	bool conditionalTextAddedFinish = false;
	bool checkBox0State();
	bool checkBox1State();
	bool checkBox2State();
	
	void spawnPlayer(sf::Vector2f pos);

	bool checkCollision(Entity& entity1, Entity& entity2);
	


	
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
