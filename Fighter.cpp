#include "Fighter.h"
#include <iostream>

// The constructor, reset, loadAnimations, move, update, updateAction methods are all the same as the last version.
// The ONLY change is in the "attack" method.

Fighter::Fighter(int player, float x, float y, bool flip, const CharacterData& data, sf::Texture& texture)
    : m_texture(texture) {
    m_player = player;
    m_size = data.size;
    m_scale = data.scale;
    m_offset = data.offset;
    m_sprite.setTexture(m_texture);
    loadAnimations(data.animationSteps);
    if (!m_attackSoundBuffer.loadFromFile(data.attackSoundPath)) {
        std::cerr << "Error loading attack sound: " << data.attackSoundPath << std::endl;
    }
    m_attackSound.setBuffer(m_attackSoundBuffer);
    if (!m_missSoundBuffer.loadFromFile(data.missSoundPath)) {
         std::cerr << "Error loading miss sound: " << data.missSoundPath << std::endl;
    }
    m_missSound.setBuffer(m_missSoundBuffer);
    reset(x, y, flip);
}

void Fighter::reset(float x, float y, bool flip) {
    m_boundingBox = sf::FloatRect(x, y, 120, 180);
    m_flip = flip;
    m_velY = 0;
    m_running = false;
    m_jump = false;
    m_attacking = false;
    m_attackType = 0;
    m_attackCooldown = 0.0f;
    m_hit = false;
    m_health = 100;
    m_alive = true;
    m_action = 0;
    m_frameIndex = 0;
    m_animationTimer.restart();
}

void Fighter::loadAnimations(const std::vector<int>& animationSteps) {
    m_animationList.clear();
    for (size_t y = 0; y < animationSteps.size(); ++y) {
        std::vector<sf::IntRect> tempList;
        for (int x = 0; x < animationSteps[y]; ++x) {
            tempList.push_back(sf::IntRect(x * m_size, y * m_size, m_size, m_size));
        }
        m_animationList.push_back(tempList);
    }
}

void Fighter::move(const sf::Vector2u& windowSize, Fighter& target, bool roundOver) {
    const float SPEED = 5.0f;
    const float GRAVITY = 2.0f;
    float dx = 0.0f;
    m_running = false;
    m_attackType = 0;
    if (!m_attacking && m_alive && !roundOver) {
        if (m_player == 1) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { dx = -SPEED; m_running = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { dx = SPEED; m_running = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && !m_jump) { m_velY = -30.0f; m_jump = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) || sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                attack(target);
                m_attackType = sf::Keyboard::isKeyPressed(sf::Keyboard::Q) ? 1 : 2;
            }
        } else if (m_player == 2) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { dx = -SPEED; m_running = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { dx = SPEED; m_running = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !m_jump) { m_velY = -30.0f; m_jump = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2)) {
                attack(target);
                m_attackType = sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1) ? 1 : 2;
            }
        }
    }
    m_velY += GRAVITY;
    float dy = m_velY;
    if (m_boundingBox.left + dx < 0) { dx = -m_boundingBox.left; }
    if (m_boundingBox.left + m_boundingBox.width + dx > windowSize.x) { dx = windowSize.x - (m_boundingBox.left + m_boundingBox.width); }
    if (m_boundingBox.top + m_boundingBox.height + dy > windowSize.y - 70) {
        m_velY = 0;
        m_jump = false;
        dy = windowSize.y - 70 - (m_boundingBox.top + m_boundingBox.height);
    }
    if (target.getBoundingBox().left > m_boundingBox.left) { m_flip = false; } else { m_flip = true; }
    if (m_attackCooldown > 0) { m_attackCooldown -= 1; }
    m_boundingBox.left += dx;
    m_boundingBox.top += dy;
}

// *** UPDATED ATTACK METHOD ***
void Fighter::attack(Fighter& target) {
    if (m_attackCooldown == 0) {
        m_attacking = true;
        m_missSound.play();
        
        // Create a more generous hitbox in front of the player
        float attackX = m_boundingBox.left;
        float attackY = m_boundingBox.top;
        float attackWidth = m_boundingBox.width * 1.5f; // Make it 50% wider
        float attackHeight = m_boundingBox.height;

        if (m_flip) {
            // If flipped (facing left), position the attack box to the left
            attackX = m_boundingBox.left - m_boundingBox.width;
        } else {
            // If not flipped (facing right), position it normally but account for width
            attackX = m_boundingBox.left + (m_boundingBox.width * 0.5f);
        }

        sf::FloatRect attackRect(attackX, attackY, attackWidth, attackHeight);

        if (attackRect.intersects(target.getBoundingBox())) {
            m_attackSound.play();
            target.m_health -= 10;
            target.m_hit = true;
        }
    }
}

void Fighter::update(float deltaTime) {
    if (m_health <= 0) {
        m_health = 0;
        m_alive = false;
        updateAction(6);
    } else if (m_hit) {
        updateAction(5);
    } else if (m_attacking) {
        updateAction(m_attackType == 1 ? 3 : 4);
    } else if (m_jump) {
        updateAction(2);
    } else if (m_running) {
        updateAction(1);
    } else {
        updateAction(0);
    }
    float animationSpeed = 70.0f;
    if (m_animationTimer.getElapsedTime().asMilliseconds() > animationSpeed) {
        m_frameIndex++;
        m_animationTimer.restart();
        if (m_frameIndex >= m_animationList[m_action].size()) {
            if (!m_alive) {
                m_frameIndex = m_animationList[m_action].size() - 1;
            } else {
                m_frameIndex = 0;
                if (m_action == 3 || m_action == 4) {
                    m_attacking = false;
                    m_attackCooldown = 25.0f;
                }
                if (m_action == 5) {
                    m_hit = false;
                    m_attacking = false;
                }
            }
        }
    }
}

void Fighter::updateAction(int newAction) {
    if (newAction != m_action) {
        m_action = newAction;
        m_frameIndex = 0;
        m_animationTimer.restart();
    }
}

void Fighter::draw(sf::RenderWindow& window) {
    m_sprite.setTextureRect(m_animationList[m_action][m_frameIndex]);
    m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
    m_sprite.setScale(m_flip ? -m_scale : m_scale, m_scale);
    m_sprite.setPosition(
        m_boundingBox.left + m_boundingBox.width / 2.0f,
        m_boundingBox.top + m_boundingBox.height / 2.0f
    );
    window.draw(m_sprite);
}