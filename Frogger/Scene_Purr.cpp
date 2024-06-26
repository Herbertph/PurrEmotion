//
// Created by David Burchill on 2023-09-27.
// Updated by Herbert Parasio on 2023-12-14

#include <fstream>
#include <iostream>

#include "Scene_Purr.h"
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
const float GRAVITY_SPEED = 150.f;

#pragma region Constructor and Initialization
Scene_Purr::Scene_Purr(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_worldView(gameEngine->window().getDefaultView()) {

	loadLevel(levelPath);
	registerActions();

	auto pos = m_worldView.getSize();

	pos.x = pos.x / 2.f;
	pos.y -= 20.f;

	spawnInvisibleCollisionBox();
	initTexts();
	spawnPlayer(pos);


	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(40);

	fadeOutRect.setSize(sf::Vector2f(gameEngine->window().getSize().x, gameEngine->window().getSize().y));
	fadeOutRect.setFillColor(sf::Color(0, 0, 0, 0)); 

	
	finalText.setFont(Assets::getInstance().getFont("main"));
	finalText.setCharacterSize(24);
	finalText.setFillColor(sf::Color::White);
	finalText.setPosition(100, 100); 

}
#pragma endregion

#pragma region Load and Level Config
void Scene_Purr::loadLevel(const std::string& path) {
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

void Scene_Purr::registerActions() {
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

void Scene_Purr::spawnPlayer(sf::Vector2f pos) {
	

	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(20.f, 20.f));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
	m_player->addComponent<CState>("grounded");
}

void Scene_Purr::spawnInvisibleCollisionBox() {

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
	m_invisibleCollisionBox->addComponent<CTransform>(sf::Vector2f(480.f, 490.f));
	m_invisibleCollisionBox->addComponent<CBoundingBox>(sf::Vector2f(1000.f, 1.f));
	m_invisibleCollisionBox->addComponent<CState>("grounded");

}

void Scene_Purr::initTexts() {
	displayText.setFont(Assets::getInstance().getFont("main"));
	displayText.setCharacterSize(24);
	displayText.setFillColor(sf::Color::White);
	displayText.setPosition(100, 100);

	textBackground.setFillColor(sf::Color(0, 0, 0, 200));
	textBackground.setPosition(90, 90);
	textBackground.setOutlineColor(sf::Color::White);
	textBackground.setSize(sf::Vector2f(displayText.getLocalBounds().width + 20, displayText.getLocalBounds().height + 20));

	timedTexts.push_back({ "The rays of sun blinds me...", sf::seconds(5), sf::seconds(10) });
	timedTexts.push_back({ "I reach for a pillow and smother my face with it.", sf::seconds(11), sf::seconds(15) });
	timedTexts.push_back({ "Moving my arms invite pain.", sf::seconds(16), sf::seconds(21) });
	timedTexts.push_back({ "I close my eyes...", sf::seconds(22), sf::seconds(27) });
	timedTexts.push_back({ "A thick fog encompasses my brain.", sf::seconds(28), sf::seconds(33) });
	timedTexts.push_back({ "Sweet.. sleep is coming back, but its hopeless.", sf::seconds(34), sf::seconds(38) });
	timedTexts.push_back({ "Another thought invades my brain.", sf::seconds(39), sf::seconds(45) });
	timedTexts.push_back({ "Sometimes just a familiar meow over my bed brings me back to reality...", sf::seconds(46), sf::seconds(56) });
	timedTexts.push_back({ " ", sf::seconds(46), sf::seconds(60) });


}

#pragma endregion

#pragma region Updates
void Scene_Purr::update(sf::Time dt) {
	m_elapsedTime += dt;

	if (currentTextIndex < timedTexts.size() && m_elapsedTime >= timedTexts[currentTextIndex].endTime) {
		
		currentTextIndex++;
	}

	if (currentTextIndex >= timedTexts.size()) {
		
	}

	if (currentTextIndex < timedTexts.size() && charIndex < timedTexts[currentTextIndex].text.length()) {
		if (m_elapsedTime - lastCharUpdateTime > sf::seconds(0.05)) {
			displayText.setString(timedTexts[currentTextIndex].text.substr(0, charIndex + 1));
			charIndex++;
			lastCharUpdateTime = m_elapsedTime;

			textBackground.setSize(sf::Vector2f(displayText.getLocalBounds().width + 20, displayText.getLocalBounds().height + 20));
			textBackground.setPosition(displayText.getPosition().x - 10, displayText.getPosition().y - 10);
		}
	}

	if (currentTextIndex < timedTexts.size() && m_elapsedTime >= timedTexts[currentTextIndex].startTime) {
	
		if (m_elapsedTime > timedTexts[currentTextIndex].endTime || charIndex >= timedTexts[currentTextIndex].text.length()) {
			currentTextIndex++;
			charIndex = 0;
			textStartTime = m_elapsedTime;
			if (currentTextIndex >= timedTexts.size()) {
				displayText.setString("");
			}
		}
	}

	if (!conditionalTextAddedSecond && m_elapsedTime >= sf::seconds(55)) {
		secondText(); 
		conditionalTextAddedSecond = true; 
	}

	if (!conditionalTextAddedThird && m_elapsedTime >= sf::seconds(115)) {
		thirdText(); 
		conditionalTextAddedThird = true;
	}

	if (!conditionalTextAddedFinish && m_elapsedTime >= sf::seconds(170)) {
		finishText();
		conditionalTextAddedFinish = true;
	}

	if (m_elapsedTime >= sf::seconds(190)) {
		endGame();
	}

	//Boxes Control
	if (!m_boxCreated[0] && m_elapsedTime >= sf::seconds(106)) {
		spawnInteractiveBoxes(0);
		m_boxCreated[0] = true;
	}

	if (!m_boxCreated[1] && m_elapsedTime >= sf::seconds(46)) {
		spawnInteractiveBoxes(1);
		m_boxCreated[1] = true;
	}

	if (!m_boxCreated[2] && m_elapsedTime >= sf::seconds(160)) {
		spawnInteractiveBoxes(2);
		m_boxCreated[2] = true;
	}

	if (m_boxCreated[0] && m_elapsedTime >= sf::seconds(120)) {
		removeInteractiveBoxes(0);
		m_boxCreated[0] = false;
	}
	if (m_boxCreated[1] && m_elapsedTime >= sf::seconds(60)) {
		removeInteractiveBoxes(1);
		m_boxCreated[1] = false;
	}
	if (m_boxCreated[2] && m_elapsedTime >= sf::seconds(170)) {
		removeInteractiveBoxes(2);
		m_boxCreated[2] = false;
	}

	
	sUpdate(dt);
	
}

void Scene_Purr::sUpdate(sf::Time dt) {
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

#pragma endregion

#pragma region Events and Actions
void Scene_Purr::sDoAction(const Command& action) {
	if (action.type() == "START") {
		if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
		else if (action.name() == "QUIT") { m_game->quitLevel(); }
		else if (action.name() == "BACK") { m_game->backLevel(); }

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }


		if (action.name() == "LEFT") { m_player->getComponent<CInput>().dir = CInput::LEFT; }
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
		else if (action.name() == "UP") { m_player->getComponent<CInput>().dir = CInput::UP; }
		else if (action.name() == "DOWN") { m_player->getComponent<CInput>().dir = CInput::DOWN; }

	}

	else if (action.type() == "END" && (action.name() == "LEFT" || action.name() == "RIGHT" || action.name() == "UP" ||
		action.name() == "DOWN")) {
		m_player->getComponent<CInput>().dir = 0;
	}
	if (action.type() == "START" && action.name() == "ACTIVATE") {
		auto& playerTransform = m_player->getComponent<CTransform>();
		for (auto& box : m_interactiveBoxes) {
			if (box != nullptr && checkCollision(*m_player, *box)) {
				box->getComponent<CState>().state = "active";
				activatedBoxes++;
				std::cout << "Activated Boxes: " << activatedBoxes << std::endl;
				std::cout << "New State of the Box: " << box->getComponent<CState>().state << std::endl;
				SoundPlayer::getInstance().play("meow");
			}
		}
	}

}

#pragma endregion

#pragma region Animation and Movement

void Scene_Purr::sMovement(sf::Time dt) {
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

void Scene_Purr::sAnimation(sf::Time dt) {
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities()) {
		if (e->hasComponent<CAnimation>()) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);
		}
	}
}

void Scene_Purr::applyGravity(sf::Time dt) {
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

void Scene_Purr::adjustPlayerPosition() {
	auto center = m_worldView.getCenter();
	sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


	auto left = 25;
	auto right = 975;
	auto top = center.y - viewHalfSize.y;
	auto bot = 500;

	auto& player_pos = m_player->getComponent<CTransform>().pos;
	auto halfSize = sf::Vector2f{ 20, 20 };
	player_pos.x = std::max(player_pos.x, left + halfSize.x);
	player_pos.x = std::min(player_pos.x, right - halfSize.x);
	player_pos.y = std::max(player_pos.y, top + halfSize.y);
	player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

void Scene_Purr::playerMovement() {
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

#pragma endregion

#pragma region Collisions

void Scene_Purr::sCollisions(sf::Time dt) {
	auto& entities = m_entityManager.getEntities();
	for (auto& entity : entities) {
		if (entity->getTag() == "player") {
			auto& playerTransform = entity->getComponent<CTransform>();
			auto& playerBox = entity->getComponent<CBoundingBox>();

			for (auto& other : entities) {
				if (entity == other || !other->hasComponent<CBoundingBox>()) continue;

				if (other->getTag() == "invisibleCollisionBox" && checkCollision(*entity, *other)) {
					entity->getComponent<CState>().state = "grounded";
					break;
				}

				else {
					entity->getComponent<CState>().state = "jumping";
				}

			}
		}
	}
}

bool Scene_Purr::checkCollision(Entity& entity1, Entity& entity2) {
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

		bool collision = rect1.intersects(rect2);
		return collision;
	}
	return false;
}

bool Scene_Purr::isOnGround() const {
	if (!m_player) return false; 

	auto& transform = m_player->getComponent<CTransform>();
	auto& boundingBox = m_player->getComponent<CBoundingBox>();

	float groundHeight = 500;

	if ((transform.pos.y + boundingBox.halfSize.y) > groundHeight) {
		return true;
	}


	return (transform.pos.y + boundingBox.halfSize.y) >= groundHeight;
}

void Scene_Purr::checkGroundCollision() {
	if (!m_player) return; 

	auto& transform = m_player->getComponent<CTransform>();
	auto& boundingBox = m_player->getComponent<CBoundingBox>();

	float groundHeight = 500; 

	if ((transform.pos.y + boundingBox.halfSize.y) > groundHeight) {
		transform.pos.y = groundHeight - boundingBox.halfSize.y;
	}
}

#pragma endregion

#pragma region Render

void Scene_Purr::sRender() {
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

	textBackground.setSize(sf::Vector2f(displayText.getGlobalBounds().width + 20, displayText.getGlobalBounds().height + 30));
	textBackground.setPosition(displayText.getPosition().x - 10, displayText.getPosition().y - 10);


	if (!displayText.getString().isEmpty()) {
		m_game->window().draw(textBackground);
		m_game->window().draw(displayText);
	}

	if (isFadingOut) {
		auto color = fadeOutRect.getFillColor();
		if (color.a < 255 - fadeOutSpeed) {
			color.a += fadeOutSpeed;
		}
		else {
			color.a = 255;
		}
		fadeOutRect.setFillColor(color);
		m_game->window().draw(fadeOutRect);

		m_game->window().draw(textBackground);
		m_game->window().draw(finalText);
	}
}

void Scene_Purr::drawBackground() {
	for (auto e : m_entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			m_game->window().draw(e->getComponent<CSprite>().sprite);
		}
	}
}

void Scene_Purr::drawEntities() {
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

void Scene_Purr::drawBoundingBox(std::shared_ptr<Entity> entity) {
	auto box = entity->getComponent<CBoundingBox>();
	sf::RectangleShape rect(sf::Vector2f{ box.size.x, box.size.y });
	centerOrigin(rect);
	rect.setPosition(entity->getComponent<CTransform>().pos);
	rect.setFillColor(sf::Color(0, 0, 0, 0));
	rect.setOutlineColor(sf::Color{ 0, 255, 0 });
	rect.setOutlineThickness(2.f);
	m_game->window().draw(rect);
}

#pragma endregion

#pragma region Texts

void Scene_Purr::secondText() {
	displayText.setFont(Assets::getInstance().getFont("main"));
	displayText.setCharacterSize(24);
	displayText.setFillColor(sf::Color::White);
	displayText.setPosition(100, 100);

	// Configura a caixa de fundo para o texto
	textBackground.setFillColor(sf::Color(0, 0, 0, 200));
	textBackground.setPosition(90, 90);
	textBackground.setOutlineColor(sf::Color::White);
	textBackground.setSize(sf::Vector2f(displayText.getLocalBounds().width + 20, displayText.getLocalBounds().height + 20));

	if (checkBox1State()) {
		timedTexts.push_back({ "Yes... thats what i was thinking about....", sf::seconds(50), sf::seconds(60) });
		timedTexts.push_back({ "", sf::seconds(60), sf::seconds(65) });
	}
	else {
		timedTexts.push_back({ "but sometimes... i cant even hear him...", sf::seconds(50), sf::seconds(60) });
		timedTexts.push_back({ "", sf::seconds(60), sf::seconds(65) });
	}

	timedTexts.push_back({ "I think I'm going to lie down all day...", sf::seconds(65), sf::seconds(70) });
	timedTexts.push_back({ "Why can�t I just go back to sleep? ", sf::seconds(71), sf::seconds(75) });
	timedTexts.push_back({ "I Take a breath...", sf::seconds(76), sf::seconds(81) });
	timedTexts.push_back({ "What to try now? If I've tried everything...", sf::seconds(82), sf::seconds(87) });
	timedTexts.push_back({ "I can feel my cat moving around the room...", sf::seconds(88), sf::seconds(93) });
	timedTexts.push_back({ "Can I make him happy?", sf::seconds(94), sf::seconds(98) });
	timedTexts.push_back({ "I think he wants to show me something...", sf::seconds(99), sf::seconds(105) });
	timedTexts.push_back({ "Haha, maybe he wants me to read something to him?", sf::seconds(106), sf::seconds(116) });
	timedTexts.push_back({ " ", sf::seconds(116), sf::seconds(120) });
}

void Scene_Purr::thirdText() {

	displayText.setFont(Assets::getInstance().getFont("main"));
	displayText.setCharacterSize(24);
	displayText.setFillColor(sf::Color::White);
	displayText.setPosition(100, 100);


	textBackground.setFillColor(sf::Color(0, 0, 0, 200));
	textBackground.setPosition(90, 90);
	textBackground.setOutlineColor(sf::Color::White);
	textBackground.setSize(sf::Vector2f(displayText.getLocalBounds().width + 20, displayText.getLocalBounds().height + 20));

	if (checkBox0State()) {
		timedTexts.push_back({ "Okay... maybe you're right.", sf::seconds(120), sf::seconds(125) });
		timedTexts.push_back({ "", sf::seconds(125), sf::seconds(130) });
	}
	else {
		timedTexts.push_back({ "Maybe he's just exercising.", sf::seconds(120), sf::seconds(125) });
		timedTexts.push_back({ "", sf::seconds(125), sf::seconds(130) });
	}

	timedTexts.push_back({ "Or maybe I should try to get up...", sf::seconds(120), sf::seconds(125) });
	timedTexts.push_back({ "Some days are more difficult than others... ", sf::seconds(126), sf::seconds(136) });
	timedTexts.push_back({ "I wish I had some friends...", sf::seconds(137), sf::seconds(141) });
	timedTexts.push_back({ "haha i know i know!", sf::seconds(142), sf::seconds(147) });
	timedTexts.push_back({ "You're trying your best", sf::seconds(148), sf::seconds(153) });
	timedTexts.push_back({ "Well... I need to find my diary... ", sf::seconds(154), sf::seconds(158) });
	timedTexts.push_back({ "Maybe I have some idea what to do...", sf::seconds(159), sf::seconds(165) });
	timedTexts.push_back({ "Behind the couch, under the bed or on the desk?", sf::seconds(166), sf::seconds(176) });
	timedTexts.push_back({ " ", sf::seconds(176), sf::seconds(180) });
}

void Scene_Purr::finishText() {

	displayText.setFont(Assets::getInstance().getFont("main"));
	displayText.setCharacterSize(24);
	displayText.setFillColor(sf::Color::White);
	displayText.setPosition(100, 100);


	textBackground.setFillColor(sf::Color(0, 0, 0, 200));
	textBackground.setPosition(90, 90);
	textBackground.setOutlineColor(sf::Color::White);
	textBackground.setSize(sf::Vector2f(displayText.getLocalBounds().width + 20, displayText.getLocalBounds().height + 20));

	if (checkBox2State()) {
		timedTexts.push_back({ "You have found it!", sf::seconds(180), sf::seconds(185) });
		timedTexts.push_back({ "I Really need to do something with my life. And i will!", sf::seconds(186), sf::seconds(190) });
		timedTexts.push_back({ "", sf::seconds(195), sf::seconds(200) });
	}
	else {
		timedTexts.push_back({ "Better forget about it...", sf::seconds(180), sf::seconds(185) });
		timedTexts.push_back({ "I Really need to do something with my life. Maybe...", sf::seconds(186), sf::seconds(190) });
		timedTexts.push_back({ "", sf::seconds(195), sf::seconds(200) });
	}

}

#pragma endregion

#pragma region EndGame

void Scene_Purr::endGame() {

	std::string resultText;
	if (activatedBoxes == 3) {
		resultText = "Today you made my day better.\n I promise to try my best to make tomorrow a little better.";
	}
	else if (activatedBoxes > 0) {
		resultText = "Thanks for trying... maybe tomorrow will be better";
	}
	else {
		resultText = "I think the best thing... is to go back to sleep.";
	}

	finalText.setString(resultText);
	sf::FloatRect textRect = finalText.getLocalBounds();
	finalText.setOrigin(textRect.width / 2, textRect.height / 2);
	finalText.setPosition(sf::Vector2f(m_game->window().getSize().x / 2, m_game->window().getSize().y / 2));

	textBackground.setSize(sf::Vector2f(textRect.width + 60, textRect.height + 60));
	textBackground.setOrigin(textBackground.getSize().x / 2, textBackground.getSize().y / 2);
	textBackground.setPosition(finalText.getPosition());

	isFadingOut = true;
}

void Scene_Purr::onEnd() {
	m_game->changeScene("MENU", nullptr, false);
}

#pragma endregion

#pragma region Support And Utilities

float Scene_Purr::getGroundLevelAt(float x) {
	return 500.0f;
}

sf::FloatRect Scene_Purr::getViewBounds() {
	return sf::FloatRect();
}

void Scene_Purr::spawnInteractiveBoxes(int boxIndex) {
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
		box->getComponent<CState>().state = "inactive";

		break;
	case 1:
		box->addComponent<CTransform>(sf::Vector2f(505.f, 350.f));
		box->addComponent<CBoundingBox>(sf::Vector2f(220.f, 50.f));
		box->getComponent<CState>().state = "inactive";

		break;
	case 2:
		box->addComponent<CTransform>(sf::Vector2f(910.f, 380.f));
		box->addComponent<CBoundingBox>(sf::Vector2f(115.f, 100.f));
		box->getComponent<CState>().state = "inactive";

	}
	m_interactiveBoxes[boxIndex] = box;
}

void Scene_Purr::removeInteractiveBoxes(int boxIndex) {
	if (boxIndex < m_interactiveBoxes.size() && m_interactiveBoxes[boxIndex] != nullptr) {
		m_interactiveBoxes[boxIndex]->getComponent<CTransform>().pos = sf::Vector2f(-1000, -1000);
		m_interactiveBoxes[boxIndex] = nullptr;
	}
}

bool Scene_Purr::checkBox0State() {
	if (!m_interactiveBoxes.empty() && m_interactiveBoxes[0] != nullptr) {
		return m_interactiveBoxes[0]->getComponent<CState>().state == "active";
	}
	return false;
}

bool Scene_Purr::checkBox1State() {
	if (!m_interactiveBoxes.empty() && m_interactiveBoxes[1] != nullptr) {
		return m_interactiveBoxes[1]->getComponent<CState>().state == "active";
	}
	return false;
}

bool Scene_Purr::checkBox2State() {
	if (!m_interactiveBoxes.empty() && m_interactiveBoxes[2] != nullptr) {
		return m_interactiveBoxes[2]->getComponent<CState>().state == "active";
	}
	return false;
}

#pragma endregion











































