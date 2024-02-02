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

	spawnPlayer(pos);

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(50);

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


void Scene_Frogger::init(const std::string& path) {
}


#pragma region Update Game State

void Scene_Frogger::update(sf::Time dt) {
	sUpdate(dt);
	if (m_player->getComponent<CState>().state == "dead" && m_player->getComponent<CAnimation>().animation.hasEnded()) {
	}
}

void Scene_Frogger::sUpdate(sf::Time dt) {
	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();

	if (m_isPaused)
		return;

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
				return;
			}
		}

	}

	if (!onSafeEntity && playerTransform.pos.y < 301.f && playerTransform.pos.y > 139.f) {
		
		return;
	}

	for (auto& lillyPad : m_entityManager.getEntities("lillyPad")) {
		if (checkCollision(*m_player, *lillyPad)) {
		
			return;
		}
	}

	if (playerTransform.pos.x <= 20.f || playerTransform.pos.x >= 460.f) {
		
	}
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





