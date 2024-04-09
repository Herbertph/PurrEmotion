#include "Scene_Menu.h"
#include "Scene_Purr.h"
#include <memory>
#include "Assets.h"

void Scene_Menu::onEnd()
{
	m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
	
}



void Scene_Menu::init()
{
	if (!m_backgroundTexture.loadFromFile("../assets/menu.png")) {
		std::cerr << "Failed to load menu background texture" << std::endl;
	}

	m_background.setTexture(m_backgroundTexture);

	sf::Vector2u textureSize = m_backgroundTexture.getSize();

	sf::Vector2u windowSize = m_game->window().getSize();

	float scaleX = static_cast<float>(windowSize.x) / textureSize.x;
	float scaleY = static_cast<float>(windowSize.y) / textureSize.y;

	m_background.setScale(scaleX, scaleY);


	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
	registerAction(sf::Keyboard::D, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");


	m_title = "PurrEmotion";
	m_menuStrings.push_back("Start");
	m_menuStrings.push_back("Exit Game");
	

	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level1.txt");

	m_menuText.setFont(Assets::getInstance().getFont("main"));

	const size_t CHAR_SIZE{ 64 };
	m_menuText.setCharacterSize(CHAR_SIZE);


}

void Scene_Menu::update(sf::Time dt)
{
	m_entityManager.update();
}


void Scene_Menu::sRender()
{
	m_game->window().clear(sf::Color::Black);
	
	sf::View view = m_game->window().getView();
	view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
	m_game->window().setView(view);

	m_game->window().draw(m_background);


	static const sf::Color selectedColor(255, 255, 0);
	static const sf::Color normalColor(0, 0, 255);

	static const sf::Color backgroundColor(100, 100, 255);

	sf::Text footer("UP: W    DOWN: S   PLAY:D    QUIT: ESC",
		Assets::getInstance().getFont("main"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 700);

	m_game->window().draw(footer);
}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		}
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			Assets::getInstance().reset();
			m_game->changeScene("PLAY", std::make_shared<Scene_Purr>(m_game, m_levelPaths[m_menuIndex]));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}
}
