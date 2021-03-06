/*
 * Level.cpp
 *
 *  Created on: 03. Feb. 2016
 *      Author: Sebastian
 */

#include "Level.h"

#include <SFML/Graphics.hpp>
#include <iostream>

#include "Managers/TileSetManager.h"
#include "Player.h"
#include "Utils/File.h"

using namespace std;
using namespace sf;

TileSet Level::debug_tiles;

void Level::setVertexArrays(unsigned i) {
  unsigned x = i % data->width;
  unsigned y = i / data->width;

  va_debug[i * 4 + 0].position = va_back[i * 4 + 0].position = va_front[i * 4 + 0].position = Vector2f(x, y) * TileSet::tile_size;
  va_debug[i * 4 + 1].position = va_back[i * 4 + 1].position = va_front[i * 4 + 1].position = Vector2f(x, y + 1) * TileSet::tile_size;
  va_debug[i * 4 + 2].position = va_back[i * 4 + 2].position = va_front[i * 4 + 2].position = Vector2f(x + 1, y + 1) * TileSet::tile_size;
  va_debug[i * 4 + 3].position = va_back[i * 4 + 3].position = va_front[i * 4 + 3].position = Vector2f(x + 1, y) * TileSet::tile_size;

  // Back
  Vector2u tile = tiles->getTileCoords(data->tiles_back[x][y]);
  va_back[i * 4 + 0].texCoords = Vector2f(tile.x, tile.y);
  va_back[i * 4 + 1].texCoords = Vector2f(tile.x, tile.y + TileSet::tile_size);
  va_back[i * 4 + 2].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y + TileSet::tile_size);
  va_back[i * 4 + 3].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y);
  if (tiles->getTileFlags(data->tiles_back[x][y]) & TileSet::ANIMATED) {
    animated_back.push_back(i);
  }

  // Front
  tile = tiles->getTileCoords(data->tiles_front[x][y]);
  va_front[i * 4 + 0].texCoords = Vector2f(tile.x, tile.y);
  va_front[i * 4 + 1].texCoords = Vector2f(tile.x, tile.y + TileSet::tile_size);
  va_front[i * 4 + 2].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y + TileSet::tile_size);
  va_front[i * 4 + 3].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y);
  if (tiles->getTileFlags(data->tiles_front[x][y]) & TileSet::ANIMATED) {
    animated_front.push_back(i);
  }
}

void Level::renderOutsideTexture(Uint16* back, Uint16* front) {
  VertexArray va_back(Quads, 2 * 2 * 4);
  VertexArray va_front(Quads, 2 * 2 * 4);
  for (unsigned i = 0; i < 4; i++) {
    unsigned x = i % 2;
    unsigned y = i / 2;

    va_back[i * 4 + 0].position = va_front[i * 4 + 0].position = Vector2f(x, y) * TileSet::tile_size;
    va_back[i * 4 + 1].position = va_front[i * 4 + 1].position = Vector2f(x, y + 1) * TileSet::tile_size;
    va_back[i * 4 + 2].position = va_front[i * 4 + 2].position = Vector2f(x + 1, y + 1) * TileSet::tile_size;
    va_back[i * 4 + 3].position = va_front[i * 4 + 3].position = Vector2f(x + 1, y) * TileSet::tile_size;

    Vector2u tile = tiles->getTileCoords(back[i]);
    va_back[i * 4 + 0].texCoords = Vector2f(tile.x, tile.y);
    va_back[i * 4 + 1].texCoords = Vector2f(tile.x, tile.y + TileSet::tile_size);
    va_back[i * 4 + 2].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y + TileSet::tile_size);
    va_back[i * 4 + 3].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y);

    tile = tiles->getTileCoords(front[i]);
    va_front[i * 4 + 0].texCoords = Vector2f(tile.x, tile.y);
    va_front[i * 4 + 1].texCoords = Vector2f(tile.x, tile.y + TileSet::tile_size);
    va_front[i * 4 + 2].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y + TileSet::tile_size);
    va_front[i * 4 + 3].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y);
  }
  texture_outside.clear(Color::Black);
  texture_outside.draw(va_back, &tiles->texture);
  texture_outside.draw(va_front, &tiles->texture);
  texture_outside.display();
  texture_outside.setRepeated(true);
}

Level::Level() {
  loaded = false;
  data = 0;
  flags = 0;
  music = 0;
  tiles = 0;
}

Level::~Level() {
  delete data;
}

bool Level::load(const char* fname) {
  Clock c;
  File f;
  if (!f.open(fname, fstream::in | fstream::binary)) {
    cerr << "Failed to load level \"" << fname << "\"!\n";
    return false;
  }
  // Set dimensions
  Uint16 width = f.read<Uint16>();
  Uint16 height = f.read<Uint16>();

  data = new LevelData(width, height);

  texture_debug.create(width * TileSet::tile_size, height * TileSet::tile_size);
  texture_back.create(width * TileSet::tile_size, height * TileSet::tile_size);
  texture_front.create(width * TileSet::tile_size, height * TileSet::tile_size);
  va_debug = VertexArray(Quads, data->width * data->height * 4);
  va_back = VertexArray(Quads, data->width * data->height * 4);
  va_front = VertexArray(Quads, data->width * data->height * 4);

  // Neighbours
  for (unsigned i = 0; i < 4; i++) {
    neighbour[i].id = f.read<Uint16>();
    neighbour[i].offset = f.read<short>();
  }

  // Flags
  flags = f.read<Uint8>();

  // TileSet
  tiles = TileSetManager::getTileSet(f.read<Uint8>());

  // Music
  music = f.read<Uint8>();

  // Data
  for (unsigned i = 0; i < (unsigned) width * (unsigned) height; i++) {
    data->movement[i % width][i / width] = f.read<Uint8>();
    data->floors[i % width][i / width] = f.read<Uint8>();
    data->tiles_back[i % width][i / width] = f.read<Uint16>();
    data->tiles_front[i % width][i / width] = f.read<Uint16>();
    setVertexArrays(i);
  }

  // Outside Tiles
  texture_outside.create(TileSet::tile_size * 2, TileSet::tile_size * 2);

  Uint16 o_back[4], o_front[4];
  for (unsigned i = 0; i < 4; i++) {
    o_back[i] = f.read<Uint16>();
    o_front[i] = f.read<Uint16>();
  }

  renderOutsideTexture(o_back, o_front);

  // Entities
  while (!f.eof()) {
    unsigned etype = f.read<Uint8>();
    cout << "Entity with type " << etype << endl;
    switch (etype) {
    // TODO
    }
  }

  f.close();

  loaded = true;

  return true;
}

void Level::update() {
  // Update all entities
  /*for (unsigned i = 0; i < entities.size(); i++) {
    entities[i]->update(data);
  }*/
}

void Level::render() {
  if (!loaded) {
    return;
  }

  texture_debug.clear(Color::Magenta);
  for (unsigned i = 0; i < unsigned(data->width) * unsigned(data->height); i++) {
    unsigned x = i % data->width;
    unsigned y = i / data->width;

    // Debug
    Vector2u tile = debug_tiles.getTileCoords((data->movement[x][y] & LevelData::TEMP_BLOCKED) ? 0 : data->movement[x][y]);
    va_debug[i * 4 + 0].texCoords = Vector2f(tile.x, tile.y);
    va_debug[i * 4 + 1].texCoords = Vector2f(tile.x, tile.y + TileSet::tile_size);
    va_debug[i * 4 + 2].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y + TileSet::tile_size);
    va_debug[i * 4 + 3].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y);
  }
  texture_debug.draw(va_debug, &debug_tiles.texture);
  texture_debug.display();

  for (unsigned i = 0; i < animated_back.size(); i++) {
    unsigned t = animated_back[i];
    unsigned x = t % data->width;
    unsigned y = t / data->width;
    unsigned time = unsigned(animation_clock.getElapsedTime().asSeconds() * 4) % 8;
    Vector2u tile = tiles->getTileCoords(data->tiles_back[x][y] + time);
    va_back[t * 4 + 0].texCoords = Vector2f(tile.x, tile.y);
    va_back[t * 4 + 1].texCoords = Vector2f(tile.x, tile.y + TileSet::tile_size);
    va_back[t * 4 + 2].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y + TileSet::tile_size);
    va_back[t * 4 + 3].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y);
  }
  texture_back.clear(Color::Black);
  texture_back.draw(va_back, &tiles->texture);
  texture_back.display();

  for (unsigned i = 0; i < animated_front.size(); i++) {
    unsigned t = animated_front[i];
    unsigned x = t % data->width;
    unsigned y = t / data->width;
    unsigned time = (unsigned) (animation_clock.getElapsedTime().asSeconds() * 4) % 8;
    Vector2u tile = tiles->getTileCoords(data->tiles_front[x][y] + time);
    va_front[t * 4 + 0].texCoords = Vector2f(tile.x, tile.y);
    va_front[t * 4 + 1].texCoords = Vector2f(tile.x, tile.y + TileSet::tile_size);
    va_front[t * 4 + 2].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y + TileSet::tile_size);
    va_front[t * 4 + 3].texCoords = Vector2f(tile.x + TileSet::tile_size, tile.y);
  }
  texture_front.clear(Color::Transparent);
  texture_front.draw(va_front, &tiles->texture);
  texture_front.display();
}
