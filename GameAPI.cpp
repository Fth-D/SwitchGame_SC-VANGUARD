#include "GameAPI.h"

GameObject* AddObject(GameObject* newObject)
{
	return GameAPI.AddObject(newObject);
}

void DeleteObject(GameObject* deleteTarget)
{
	GameAPI.DeleteObject(deleteTarget);
}

void DeleteAllObject()
{
	GameAPI.DeleteAllObject();
}

void Process()
{
	GameAPI.Process();
}

void Initialize()
{
	GameAPI.Initialize();
}

void ShakeScreen(float intensity, float duration)
{
	GameAPI.ShakeScreen(intensity, duration);
}

GameObject* FindNearestObject(float x, float y, ObjectType type)
{
	return GameAPI.FindNearestObject(x, y, type);
}

void Finalize()
{
	GameAPI.Finalize();
}