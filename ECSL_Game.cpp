#include <sc2api/sc2_api.h>
#include <sc2utils/sc2_manage_process.h>

#include <sc2api\sc2_interfaces.h>

#include <GameStart.h>

//#include <bot_examples.h>

#include <iostream>
#include <fstream>

using namespace sc2;

class EmptyBot : public sc2::Agent {
public:
	virtual void OnGameStart() final {
		std::cout << "Game Started";
	}
};

int main(int argc, char* argv[]) {
	sc2::Coordinator coordinator;
	if (!coordinator.LoadSettings(argc, argv)) {
		return 1;
	}

	coordinator.SetMultithreaded(true);
	//if (PlayerOneIsHuman) {
	coordinator.SetRealtime(true);
	//}

	// Add the custom bot, it will control the players.
	//MyFirstBot bot1, bot2;
	//Human human_bot;
	EmptyBot eBot;
	//RahulBot rBot1, rBot2;

	sc2::Agent* player_one = &eBot;

	coordinator.SetParticipants({
		CreateParticipant(sc2::Race::Terran, player_one),
		CreateComputer(sc2::Race::Protoss),// , &rBot2),
	});
	//Call GA here
	GameStart(coordinator);


	player_one->Control()->DumpProtoUsage();
	//bot2.Control()->DumpProtoUsage();
}