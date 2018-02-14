#include <GameStart.h>
#include <sc2api/sc2_api.h>
#include <sc2utils/sc2_manage_process.h>

#include <sc2api\sc2_interfaces.h>

//using namespace sc2;

int GameStart(sc2::Coordinator coordinator) {
	// Start the game.
	coordinator.LaunchStarcraft();

	bool do_break = false;
	while (!do_break) {
		if (!coordinator.StartGame(sc2::kMapTesting)) {
			break;
		}
		while (coordinator.Update() && !do_break) {
			if (sc2::PollKeyPress()) {
				do_break = true;
			}
		}
	}

	return 0;
}