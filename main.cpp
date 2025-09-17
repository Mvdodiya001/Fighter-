#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <numeric>
#include <map>
#include <algorithm> // Required for std::shuffle

#include "Fighter.h"

// Helper functions (drawText, drawHealthBar) remain the same...
void drawText(const std::string& text, sf::Font& font, int size, sf::Color color, float x, float y, sf::RenderWindow& window) {
    sf::Text txt(text, font, size);
    txt.setFillColor(color);
    txt.setPosition(x, y);
    window.draw(txt);
}

void drawHealthBar(float health, float x, float y, sf::RenderWindow& window) {
    float ratio = health / 100.0f;
    sf::RectangleShape bg(sf::Vector2f(300, 30));
    bg.setFillColor(sf::Color::White);
    bg.setPosition(x, y);
    window.draw(bg);

    sf::RectangleShape fg(sf::Vector2f(300 * ratio, 30));
    fg.setFillColor(sf::Color::Red);
    fg.setPosition(x, y);
    window.draw(fg);
}


int main() {
    const int SC_WIDTH = 1000;
    const int SC_HEIGHT = 540;

    sf::RenderWindow window(sf::VideoMode(SC_WIDTH, SC_HEIGHT), "Pixel Warrior");
    window.setFramerateLimit(60);

    // Game State Variables, Asset Loading... (same as before)
    int introCount = 4;
    sf::Clock introClock;
    int score[2] = {0, 0};
    bool roundOver = false;
    const float ROUND_OVER_COOLDOWN = 2000.0f;
    sf::Clock roundOverClock;

    sf::Music bgMusic;
    if (!bgMusic.openFromFile("music/bgmusic.mp3")) return -1;
    bgMusic.setVolume(50);
    bgMusic.setLoop(true);
    bgMusic.play();
    
    sf::SoundBuffer introSoundBuffer;
    if (!introSoundBuffer.loadFromFile("music/321fight.mp3")) return -1;
    sf::Sound introSound(introSoundBuffer);

    sf::Texture victory1Tex, victory2Tex, healthBarTex;
    if (!victory1Tex.loadFromFile("p1.png")) return -1;
    if (!victory2Tex.loadFromFile("p2.png")) return -1;
    if (!healthBarTex.loadFromFile("health bar.png")) return -1;
    sf::Sprite victory1Sprite(victory1Tex), victory2Sprite(victory2Tex), healthBarSprite(healthBarTex);

    std::map<int, sf::Texture> countdownTextures;
    for (int i = 1; i <= 3; ++i) {
        sf::Texture tex;
        if (!tex.loadFromFile("intro/" + std::to_string(i) + ".png")) return -1;
        countdownTextures[i] = tex;
    }
    
    sf::Font pixelFont;
    if (!pixelFont.loadFromFile("VCR_OSD_MONO_1.001.ttf")) return -1;

    // Parallax Background (same as before)
    std::vector<sf::Texture> bgTextures;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> bgDist(1, 3);
    int bgChoice = bgDist(rng);
    
    if (bgChoice == 1) { for (int i = 4; i > 0; --i) { sf::Texture t; if(t.loadFromFile("Background/paralaxbg1/img " + std::to_string(i) + ".png")) bgTextures.push_back(t); } }
    else if (bgChoice == 2) { for (int i = 4; i > 0; --i) { sf::Texture t; if(t.loadFromFile("Background/paralaxbg2/img " + std::to_string(i) + ".png")) bgTextures.push_back(t); } }
    else { for (int i = 7; i > 0; --i) { sf::Texture t; if(t.loadFromFile("Background/paralaxbg3/img " + std::to_string(i) + ".png")) bgTextures.push_back(t); } }
    
    std::vector<sf::Sprite> bgSprites;
    for (const auto& tex : bgTextures) bgSprites.emplace_back(tex);
    float scroll = 0;


    // --- CHARACTER SELECTION (RESTORED TO ORIGINAL RANDOM LOGIC) ---
    std::map<int, CharacterData> characterDefs;
    characterDefs[1] = {"Good Fighter/knight moves.png", {11, 8, 3, 7, 7, 4, 11, 3}, 180, 3.0f, {220, 150}, "music/swordattack.wav", "music/swordmissattack.flac"};
    characterDefs[2] = {"Good Fighter/martial 1 moves.png", {8, 8, 2, 6, 6, 4, 6, 2}, 180, 3.0f, {232, 150}, "music/swordattack.wav", "music/swordmissattack.flac"};
    characterDefs[3] = {"Good Fighter/martial 2 moves.png", {4, 8, 2, 4, 4, 3, 7, 2}, 180, 3.0f, {220, 150}, "music/swordattack.wav", "music/swordmissattack.flac"};
    characterDefs[4] = {"Good Fighter/martial 3 moves.png", {10, 8, 3, 7, 9, 3, 11, 3}, 180, 3.0f, {220, 150}, "music/swordattack.wav", "music/swordmissattack.flac"};
    characterDefs[5] = {"Good Fighter/wiz 2 moves.png", {6, 8, 2, 8, 8, 5, 7, 2}, 180, 2.0f, {120, 129}, "music/fireattack.wav", "music/firemissattack.wav"};
    
    std::vector<int> charIds = {1, 2, 3, 4, 5};
    std::shuffle(charIds.begin(), charIds.end(), rng);
    int p1_id = charIds[0];
    int p2_id = charIds[1];
    
    std::map<int, sf::Texture> fighterTextures;
    // Load texture for Player 1
    if (!fighterTextures[p1_id].loadFromFile(characterDefs[p1_id].spritesheetPath)) {
        std::cerr << "FATAL ERROR: Could not load player 1 texture: " << characterDefs[p1_id].spritesheetPath << std::endl;
        return -1;
    }
    // Load texture for Player 2
    if (!fighterTextures[p2_id].loadFromFile(characterDefs[p2_id].spritesheetPath)) {
        std::cerr << "FATAL ERROR: Could not load player 2 texture: " << characterDefs[p2_id].spritesheetPath << std::endl;
        return -1;
    }

    Fighter F1(1, 100, 290, false, characterDefs[p1_id], fighterTextures[p1_id]);
    Fighter F2(2, 800, 290, true, characterDefs[p2_id], fighterTextures[p2_id]);

    sf::Clock deltaClock;

    // --- Game Loop ---
    while (window.isOpen()) {
        // ... The entire game loop remains the same as the last version ...
        float deltaTime = deltaClock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if (introCount <= 0) {
            F1.move(window.getSize(), F2, roundOver);
            F2.move(window.getSize(), F1, roundOver);
        } else {
            if (introClock.getElapsedTime().asSeconds() >= 1.0f) {
                if (introCount == 4) introSound.play();
                introCount--;
                introClock.restart();
            }
        }
        F1.update(deltaTime);
        F2.update(deltaTime);
        if (!roundOver) {
            if (!F1.isAlive()) {
                score[1]++;
                roundOver = true;
                roundOverClock.restart();
            } else if (!F2.isAlive()) {
                score[0]++;
                roundOver = true;
                roundOverClock.restart();
            }
        } else {
            if (roundOverClock.getElapsedTime().asMilliseconds() > ROUND_OVER_COOLDOWN) {
                roundOver = false;
                introCount = 4;
                F1.reset(100, 290, false);
                F2.reset(800, 290, true);
            }
        }
        window.clear();
        float speed = 0.5f;
        for (auto& bg_sprite : bgSprites) {
            scroll = (F1.getBoundingBox().left + F2.getBoundingBox().left) / 20.0f;
            float x_pos = -scroll * speed;
            while(x_pos <= -bg_sprite.getGlobalBounds().width) x_pos += bg_sprite.getGlobalBounds().width;
            bg_sprite.setPosition(x_pos, 0);
            window.draw(bg_sprite);
            bg_sprite.setPosition(x_pos + bg_sprite.getGlobalBounds().width, 0);
            window.draw(bg_sprite);
            speed += 0.2f;
        }
        F1.draw(window);
        F2.draw(window);
        drawHealthBar(F1.getHealth(), 70, 25, window);
        drawHealthBar(F2.getHealth(), 630, 25, window);
        window.draw(healthBarSprite);
        drawText(std::to_string(score[0]), pixelFont, 30, sf::Color::White, 7, 92, window);
        drawText(std::to_string(score[1]), pixelFont, 30, sf::Color::White, 900, 92, window);
        if (introCount > 0 && introCount < 4) {
            sf::Sprite cdSprite(countdownTextures[introCount]);
            window.draw(cdSprite);
        }
        if (roundOver) {
            if (F1.isAlive() && !F2.isAlive()) {
                window.draw(victory1Sprite);
            } else if (F2.isAlive() && !F1.isAlive()) {
                window.draw(victory2Sprite);
            }
        }
        window.display();
    }

    return 0;
}