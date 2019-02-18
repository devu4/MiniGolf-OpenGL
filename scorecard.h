#pragma once
#include <vector>
#include <string>
#include <glui.h>

class Player;

#define SCORECARD_ROW_WIDTH 5
#define SCORECARD_OK 82919
#define NO_SCORE -1

//create Scorecard GUI
static GLUI *CreateScoreCard(std::vector<Player> players, GLUI_CB callback) {
	GLUI *scorecard;
	GLUI_Panel *panel;

	int i, j, firstHole, endHole, score;
	int numHoles = gCourse->holes.size();
	int numPlayers = gCourse->players.size();

	std::string holeText;

	scorecard = GLUI_Master.create_glui("Scorecard");

	for (firstHole = 0; firstHole < numHoles; firstHole += SCORECARD_ROW_WIDTH)
	{
		endHole = firstHole + SCORECARD_ROW_WIDTH - 1;
		if (endHole >= numHoles) {
			endHole = numHoles - 1;
		}

		panel = scorecard->add_panel("");
		panel->set_alignment(GLUI_ALIGN_LEFT);

		scorecard->add_statictext_to_panel(panel, "Hole");
		scorecard->add_separator_to_panel(panel);
		for (i = 0; i<numPlayers; i++) {
			scorecard->add_statictext_to_panel(panel, players[i].name.c_str());
		}

		for (i = firstHole; i <= endHole; i++) {
			scorecard->add_column_to_panel(panel, true);
			holeText = std::to_string(i + 1);
			scorecard->add_statictext_to_panel(panel, holeText.c_str());
			scorecard->add_separator_to_panel(panel);
			for (j = 0; j<numPlayers; j++) {
				if (players[j].scores[i] == NO_SCORE) {
					scorecard->add_statictext_to_panel(panel, "-");
				}
				else {
					holeText = std::to_string(players[j].scores[i]);
					scorecard->add_statictext_to_panel(panel, holeText.c_str());
				}
				
			}
		}
	}

	panel = scorecard->add_panel("Scores");
	for (i = 0; i<numPlayers; i++) {
			scorecard->add_statictext_to_panel(panel, players[i].name.c_str());
	}
	scorecard->add_column_to_panel(panel, true);
	for (i = 0; i<numPlayers; i++) {
		/* score */
		score = 0;
		for (j = 0; j<numHoles; j++) {
			if (players[i].scores[j] != NO_SCORE) {
				score += players[i].scores[j];
			}
		}
		holeText = std::to_string(score);
		scorecard->add_statictext_to_panel(panel, holeText.c_str());
		
	}

	scorecard->add_button("OK", SCORECARD_OK, callback);

	return scorecard;
};

static std::vector<int> GetWinnerID() {
	struct scores{
		int id;
		int score;
	};
	std::vector<scores> totalScores;

	for (size_t i = 0; i < gCourse->players.size(); i++)
	{
		scores score;
		score.id = i;
		score.score = 0;
		for (size_t j = 0; j<gCourse->holes.size(); j++) {
			if (gCourse->players[i].scores[j] != NO_SCORE) {
				score.score += gCourse->players[i].scores[j];
			}
		}

		totalScores.push_back(score);
	}

	std::sort(totalScores.begin(), totalScores.end(), [](scores const & a, scores const & b) -> bool
	{ return a.score < b.score; });

	std::vector<int> IDs;

	IDs.push_back(totalScores[0].id);

	for (size_t j = 1; j<totalScores.size(); j++) {
		if (totalScores[j].score <= totalScores[0].score) {
			IDs.push_back(totalScores[j].id);
		}
	}

	return IDs;

};
