#include "Game.h"
#include "Input.h"
#include "controller.h"

void Game::InitializeTitle()
{
	// ここにタイトルシーン初期化コードを書く
	Input::Bind(Command::Confirm, Button::A);
	Input::Bind(Command::Confirm, Key::Enter);
}

void Game::UpdateTitle(float dt)
{
	// ここにタイトルシーン更新コードを書く
	if (Input::IsTrigger(Command::Confirm))
	{
		nextScene = SAMPLE;
	}
}