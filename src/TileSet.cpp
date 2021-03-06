/*
 * TileSet.cpp
 *
 *  Created on: 25. Feb. 2016
 *      Author: Sebastian
 */

#include "TileSet.h"

#include <string>

#include "Utils/File.h"

using namespace std;
using namespace sf;

float TileSet::tile_size = 16;

TileSet::TileSet() {
  loaded = false;
}

TileSet::~TileSet() {
}

bool TileSet::loadFromFile(const char* fname) {
  if (texture.loadFromFile(fname)) {
    loaded = true;
    string ffname(fname);
    ffname = ffname.substr(0, ffname.find_last_of('.')) + ".flg";
    File f;
    if (f.open(ffname.c_str(), fstream::in | fstream::binary)) {
      while (!f.eof()) {
        Uint16 id = f.read<Uint16>();
        Uint8 fl = f.read<Uint8>();
        flags[id] = fl;
      }
    }
  }
  return loaded;
}

Vector2u TileSet::getTileCoords(Uint16 index) {
  if (!loaded) {
    return Vector2u();
  }
  unsigned w = texture.getSize().x / tile_size;
  return Vector2u((index % w) * tile_size, (index / w) * tile_size);
}

Uint8 TileSet::getTileFlags(Uint16 index) {
  if (flags.find(index) == flags.end()) {
    return 0;
  }
  return flags[index];
}
