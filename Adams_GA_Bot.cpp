#include <sc2api/sc2_api.h>
#include <sc2utils/sc2_manage_process.h>

#include <sc2api/sc2_interfaces.h>

//#include <bot_examples.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace sc2;

bool do_break = false;

class EmptyBot : public sc2::Agent {
public:
	virtual void OnGameStart() final {
		std::cout << "Game Started";
	}
};

class GABot : public sc2::Agent {
public:
	virtual void OnGameStart() final {
		std::cout << "Game Started";
		Debug()->DebugShowMap();
		Debug()->SendDebug();
		reset = false;
		chromosomes = new int*[population];
		fitnesses = new int[population];
		parents = new int*[population];
		goal.x = 100;
		goal.y = 8;
		start.x = 50;
		start.y = 8;

		for (int i = 0; i < population; i++)
		{
			chromosomes[i] = new int[chromSize];
			parents[i] = new int[chromSize];
		}

		initChroms();
		SpawnMarines();
	}

	virtual void OnStep() final {
			if (timer > 100 && chrom < population) {
				fitnesses[chrom] = fitness();
				chrom++;
				timer = 0;
				reset = true;
			}
			else if(reset && chrom < population){
				resetMap();
				SpawnMarines();
			}
			else if (chrom >= population) {
				selection();
				crossover();
				mutate();
				generation++;
				chrom = 0;
				if (generation > maxGeneration) {
					//output final chroms and fitnesses

					//reinit GA for next run
					initChroms();
					reset = true;
					generation = 0;
					run++;
					if (run > maxRuns) {
						//end GA and send output
						endGA();
					}
				}
			}
			else {
				sendOrders();
				timer++;
			}
	}

	void SpawnMarines() {
		if (!reset) {
			for (int iter = 0; iter < chromSize; iter++) {
				Debug()->DebugCreateUnit(UNIT_TYPEID::TERRAN_MARINE, start, Observation()->GetPlayerID(), 1);
				start.y += offset;
			}
			start.y = 8;
			Debug()->DebugMoveCamera(start);
			Debug()->SendDebug();
			spawned = true;
		}
	}

	void resetMap() {
		for (int iter = 0; iter < unitlist.size(); iter++)
		{
			Debug()->DebugKillUnit(unitlist[iter]);
		}
		Debug()->SendDebug();
		unitlist.clear();
		if (Observation()->GetUnits(Unit::Alliance::Self).size() < 1) {
			reset = false;
		}
	}

	void sendOrders() {
		if (spawned) {
			unitlist = Observation()->GetUnits(Unit::Alliance::Self);
			if (unitlist.size() > 0)
			{
				spawned = false;
			}
		}
		else {
			for (int iter = 0; iter < unitlist.size(); iter++)
			{
				if (chromosomes[chrom][iter] > 0) {
					Actions()->UnitCommand(unitlist[iter], ABILITY_ID::MOVE, goal);
				}
				else
				{
					Actions()->UnitCommand(unitlist[iter], ABILITY_ID::MOVE, start);
				}
				goal.y += 1;
				start.y += 1;
			}
			goal.y = 8;
			start.y = 8;
		}
	}

	void initChroms() {
		for (int i = 0; i < population; i++)
		{
			for (int j = 0; j < chromSize; j++)
			{
				chromosomes[i][j] = rand()%2;
			}
		}
	}

	int fitness() {
		int fit = 1000;
		for (int i = 0; i < chromSize; i++)
		{
			fit = fit - ((goal.x - unitlist[i]->pos.x) + (goal.y - unitlist[i]->pos.y));
		}
		return fit;
	}

	void selection() {
		int fit = 0;
		int maxFit = getMaxFit();
		int selectFit = 0;
		int iter = -1;
		for (int indiv = 0; indiv < population; indiv++)
		{
			//random select fitness value between 0 and max total fitnesss
			selectFit = GetRandomInteger(0, maxFit);
			//loop until individual is found
			do {
				iter++;
				fit = fit + fitnesses[iter];
			} while (fit < selectFit);
			//copy chromosome into parent list
			copyToParents(iter, indiv);
			//reset to select next parent
			iter = -1;
			fit = 0;
		}
	}

	void crossover() {
		int crossPoint = 0;
		for (int indiv = 0; indiv < population; indiv+=2) {
			//randomly select crossover point
			crossPoint = GetRandomInteger(0, chromSize);
			for (int iter = 0; iter < chromSize; iter++) {
				if (iter < crossPoint) {
					chromosomes[indiv][iter] = parents[indiv][iter];
					chromosomes[indiv+1][iter] = parents[indiv+1][iter];
				}
				else {
					chromosomes[indiv][iter] = parents[indiv + 1][iter];
					chromosomes[indiv + 1][iter] = parents[indiv][iter];
				}
			}
		}
	}

	void mutate() {
		int mut = 0;
		for (int indiv = 0; indiv < population; indiv++) {
			for (int i = 0; i < chromSize; i++){
				mut = GetRandomInteger(0, mutRate - 1);
				if (mut < 1) {
					chromosomes[indiv][i] = 1 - chromosomes[indiv][i];
				}
			}
		}
	}

	void copyToParents(int selected,int parent) {
		for (int i = 0; i < chromSize; i++) {
			parents[parent][i] = chromosomes[selected][i];
		}
	}

	int getMaxFit() {
		int fit = 0;
		for (int i = 0; i < population; i++)
		{
			fit = fit + fitnesses[i];
		}
		return fit;
	}

	void endGA() {
		//Debug()->DebugEndGame(true);
		//Debug()->DebugTestApp(Debug()->exit , 0);
		//Debug()->SendDebug();
		do_break = true;
	}

	void outputDataFiles() {
		std::ofstream fout;
		fout.open(outFile + ".txt");
	}

private:
	int chromSize = 10;
	int population = 10;
	int generation = 0;
	int maxGeneration = 1;
	int offset = 1;
	int chrom = 0;
	int run = 0;
	int maxRuns = 1;
	int crossRate = 70;
	int mutRate = 1000;
	
	bool spawned = false;
	bool reset = false;
	int timer = 0;

	sc2::Point2D goal;
	sc2::Point2D start;
	int unitCursor = 0;

	int **chromosomes;
	int *fitnesses;
	int **parents;
	Units unitlist;

	std::string outFile = "SC2_GA";

};

int main(int argc, char* argv[]) {
	sc2::Coordinator coordinator;
	if (!coordinator.LoadSettings(argc, argv)) {
		return 1;
	}
	coordinator.SetMultithreaded(true);
	coordinator.SetRealtime(false);
	//if (PlayerOneIsHuman) {
	//coordinator.SetRealtime(true);
	//}

	// Add the custom bot, it will control the players.
	EmptyBot eBot;
	GABot ga;

	sc2::Agent* player_one = &ga;

	coordinator.SetParticipants({
		CreateParticipant(sc2::Race::Terran, player_one),
		CreateComputer(sc2::Race::Protoss),// , &rBot2),
	});
	coordinator.LaunchStarcraft();
	do_break = false;
	while (!do_break) {
		if (!coordinator.StartGame(sc2::kMapEmpty)) {
			break;
		}
		while (coordinator.Update() && !do_break) {
			if (sc2::PollKeyPress()) {
				do_break = true;
			}
		}
	}

	player_one->Control()->DumpProtoUsage();
	//bot2.Control()->DumpProtoUsage();

	return 0;
}