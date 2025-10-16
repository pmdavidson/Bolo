#include "GameEngine.h"
#include "bolo/Bolo.h"
#include "bolo/Player.h"
#include "bolo/Base.h"
#include "bolo/Wall.h"

int main()
{
	CMPUT350::GameEngine engine(1440, 1080, "Bolo");
	engine.AddGameObject(std::make_shared<CMPUT350::Bolo>(58, 58, 256, 1)); // Bolo has a 58x58 map
	engine.Run();
	return 0;
}

// int main()
// {
// 	CMPUT350::GameEngine engine(1280, 1024, "Bolo");
	
// 	// Create the Bolo game manager
// 	auto bolo = std::make_shared<CMPUT350::Bolo>();
// 	engine.AddGameObject(bolo);
	
// 	engine.Run();

// 	return 0;
// }
