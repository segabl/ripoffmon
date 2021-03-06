/*
 * Player.cpp
 *
 *  Created on: 03. Feb. 2016
 *      Author: Sebastian
 */

#include "Player.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <queue>

#include "LevelData.h"

using namespace std;
using namespace sf;

Player* Player::instance = 0;

Player* Player::get() {
  if (!instance) {
    instance = new Player();
  }
  return instance;
}

Player::Player() : Entity() {

}

void Player::performActions(LevelData* data) {
  bool right = Keyboard::isKeyPressed(Keyboard::Right);
  bool up = Keyboard::isKeyPressed(Keyboard::Up);
  bool left = Keyboard::isKeyPressed(Keyboard::Left);
  bool down = Keyboard::isKeyPressed(Keyboard::Down);
  bool run = Keyboard::isKeyPressed(Keyboard::LShift);

  switch (movement) {
    case LevelData::LEDGE_RIGHT:
    action_queue.push(ActionPair(JUMP_RIGHT, 1));
    break;
    case LevelData::LEDGE_UP:
    action_queue.push(ActionPair(JUMP_UP, 1));
    break;
    case LevelData::LEDGE_LEFT:
    action_queue.push(ActionPair(JUMP_LEFT, 1));
    break;
    case LevelData::LEDGE_DOWN:
    action_queue.push(ActionPair(JUMP_DOWN, 1));
    break;

    case LevelData::PUSH_RIGHT:
    action_queue.push(ActionPair(MOVE_RIGHT, 2));
    break;
    case LevelData::PUSH_UP:
    action_queue.push(ActionPair(MOVE_UP, 2));
    break;
    case LevelData::PUSH_LEFT:
    action_queue.push(ActionPair(MOVE_LEFT, 2));
    break;
    case LevelData::PUSH_DOWN:
    action_queue.push(ActionPair(MOVE_DOWN, 2));
    break;

    default:
    if (up) {
      if (canMove(0, -1, data) && facing == UP) {
        action_queue.push(ActionPair(MOVE_UP, 1.5 + 1.5 * run));
      } else {
        action_queue.push(ActionPair(LOOK_UP, 4));
      }
    } else if (left) {
      if (canMove(-1, 0, data) && facing == LEFT) {
        action_queue.push(ActionPair(MOVE_LEFT, 1.5 + 1.5 * run));
      } else {
        action_queue.push(ActionPair(LOOK_LEFT, 4));
      }
    } else if (down) {
      if (canMove(0, 1, data) && facing == DOWN) {
        action_queue.push(ActionPair(MOVE_DOWN, 1.5 + 1.5 * run));
      } else {
        action_queue.push(ActionPair(LOOK_DOWN, 4));
      }
    } else if (right) {
      if (canMove(1, 0, data) && facing == RIGHT) {
        action_queue.push(ActionPair(MOVE_RIGHT, 1.5 + 1.5 * run));
      } else {
        action_queue.push(ActionPair(LOOK_RIGHT, 4));
      }
    }
    break;
  }
}

#define ABS(A) ((A) < 0 ? -A : A)
void Player::draw(sf::RenderTarget& rt, sf::RenderStates rs) const {
  short off = 0;
  if (!action_queue.empty()) {
    ActionPair ap = action_queue.front();
    if (ap.first >= JUMP_RIGHT && ap.first <= JUMP_DOWN) {
      off = -8 * (1 - ABS((action_step - 0.5) * 2));
    }
  }

  CircleShape cshp(8, 8);
  cshp.setOrigin(8, 8);
  cshp.setScale(1, 0.5);
  cshp.setPosition((prev_x * (1 - action_step) + x * action_step) * 16 + 8,
      (prev_y * (1 - action_step) + y * action_step) * 16 + 8 + 4);
  cshp.setFillColor(Color(0, 0, 0, 100));
  rt.draw(cshp, rs);

  RectangleShape shp(Vector2f(16, 24));
  shp.setOrigin(8, 24 - 8);
  shp.setPosition((prev_x * (1 - action_step) + x * action_step) * 16 + 8,
      (prev_y * (1 - action_step) + y * action_step) * 16 + 8 + off);
  shp.setFillColor(Color(on_floor * 100, 0, 255));
  rt.draw(shp, rs);

  cshp.setRadius(6);
  cshp.setPointCount(3);
  cshp.setOrigin(6, 6);
  cshp.setScale(1, 1);
  cshp.setRotation(90 - facing * 90);
  cshp.setPosition((prev_x * (1 - action_step) + x * action_step) * 16 + 8,
      (prev_y * (1 - action_step) + y * action_step) * 16 + 8 + off);
  cshp.setFillColor(Color(0, 0, 0, 100));
  rt.draw(cshp, rs);
}
