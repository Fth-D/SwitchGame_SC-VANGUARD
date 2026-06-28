#pragma once
#include "GameObject.h"
#include "Game.h"

// ゲームの管理システムを操作するためのインターフェース（窓口）
// これらの関数を通じてゲーム側にアクセスしていきます。
// いちいちGame::GetInstance().AddObject()とか書いてられないからね。時短時短。

GameObject* AddObject(GameObject* newObject);
void DeleteObject(GameObject* deleteTarget);
void DeleteAllObject();

void Initialize();
void Finalize();

void Process();
void ShakeScreen(float intensity, float duration);
GameObject* FindNearestObject(float x, float y, ObjectType type);