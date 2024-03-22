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


const float GRAVITY_SPEED = 100.f;


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

	spawnInvisibleCollisionBox();
	//spawnInteractiveBoxes();
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
	registerAction(sf::Keyboard::Space, "ACTIVATE");
}

void Scene_Frogger::spawnPlayer(sf::Vector2f pos) {
	

	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(20.f, 20.f));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
	m_player->addComponent<CState>("grounded");
}


void Scene_Frogger::init(const std::string& path) {
}



//Criacao de Remocao de caixas Interativas
void Scene_Frogger::update(sf::Time dt) {
	m_elapsedTime += dt;

	if (!m_boxCreated[0] && m_elapsedTime >= sf::seconds(5)) {
		spawnInteractiveBoxes(0);
		m_boxCreated[0] = true;
	}

	if (!m_boxCreated[1] && m_elapsedTime >= sf::seconds(10)) {
		spawnInteractiveBoxes(1);
		m_boxCreated[1] = true;
	}

	if (!m_boxCreated[2] && m_elapsedTime >= sf::seconds(15)) {
		spawnInteractiveBoxes(2);
		m_boxCreated[2] = true;
	}

	if (m_boxCreated[0] && m_elapsedTime >= sf::seconds(10)) {
		removeInteractiveBoxes(0);
		m_boxCreated[0] = false;
	}
	if (m_boxCreated[1] && m_elapsedTime >= sf::seconds(15)) {
		removeInteractiveBoxes(1);
		m_boxCreated[1] = false;
	}
	if (m_boxCreated[2] && m_elapsedTime >= sf::seconds(20)) {
		removeInteractiveBoxes(2);
		m_boxCreated[2] = false;
	}
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
	applyGravity(dt);
	adjustPlayerPosition();
	sCollisions(dt);
	
	m_elapsedTime += dt;

	

}

void Scene_Frogger::applyGravity(sf::Time dt) {
	auto& state = m_player->getComponent<CState>().state;
	if (state == "jumping") {
		
		auto& pos = m_player->getComponent<CTransform>().pos;
		auto& vel = m_player->getComponent<CTransform>().vel;
		vel.y += GRAVITY_SPEED * 0.1; 
		pos.y += vel.y * 0.1; 

		
		if (isOnGround()) {
			state = "grounded"; 
			vel.y = 0; 
		}
	}
}

bool Scene_Frogger::isOnGround() const {
	if (!m_player) return false; 

	auto& transform = m_player->getComponent<CTransform>();
	auto& boundingBox = m_player->getComponent<CBoundingBox>();

	float groundHeight = 500;

	if ((transform.pos.y + boundingBox.halfSize.y) > groundHeight) {
		return true;
	}


	return (transform.pos.y + boundingBox.halfSize.y) >= groundHeight;
}

void Scene_Frogger::checkGroundCollision() {
	if (!m_player) return; 

	auto& transform = m_player->getComponent<CTransform>();
	auto& boundingBox = m_player->getComponent<CBoundingBox>();

	float groundHeight = 500; 

	if ((transform.pos.y + boundingBox.halfSize.y) > groundHeight) {
		transform.pos.y = groundHeight - boundingBox.halfSize.y;
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
	auto& dir = m_player->getComponent<CInput>().dir;
	auto& pos = m_player->getComponent<CTransform>().pos;
	auto& vel = m_player->getComponent<CTransform>().vel; 
	auto& state = m_player->getComponent<CState>().state;

	
	if (dir & CInput::LEFT) {
		
		pos.x -= 3; 
		if (state == "grounded" || state == "jumping") { 
			m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("left"));
		}
	}
	if (dir & CInput::RIGHT) {
		
		pos.x += 3; 
		if (state == "grounded" || state == "jumping") { 
			m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("right"));
		}
	}

	if ((dir & CInput::UP) && state == "grounded") {
		state = "jumping";
		vel.y = -200; 
	}

	if (dir == 0 && state == "grounded") {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
	}
}

void Scene_Frogger::sCollisions(sf::Time dt) {
	auto& entities = m_entityManager.getEntities();
	for (auto& entity : entities) {
		if (entity->getTag() == "player") {
			auto& playerTransform = entity->getComponent<CTransform>();
			auto& playerBox = entity->getComponent<CBoundingBox>();

			for (auto& other : entities) {
				if (entity == other || !other->hasComponent<CBoundingBox>()) continue;

				if (other->getTag() == "invisibleCollisionBox" && checkCollision(*entity, *other)) {
					// Tratar as caixas invisíveis como chão
					entity->getComponent<CState>().state = "grounded";
					break; // Pode parar de verificar outras colisões se já encontrou chão
				}
				// if not touching anything, apply gravity
				else {
					entity->getComponent<CState>().state = "jumping";
				}

			}
		}
	}
}


float Scene_Frogger::getGroundLevelAt(float x) {
	return 500.0f; 
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

		// Verifica se as bounding boxes se intersectam
		bool collision = rect1.intersects(rect2);

		return collision;
	}

	// Retorna falso se uma das entidades não tiver o componente CBoundingBox
	return false;
}



sf::FloatRect Scene_Frogger::getViewBounds() {
	return sf::FloatRect();
}

void Scene_Frogger::sRender() {
	m_game->window().setView(m_worldView);
	drawBackground();
	drawEntities();
	if (m_drawAABB) {
		for (auto& e : m_entityManager.getEntities()) {
			if (e->hasComponent<CBoundingBox>()) {
				drawBoundingBox(e);
			}
		}
	}
	
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
	if (action.type() == "START" && action.name() == "ACTIVATE") {
		for (auto& entity : m_entityManager.getEntities()) {
			// Certifique-se de que a entidade é uma caixa interativa
			if (entity->getTag() == "interactiveBox") {
				if (entity->hasComponent<CState>() && entity->getComponent<CState>().state == "inactive") {
					if (checkCollision(*m_player, *entity)) {
						// Ativa a caixa
						entity->getComponent<CState>().state = "active";
						std::cout << "Caixa interativa ativada: " << &entity << std::endl;
					}
				}
			}
		}
	}
}



void Scene_Frogger::sAnimation(sf::Time dt) {
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities()) {
		// update all animations
		if (e->hasComponent<CAnimation>()) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);
		}
	}
}

void Scene_Frogger::adjustPlayerPosition() {
	auto center = m_worldView.getCenter();
	sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


	auto left = 25;
	auto right = 975;
	auto top = center.y - viewHalfSize.y;
	auto bot = 500;

	auto& player_pos = m_player->getComponent<CTransform>().pos;
	auto halfSize = sf::Vector2f{ 20, 20 };
	// keep player in bounds
	player_pos.x = std::max(player_pos.x, left + halfSize.x);
	player_pos.x = std::min(player_pos.x, right - halfSize.x);
	player_pos.y = std::max(player_pos.y, top + halfSize.y);
	player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

void Scene_Frogger::spawnInvisibleCollisionBox() {

	sf::Vector2f viewSize = m_worldView.getSize();


	//left
	m_invisibleCollisionBox = m_entityManager.addEntity("invisibleCollisionBox");
	m_invisibleCollisionBox->addComponent<CTransform>(sf::Vector2f(110.f, 370.f));
	m_invisibleCollisionBox->addComponent<CBoundingBox>(sf::Vector2f(135.f, 1.f));
	m_invisibleCollisionBox->addComponent<CState>("grounded");

	//right
	m_invisibleCollisionBox = m_entityManager.addEntity("invisibleCollisionBox");
	m_invisibleCollisionBox->addComponent<CTransform>(sf::Vector2f(910.f, 380.f));
	m_invisibleCollisionBox->addComponent<CBoundingBox>(sf::Vector2f(115.f, 1.f));
	m_invisibleCollisionBox->addComponent<CState>("grounded");

	//bed
	m_invisibleCollisionBox = m_entityManager.addEntity("invisibleCollisionBox");
	m_invisibleCollisionBox->addComponent<CTransform>(sf::Vector2f(505.f, 350.f));
	m_invisibleCollisionBox->addComponent<CBoundingBox>(sf::Vector2f(220.f, 1.f));
	m_invisibleCollisionBox->addComponent<CState>("grounded");

	//drawn a line in the middle of initial position of the player
	m_invisibleCollisionBox = m_entityManager.addEntity("invisibleCollisionBox");
	m_invisibleCollisionBox->addComponent<CTransform>(sf::Vector2f(480.f, 480.f));
	m_invisibleCollisionBox->addComponent<CBoundingBox>(sf::Vector2f(1000.f, 20.f));
	m_invisibleCollisionBox->addComponent<CState>("grounded");

}


void Scene_Frogger::spawnInteractiveBoxes(int boxIndex) {
	if (boxIndex >= m_interactiveBoxes.size()) {
		m_interactiveBoxes.resize(boxIndex + 1, nullptr);
	}
	if (m_interactiveBoxes[boxIndex] != nullptr) {
		return; 
	}

	std::shared_ptr<Entity> box = m_entityManager.addEntity("interactiveBox");

	switch (boxIndex) {
	case 0: 
		box->addComponent<CTransform>(sf::Vector2f(110.f, 370.f));
		box->addComponent<CBoundingBox>(sf::Vector2f(135.f, 100.f));
		break;
	case 1: 
		box->addComponent<CTransform>(sf::Vector2f(505.f, 320.f));
		box->addComponent<CBoundingBox>(sf::Vector2f(50.f, 50.f));
		break;
	case 2: 
		box->addComponent<CTransform>(sf::Vector2f(910.f, 380.f));
		box->addComponent<CBoundingBox>(sf::Vector2f(115.f, 100.f));
		break;
	}

	box->addComponent<CState>("inactive"); 
	m_interactiveBoxes[boxIndex] = box;
}

void Scene_Frogger::removeInteractiveBoxes(int boxIndex) {
	if (boxIndex < m_interactiveBoxes.size() && m_interactiveBoxes[boxIndex] != nullptr) {
		m_interactiveBoxes[boxIndex]->getComponent<CTransform>().pos = sf::Vector2f(-1000, -1000);
		m_interactiveBoxes[boxIndex] = nullptr;
	}
}