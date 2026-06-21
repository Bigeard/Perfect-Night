#include "score.h"

static bool ScoreTeamIndexValid(int teamIndex)
{
    return teamIndex >= 0 && teamIndex < SCORE_TEAM_COUNT;
}

void ScoreResetAll(int scores[SCORE_TEAM_COUNT])
{
    for (int i = 0; i < SCORE_TEAM_COUNT; i++) scores[i] = -1;
}

void ScoreClearActive(int scores[SCORE_TEAM_COUNT])
{
    for (int i = 0; i < SCORE_TEAM_COUNT; i++)
    {
        if (scores[i] >= 0) scores[i] = 0;
    }
}

bool ScoreActivateTeam(int scores[SCORE_TEAM_COUNT], int teamIndex)
{
    if (!ScoreTeamIndexValid(teamIndex) || scores[teamIndex] >= 0) return false;
    scores[teamIndex] = 0;
    return true;
}

int ScoreCountActive(const int scores[SCORE_TEAM_COUNT])
{
    int count = 0;
    for (int i = 0; i < SCORE_TEAM_COUNT; i++) count += scores[i] >= 0;
    return count;
}

int ScoreCollectActive(const int scores[SCORE_TEAM_COUNT], int activeTeams[SCORE_TEAM_COUNT])
{
    int count = 0;
    for (int i = 0; i < SCORE_TEAM_COUNT; i++)
    {
        if (scores[i] >= 0) activeTeams[count++] = i;
    }
    return count;
}

int ScoreResolveAliveTeam(const bool aliveTeams[SCORE_TEAM_COUNT])
{
    int result = SCORE_NO_TEAM;
    for (int i = 0; i < SCORE_TEAM_COUNT; i++)
    {
        if (!aliveTeams[i]) continue;
        if (result != SCORE_NO_TEAM) return SCORE_MULTIPLE_TEAMS;
        result = i;
    }
    return result;
}

bool ScoreRoundHasResult(int activeTeamCount, int aliveTeam)
{
    return activeTeamCount > 1 && aliveTeam != SCORE_MULTIPLE_TEAMS;
}

int ScoreBest(const int scores[SCORE_TEAM_COUNT])
{
    int best = 0;
    for (int i = 0; i < SCORE_TEAM_COUNT; i++)
    {
        if (scores[i] > best) best = scores[i];
    }
    return best;
}

bool ScoreAwardTeam(int scores[SCORE_TEAM_COUNT], int teamIndex, int maxScore, int *bestScore)
{
    if (!ScoreTeamIndexValid(teamIndex) || scores[teamIndex] < 0) return false;
    scores[teamIndex]++;
    if (bestScore && scores[teamIndex] > *bestScore) *bestScore = scores[teamIndex];
    return scores[teamIndex] >= maxScore;
}

void ScoreDrawResult(Vector2 center, float arenaWidth, Color backgroundColor,
                     const int scores[SCORE_TEAM_COUNT], const Color teamColors[SCORE_TEAM_COUNT],
                     int maxScore, int bestScore, bool isDraw, int winningTeam)
{
    Color scoreBackground = Fade(backgroundColor, 0.9f);
    if (maxScore <= bestScore) scoreBackground = Fade(GOLD, 0.1f);

    const Rectangle scoreBar = {center.x - arenaWidth*2.0f, center.y - 10.0f, arenaWidth*4.0f, 110.0f};
    DrawRectangleRec(scoreBar, scoreBackground);
    if (maxScore <= bestScore + 1)
    {
        DrawRectangleLinesEx(scoreBar, 2.5f, Fade(GOLD, 0.3f));
    }

    int activeTeams[SCORE_TEAM_COUNT] = {0};
    const int activeCount = ScoreCollectActive(scores, activeTeams);
    for (int slot = 0; slot < activeCount; slot++)
    {
        const int teamIndex = activeTeams[slot];
        const char *textScore = TextFormat("%d", scores[teamIndex]);
        const int textScoreSize = MeasureText(textScore, 100);
        const float scoreX = center.x + ((float)slot - ((float)activeCount - 1.0f)/2.0f)*320.0f;
        DrawText(textScore, (int)(scoreX - (float)textScoreSize/2.0f), (int)center.y, 100, teamColors[teamIndex]);
    }

    if (isDraw)
    {
        const char *drawText = "DRAW";
        DrawText(drawText, (int)(center.x - MeasureText(drawText, 100)/2.0f), (int)(center.y - 240.0f), 100, LIGHTGRAY);
        return;
    }

    if (!ScoreTeamIndexValid(winningTeam)) return;
    DrawCircle((int)center.x, (int)(center.y - 300.0f), 50.0f, BLACK);
    DrawCircle((int)center.x, (int)(center.y - 300.0f), 48.0f, WHITE);
    DrawCircle((int)center.x, (int)(center.y - 300.0f), 40.0f, teamColors[winningTeam]);
    DrawText("WINS THIS GAME", (int)(center.x - 50.0f*7.0f), (int)(center.y - 200.0f), 80, teamColors[winningTeam]);
}
