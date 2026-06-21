#ifndef SCORE_H
#define SCORE_H

#include <stdbool.h>
#include "raylib.h"

#define SCORE_TEAM_COUNT 8
#define SCORE_NO_TEAM -1
#define SCORE_MULTIPLE_TEAMS -2

void ScoreResetAll(int scores[SCORE_TEAM_COUNT]);
void ScoreClearActive(int scores[SCORE_TEAM_COUNT]);
bool ScoreActivateTeam(int scores[SCORE_TEAM_COUNT], int teamIndex);
int ScoreCountActive(const int scores[SCORE_TEAM_COUNT]);
int ScoreCollectActive(const int scores[SCORE_TEAM_COUNT], int activeTeams[SCORE_TEAM_COUNT]);
int ScoreResolveAliveTeam(const bool aliveTeams[SCORE_TEAM_COUNT]);
bool ScoreRoundHasResult(int activeTeamCount, int aliveTeam);
int ScoreBest(const int scores[SCORE_TEAM_COUNT]);
bool ScoreAwardTeam(int scores[SCORE_TEAM_COUNT], int teamIndex, int maxScore, int *bestScore);
void ScoreDrawResult(Vector2 center, float arenaWidth, Color backgroundColor,
                     const int scores[SCORE_TEAM_COUNT], const Color teamColors[SCORE_TEAM_COUNT],
                     int maxScore, int bestScore, bool isDraw, int winningTeam);

#endif
