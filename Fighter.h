#ifndef FIGHTER_H
#define FIGHTER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>

// Forward declaration to resolve circular dependency
class Fighter;

struct CharacterData {
    std::string spritesheetPath;
    std::vector<int> animationSteps;
    int size;
    float scale;
    sf::Vector2f offset;
    std::string attackSoundPath;
    std::string missSoundPath;
};

class Fighter {
public:
    Fighter(int player, float x, float y, bool flip, const CharacterData& data, sf::Texture& texture);

    void move(const sf::Vector2u& windowSize, Fighter& target, bool roundOver);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    void attack(Fighter& target);
    void reset(float x, float y, bool flip);

    sf::FloatRect getBoundingBox() const { return m_boundingBox; }
    int getHealth() const { return m_health; }
    bool isAlive() const { return m_alive; }
    
private:
    void updateAction(int newAction);
    void loadAnimations(const std::vector<int>& animationSteps);

    // Member Variables
    int m_player;
    int m_size;
    float m_scale;
    sf::Vector2f m_offset;
    bool m_flip;
    float m_velY;
    bool m_running;
    bool m_jump;
    bool m_attacking;
    int m_attackType;
    float m_attackCooldown;
    bool m_hit;
    int m_health;
    bool m_alive;
    
    sf::FloatRect m_boundingBox;
    sf::Sprite m_sprite;
    sf::Texture& m_texture;

    // Animation
    std::vector<std::vector<sf::IntRect>> m_animationList;
    int m_action; // 0:Idle, 1:Run, 2:Jump, 3:Attack1, 4:Attack2, 5:Hit, 6:Death
    int m_frameIndex;
    sf::Clock m_animationTimer;

    // Sound
    sf::SoundBuffer m_attackSoundBuffer;
    sf::Sound m_attackSound;
    sf::SoundBuffer m_missSoundBuffer;
    sf::Sound m_missSound;
};

#endif // FIGHTER_H