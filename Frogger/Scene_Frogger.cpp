//
// Created by David Burchill on 2023-09-27.
// Updated by Herbert Parasio on 2023-12-14

#include <fstream>
#include <iostream>

#include "Scene_Frogger.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include <random>

namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}



#pragma region Constructor and Initialization
Scene_Frogger::Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_worldView(gameEngine->window().getDefaultView()) {

	loadLevel(levelPath);
	registerActions();

	auto pos = m_worldView.getSize();

	// Spawn frog in middle of first row
	pos.x = pos.x / 2.f;
	pos.y -= 20.f;


	// Spawn lanes
	spawnLane("moveLeft", { 480.f + 60.f, 600.f - 60.f }, { -40.f, 0.f }, 3, "raceCarL", { 30.f, 15.f });
	spawnLane("moveRight", { 0.f - 60.f, 600.f - 100.f }, { 60.f, 0.f }, 3, "tractor", { 30.f, 15.f });
	spawnLane("moveLeft", { 480.f + 60.f, 600.f - 140.f }, { -50.f, 0.f }, 3, "car", { 30.f, 15.f });
	spawnLane("moveRight", { 0.f - 30.f, 600.f - 180.f }, { 60.f, 0.f }, 3, "raceCarR", { 30.f, 15.f });
	spawnLane("moveLeft", { 480.f + 180.f, 600.f - 220.f }, { -50.f, 0.f }, 2, "truck", { 60.f, 15.f });
	spawnLane("moveLeft", { 480.f + 50.f, 600.f - 300.f }, { -50.f, 0.f }, 4, "3turtles", { 90.f, 15.f });
	spawnLane("moveRight", { 0.f - 20.f, 600.f - 340.f }, { 60.f, 0.f }, 2, "tree1", { 93.f, 15.f });
	spawnLane("moveRight", { 0.f - 150.f, 600.f - 380.f }, { 100.f, 0.f }, 2, "tree2", { 200.f, 15.f });
	spawnLane("moveLeft", { 480.f + 40.f, 600.f - 420.f }, { -50.f, 0.f }, 3, "2turtles", { 60.f, 15.f });
	spawnLane("moveRight", { 0.f - 40.f, 600.f - 460.f }, { 100.f, 0.f }, 3, "tree1", { 93.f, 15.f });
	flowerLane();

	spawnPlayer(pos);

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(50);

	//m_hasCrossedLane = false;
	m_laneCrossed["lane1"] = false;
	m_laneCrossed["lane2"] = false;
	m_laneCrossed["lane3"] = false;
	m_laneCrossed["lane4"] = false;
	m_laneCrossed["lane5"] = false;
	m_laneCrossed["lane6"] = false;
	m_laneCrossed["lane7"] = false;
	m_laneCrossed["lane8"] = false;
	m_laneCrossed["lane9"] = false;
	m_laneCrossed["lane10"] = false;

}

void Scene_Frogger::loadLevel(const std::string& path) {
	std::ifstream config(path);
	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Bkg") {
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;
			auto e = m_entityManager.addEntity("bkg");

			// for background, no textureRect its just the whole texture
			// and no center origin, position by top left corner
			// stationary so no CTransfrom required.
			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
		}
		else if (token[0] == '#') {
			std::cout << token;
		}

		config >> token;
	}
	config.close();
}

void Scene_Frogger::registerActions() {
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
}

void Scene_Frogger::spawnPlayer(sf::Vector2f pos) {
	m_lillyPadsOccupied = 0;

	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(15.f, 15.f));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
	m_player->addComponent<CState>("grounded");
}

void Scene_Frogger::spawnLane(const std::string& tag, sf::Vector2f initialPos, sf::Vector2f velocity, int entityCount, const std::string& animationName, sf::Vector2f boundingBoxSize) {
	for (int i = 0; i < entityCount; ++i) {


		auto entity = m_entityManager.addEntity(tag);
		auto& tfm = entity->addComponent<CTransform>(initialPos, velocity);

		entity->addComponent<CBoundingBox>(boundingBoxSize);
		entity->addComponent<CAnimation>(Assets::getInstance().getAnimation(animationName));
		initialPos.x += velocity.x < 0 ? -140.f : 140.f;
	}
}

void Scene_Frogger::flowerLane()
{
	sf::Vector2f pos{ 0.f + 38.f, 600.f - 500.f };
	for (int i{ 0 }; i < 5; ++i) {
		auto car = m_entityManager.addEntity("lillyPad");
		auto& tfm = car->addComponent<CTransform>(pos);
		car->addComponent<CBoundingBox>(sf::Vector2f{ 30.f, 30.f });
		car->addComponent<CAnimation>(Assets::getInstance().getAnimation("lillyPad"));
		pos.x += 100.f;
	}
}

void Scene_Frogger::spawnBugIcon()
{

	std::vector<sf::Vector2f> lillyPadPositions = {
		{ 0.f + 38.f, 600.f - 500.f },
		{ 0.f + 138.f, 600.f - 500.f },
		{ 0.f + 238.f, 600.f - 500.f },
		{ 0.f + 338.f, 600.f - 500.f },
		{ 0.f + 438.f, 600.f - 500.f }
	};

	std::uniform_int_distribution<std::size_t> dist(0, lillyPadPositions.size() - 1);
	std::size_t randomIndex = dist(rng);

	sf::Vector2f randomPos = lillyPadPositions[randomIndex];

	auto car = m_entityManager.addEntity("bugIcon");
	auto& tfm = car->addComponent<CTransform>(randomPos);
	car->addComponent<CBoundingBox>(sf::Vector2f{ 30.f, 30.f });
	car->addComponent<CAnimation>(Assets::getInstance().getAnimation("bugIcon"));

	// Reset the timer
	bugIconTimer.restart();
}

#pragma endregion

#pragma region Configuration and Support
void Scene_Frogger::init(const std::string& path) {
}

void Scene_Frogger::resetEntityPosition(std::shared_ptr<Entity> entity) {
	auto& tfm = entity->getComponent<CTransform>();
	float windowWidth = 480.f;

	if ((entity->getTag() == "moveLeft" && tfm.pos.x < -50.f) ||
		(entity->getTag() == "tree" && tfm.pos.x > windowWidth + 100.f)) {
		tfm.pos.x = windowWidth + 50.f;
	}

	if (entity->getTag() == "moveRight" && tfm.pos.x > windowWidth + 50.f) {
		tfm.pos.x = -50.f;
	}
}

void Scene_Frogger::updateFlowerState(std::shared_ptr<Entity>& lillyPad) {
	m_player->getComponent<CTransform>().pos = lillyPad->getComponent<CTransform>().pos;
	lillyPad->addComponent<CState>().state = "hasFrog";

	// Criar uma nova entidade 'player' para representar o sapo no lillyPad
	auto newPlayer = m_entityManager.addEntity("player");
	newPlayer->addComponent<CTransform>(lillyPad->getComponent<CTransform>().pos);
	newPlayer->addComponent<CBoundingBox>(sf::Vector2f(15.f, 15.f));
	newPlayer->addComponent<CInput>();
	newPlayer->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
	newPlayer->addComponent<CState>("grounded");

	m_player = newPlayer; // Atualizar referência do jogador para a nova entidade
}

void Scene_Frogger::updateScoreAndOccupiedCount() {
	m_lillyPadsOccupied++;
	Assets::getInstance().scoreUp(20);

	if (m_lillyPadsOccupied == 5) {
		Assets::getInstance().scoreUp(100); // Bônus por ocupar todos os lillyPads
	}
}
#pragma endregion

#pragma region Update Game State

void Scene_Frogger::update(sf::Time dt) {
	sUpdate(dt);
	if (m_player->getComponent<CState>().state == "dead" && m_player->getComponent<CAnimation>().animation.hasEnded()) {
		respawnPlayer();
	}
}

void Scene_Frogger::sUpdate(sf::Time dt) {
	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();

	if (m_isPaused)
		return;

	updateScoreText();
	updateLivesText();
	sAnimation(dt);
	sMovement(dt);
	adjustPlayerPosition();
	checkPlayerState();
	sCollisions(dt);

	// Check if the bugIcon duration has passed
	if (bugIconTimer.getElapsedTime() >= bugIconDuration) {
		auto bugIcon = m_entityManager.getEntities("bugIcon");
		// Remove the CAnimation component from bugIcon
		for (auto& bug : bugIcon) {
			bug->removeComponent<CAnimation>();
		}

		// Spawn a new bugIcon in a random position
		spawnBugIcon();
	}
}

void Scene_Frogger::sMovement(sf::Time dt) {
	playerMovement();

	for (auto e : m_entityManager.getEntities()) {
		if (e->hasComponent<CInput>()) continue;

		if (e->hasComponent<CTransform>()) {
			auto& tfm = e->getComponent<CTransform>();
			tfm.pos += tfm.vel * dt.asSeconds();
			tfm.angle += tfm.angVel * dt.asSeconds();

			resetEntityPosition(e);
		}
	}
}

void Scene_Frogger::playerMovement() {
	// no movement if player is dead
	if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
		return;

	auto& dir = m_player->getComponent<CInput>().dir;
	auto& pos = m_player->getComponent<CTransform>().pos;

	if (dir & CInput::UP) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
		pos.y -= 40.f;
	}
	if (dir & CInput::DOWN) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
		pos.y += 40.f;
	}

	if (dir & CInput::LEFT) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("left"));
		pos.x -= 40.f;
	}

	if (dir & CInput::RIGHT) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("right"));
		pos.x += 40.f;
	}

	if (dir != 0) {
		SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
		dir = 0;
	}
}

void Scene_Frogger::sCollisions(sf::Time dt) {
	adjustPlayerPosition();

	if (m_player->getComponent<CState>().state == "dead") {
		return;
	}

	bool onSafeEntity = false;
	auto& playerTransform = m_player->getComponent<CTransform>();



	for (auto& bug : m_entityManager.getEntities("bugIcon")) {
		if (checkCollision(*m_player, *bug)) {
			Assets::getInstance().scoreUp(100);

			bug->removeComponent<CAnimation>();
			bugIconTimer.restart();
		}
	}

	for (auto& entity : m_entityManager.getEntities()) {
		if (checkCollision(*m_player, *entity)) {
			const auto& entityTag = entity->getTag();
			const auto& entityTransform = entity->getComponent<CTransform>();


			if ((entityTag == "moveLeft" || entityTag == "moveRight" || entityTag == "tree") &&
				playerTransform.pos.y < 301.f && playerTransform.pos.y > 139.f) {
				onSafeEntity = true;
				playerTransform.pos.x += entityTransform.vel.x * dt.asSeconds();
			}

			if ((entityTag == "moveLeft" || entityTag == "moveRight") && playerTransform.pos.y < 541.f && playerTransform.pos.y > 379.f) {
				onPlayerCollision();
				return;
			}
		}

	}

	if (!onSafeEntity && playerTransform.pos.y < 301.f && playerTransform.pos.y > 139.f) {
		playerDeath();
		return;
	}

	for (auto& lillyPad : m_entityManager.getEntities("lillyPad")) {
		if (checkCollision(*m_player, *lillyPad)) {
			onPlayerCollisionFlower();
			return;
		}
	}

	if (playerTransform.pos.x <= 20.f || playerTransform.pos.x >= 460.f) {
		playerDeath();
	}
}

void Scene_Frogger::onPlayerCollision() {
	SoundPlayer::getInstance().play("death", m_player->getComponent<CTransform>().pos);
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
	Assets::getInstance().lifeDown(1);
	updateLivesText();

	if (Assets::getInstance().lifeUp() == 0) {
		m_player->addComponent<CState>().state = "dead";
		MusicPlayer::getInstance().stop();
		onEnd();
	}
	else {
		m_player->addComponent<CState>().state = "dead";
	}
	if (m_player->getComponent<CTransform>().pos.y < 600.f - 260.f && m_player->getComponent<CTransform>().pos.y > 300.f) {
		bool onSafeEntity = false;

		for (auto& safeEntity : m_entityManager.getEntities()) {
			if (safeEntity->getTag() == "Tree1" || safeEntity->getTag() == "2Turtles" || safeEntity->getTag() == "3Turtles" || safeEntity->getTag() == "Tree2") {
				if (checkCollision(*m_player, *safeEntity)) {
					onSafeEntity = true;
					break;
				}
			}
		}

		if (!onSafeEntity) {
			playerDeath();
		}
	}

}

void Scene_Frogger::onPlayerCollisionFlower() {
	for (auto& lillyPad : m_entityManager.getEntities("lillyPad")) {
		if (!checkCollision(*m_player, *lillyPad)) continue;

		// Colisão com lillyPad que já tem sapo
		if (lillyPad->getComponent<CState>().state == "hasFrog") {
			Assets::getInstance().scoreDown(20);
			onPlayerCollision();
		}
		else {
			// Atualiza o estado do lillyPad e a pontuação
			updateFlowerState(lillyPad);
			updateScoreAndOccupiedCount();
		}

		respawnPlayer();
		break; // Sair do loop após tratar a colisão
	}
}

void Scene_Frogger::playerDeath() {

	SoundPlayer::getInstance().play("death", m_player->getComponent<CTransform>().pos);
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
	Assets::getInstance().lifeDown(1);
	updateLivesText();

	if (Assets::getInstance().lifeUp() == 0) {
		m_player->addComponent<CState>().state = "dead";
		MusicPlayer::getInstance().stop();
		onEnd();
	}
	else {

		m_player->addComponent<CState>().state = "dead";
	}
}

void Scene_Frogger::respawnPlayer() {
	auto pos = m_worldView.getSize();
	pos.x = pos.x / 2.f;
	pos.y -= 20.f;
	m_player->getComponent<CTransform>().pos = pos;

	// set the player to initial state 
	auto& playerState = m_player->getComponent<CState>().state;
	playerState = "grounded";

	// set the player to initial animation
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
}

bool Scene_Frogger::checkCollision(Entity& entity1, Entity& entity2) {
	if (entity1.hasComponent<CBoundingBox>() && entity2.hasComponent<CBoundingBox>()) {
		auto& box1 = entity1.getComponent<CBoundingBox>();
		auto& box2 = entity2.getComponent<CBoundingBox>();

		sf::FloatRect rect1(
			entity1.getComponent<CTransform>().pos.x - box1.size.x / 2.f,
			entity1.getComponent<CTransform>().pos.y - box1.size.y / 2.f,
			box1.size.x, box1.size.y);

		sf::FloatRect rect2(
			entity2.getComponent<CTransform>().pos.x - box2.size.x / 2.f,
			entity2.getComponent<CTransform>().pos.y - box2.size.y / 2.f,
			box2.size.x, box2.size.y);

		return rect1.intersects(rect2);
	}

	return false;
}

void Scene_Frogger::checkPlayerState() {
	auto& state = m_player->getComponent<CState>().state;
	if (state == "dead" && m_player->getComponent<CAnimation>().animation.hasEnded()) {
		respawnPlayer();
	}
}

#pragma endregion

#pragma region Renderings

sf::FloatRect Scene_Frogger::getViewBounds() {
	return sf::FloatRect();
}

void Scene_Frogger::sRender() {
	m_game->window().setView(m_worldView);
	drawBackground();
	drawEntities();
	drawUI();
}

void Scene_Frogger::drawBackground() {
	for (auto e : m_entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			m_game->window().draw(e->getComponent<CSprite>().sprite);
		}
	}
}

void Scene_Frogger::drawEntities() {
	for (auto& e : m_entityManager.getEntities()) {
		if (!e->hasComponent<CAnimation>()) continue;

		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		anim.getSprite().setPosition(tfm.pos);
		anim.getSprite().setRotation(tfm.angle);
		m_game->window().draw(anim.getSprite());

		if (m_drawAABB && e->hasComponent<CBoundingBox>()) {
			drawBoundingBox(e);
		}
	}
}

void Scene_Frogger::drawBoundingBox(std::shared_ptr<Entity> entity) {
	auto box = entity->getComponent<CBoundingBox>();
	sf::RectangleShape rect(sf::Vector2f{ box.size.x, box.size.y });
	centerOrigin(rect);
	rect.setPosition(entity->getComponent<CTransform>().pos);
	rect.setFillColor(sf::Color(0, 0, 0, 0));
	rect.setOutlineColor(sf::Color{ 0, 255, 0 });
	rect.setOutlineThickness(2.f);
	m_game->window().draw(rect);
}

void Scene_Frogger::drawUI() {
	m_game->window().draw(m_scoreText);
	m_game->window().draw(m_livesText);
}

#pragma endregion

#pragma region Endgame and Actions

void Scene_Frogger::onEnd() {
	m_game->changeScene("MENU", nullptr, false);
}

void Scene_Frogger::sDoAction(const Command& action) {
	// On Key Press
	if (action.type() == "START") {
		if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
		else if (action.name() == "QUIT") { m_game->quitLevel(); }
		else if (action.name() == "BACK") { m_game->backLevel(); }

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

		// Player control
		if (action.name() == "LEFT") { m_player->getComponent<CInput>().dir = CInput::LEFT; }
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
		else if (action.name() == "UP") { m_player->getComponent<CInput>().dir = CInput::UP; }
		else if (action.name() == "DOWN") { m_player->getComponent<CInput>().dir = CInput::DOWN; }
	}
	// on Key Release
	// the frog can only go in one direction at a time, no angles
	// use a bitset and exclusive setting.
	else if (action.type() == "END" && (action.name() == "LEFT" || action.name() == "RIGHT" || action.name() == "UP" ||
		action.name() == "DOWN")) {
		m_player->getComponent<CInput>().dir = 0;
	}
}

#pragma endregion

#pragma region UIUpdate and Others

void Scene_Frogger::updateScoreText() {
	m_font = Assets::getInstance().getFont("Arcade");
	m_scoreText.setString("Score  " + std::to_string(Assets::getInstance().getScore()));

	// Setting the score text properties
	m_scoreText.setPosition(20.f, 15.f);
	m_scoreText.setFont(m_font);
	m_scoreText.setCharacterSize(30);
	m_scoreText.setFillColor(sf::Color::Green);
}

void Scene_Frogger::updateLivesText() {
	m_font = Assets::getInstance().getFont("Arcade");
	m_livesText.setString("Lives  " + std::to_string(Assets::getInstance().lifeUp()));
	// Setting the lives text properties
	m_livesText.setPosition(320.f, 15.f);
	m_livesText.setFont(m_font);
	m_livesText.setCharacterSize(30);
	m_livesText.setFillColor(sf::Color::Red);
}

void Scene_Frogger::sAnimation(sf::Time dt) {
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities()) {
		// update all animations
		if (e->hasComponent<CAnimation>()) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);
			// do nothing if animation has ended
		}
	}
}

void Scene_Frogger::adjustPlayerPosition() {
	auto center = m_worldView.getCenter();
	sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


	auto left = center.x - viewHalfSize.x;
	auto right = center.x + viewHalfSize.x;
	auto top = center.y - viewHalfSize.y;
	auto bot = center.y + viewHalfSize.y;

	auto& player_pos = m_player->getComponent<CTransform>().pos;
	auto halfSize = sf::Vector2f{ 20, 20 };
	// keep player in bounds
	player_pos.x = std::max(player_pos.x, left + halfSize.x);
	player_pos.x = std::min(player_pos.x, right - halfSize.x);
	player_pos.y = std::max(player_pos.y, top + halfSize.y);
	player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

#pragma endregion





