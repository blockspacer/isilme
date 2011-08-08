﻿#include "Isilme.h"
#include "Engine/ScriptAPI.h"

Engine* Engine::mInstance = 0;

Engine::Engine() :
	mResourceManager(0),
	mInputSystem(0),
	mLua(0)
{
	// Инициализируем Lua
	mLua = new Lua();
	
	// Инициализируем подсистему ввода
	SetInputSystem(new HGEInputSystem(mHGE));

	// Запоминаем себя
	mInstance = this;

	// Регистрируем ScripAPI
	ScriptAPI::RegisterEngineAPI();
}

Engine::~Engine()
{
	if (mHGE)
	{
		mHGE->Release();
		mHGE = 0;
	}

	if (mResourceManager)
	{
		delete mResourceManager;
		mResourceManager = 0;
	}
}

InputSystem* Engine::GetInputSystem()
{
	return mInputSystem;
}

void	Engine::SetInputSystem(InputSystem* inputSystem)
{
	mInputSystem = inputSystem;
}

Lua*	Engine::GetLua()
{
	return mLua;
}

void Engine::ShowLogo()
{
	Sleep(200);
	mLogo = new LogoState(new hgeSprite(mResourceManager->GetTexture("../Data/Textures/Logo.png"),0,0,954, 755));
	mStateManager->Push(StatePtr(mLogo));
	mHGE->System_Start();
	Sleep(200);
}

ResourceManager* Engine::GetResourceManager()
{
	return mResourceManager;
}

void	Engine::SetResourceManager(ResourceManager* resourceManager)
{
	mResourceManager = resourceManager;
}

Engine*	Engine::GetSingleton()
{
	if (mInstance)
		return mInstance;

	mInstance = new Engine();
	return mInstance;
}

SoundSystem*	Engine::GetSoundSystem()
{
	return mSoundSystem;
}

void			Engine::SetSoundSystem(SoundSystem* soundSystem)
{
	mSoundSystem = soundSystem;
}