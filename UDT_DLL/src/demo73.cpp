#include "demo73.hpp"

#include <cstdlib>


// Offset from the start of the structure == absolute address when the struct is at address 0.
#define ESF(x) #x, (int)&((entityState_73_t*)0)->x

const netField_t Demo73::EntityStateFields[] =
{
	{ ESF(pos.trTime), 32 },
	{ ESF(pos.trBase[0]), 0 },
	{ ESF(pos.trBase[1]), 0 },
	{ ESF(pos.trDelta[0]), 0 },
	{ ESF(pos.trDelta[1]), 0 },
	{ ESF(pos.trBase[2]), 0 },
	{ ESF(apos.trBase[1]), 0 },
	{ ESF(pos.trDelta[2]), 0 },
	{ ESF(apos.trBase[0]), 0 },
	{ ESF(pos.gravity), 32 },
	{ ESF(event), 10 },
	{ ESF(angles2[1]), 0 },
	{ ESF(eType), 8 },
	{ ESF(torsoAnim), 8 },
	{ ESF(eventParm), 8 },
	{ ESF(legsAnim), 8 },
	{ ESF(groundEntityNum), GENTITYNUM_BITS },
	{ ESF(pos.trType), 8 },
	{ ESF(eFlags), 19 },
	{ ESF(otherEntityNum), GENTITYNUM_BITS },
	{ ESF(weapon), 8 },
	{ ESF(clientNum), 8 },
	{ ESF(angles[1]), 0 },
	{ ESF(pos.trDuration), 32 },
	{ ESF(apos.trType), 8 },
	{ ESF(origin[0]), 0 },
	{ ESF(origin[1]), 0 },
	{ ESF(origin[2]), 0 },
	{ ESF(solid), 24 },
	{ ESF(powerups), 16 },
	{ ESF(modelindex), 8 },
	{ ESF(otherEntityNum2), GENTITYNUM_BITS },
	{ ESF(loopSound), 8 },
	{ ESF(generic1), 8 },
	{ ESF(origin2[2]), 0 },
	{ ESF(origin2[0]), 0 },
	{ ESF(origin2[1]), 0 },
	{ ESF(modelindex2), 8 },
	{ ESF(angles[0]), 0 },
	{ ESF(time), 32 },
	{ ESF(apos.trTime), 32 },
	{ ESF(apos.trDuration), 32 },
	{ ESF(apos.trBase[2]), 0 },
	{ ESF(apos.trDelta[0]), 0 },
	{ ESF(apos.trDelta[1]), 0 },
	{ ESF(apos.trDelta[2]), 0 },
	{ ESF(apos.gravity), 32 },
	{ ESF(time2), 32 },
	{ ESF(angles[2]), 0 },
	{ ESF(angles2[0]), 0 },
	{ ESF(angles2[2]), 0 },
	{ ESF(constantLight), 32 },
	{ ESF(frame), 16 }
};

#undef ESF

const int Demo73::EntityStateFieldCount = sizeof(Demo73::EntityStateFields) / sizeof(Demo73::EntityStateFields[0]);


Demo73::Demo73() : Demo()
{
}

Demo73::~Demo73()
{
}

void Demo73::ProtocolInit()
{
	_inParseEntities.resize(MAX_PARSE_ENTITIES);
	_inEntityBaselines.resize(MAX_PARSE_ENTITIES);
	memset(&_inEntityBaselines[0], 0, MAX_PARSE_ENTITIES * sizeof(entityState_73_t));
}

void Demo73::ProtocolParseBaseline(msg_t* msg, msg_t* msgOut)
{
	ParseBaselineT<Demo73, entityState_73_t>(msg, msgOut);
}

void Demo73::ProtocolParsePacketEntities(msg_t* msg, msg_t* msgOut, clSnapshot_t* oldframe, clSnapshot_t* newframe)
{
	ParsePacketEntitiesT<Demo73, entityState_73_t>(msg, msgOut, oldframe, newframe);
}

void Demo73::ProtocolEmitPacketEntities(clSnapshot_t* from, clSnapshot_t* to)
{
	EmitPacketEntitiesT<Demo73, entityState_73_t>(from, to);
}

void Demo73::ProtocolAnalyzeConfigString(int csIndex, const std::string& input)
{
	if(csIndex == CS_WARMUP)
	{
		int startTime = -1;
		if(GetVariable(input, "time", &startTime) && startTime > 0)
		{
			_gameStartTime = startTime;
		}
	}
	else if(csIndex == CS_WARMUP_END)
	{
		const int startTime = atoi(input.c_str());
		if(startTime > 0)
		{
			std::string val;
			GetVariable(val, input, "g_gameState");
			const bool inProgress = val == "IN_PROGRESS";

			if(inProgress || (!inProgress && _gameStartTime < startTime))
			{
				_gameStartTime = startTime;
			}
		}
	}
	else if(csIndex >= CS_PLAYERS_73 && csIndex < CS_PLAYERS_73 + MAX_CLIENTS)
	{
		AnalyzePlayerInfo(csIndex - CS_PLAYERS_73, input);
	}
	else if(csIndex == CS_SCORES1)
	{
		ReadScore(input.c_str(), &_score1);
	}
	else if(csIndex == CS_SCORES2)
	{
		ReadScore(input.c_str(), &_score2);
	}
}

void Demo73::ProtocolFixConfigString(int /*csIndex*/, const std::string& input, std::string& output)
{
	output = input;
}

void Demo73::ProtocolAnalyzeAndFixCommandString(const char* command, std::string& output)
{
	int csIndex = -1;
	std::string configString;
	if(ExtractConfigStringFromCommand(csIndex, configString, command))
	{
		_inConfigStrings[csIndex] = configString;

		// Players.
		if(csIndex >= CS_PLAYERS_73 && csIndex < CS_PLAYERS_73 + MAX_CLIENTS)
		{
			ProtocolAnalyzeConfigString(csIndex, command);
			output = command;
		}
		// Scores.
		else if(csIndex == CS_SCORES1)
		{
			ReadScore(configString.c_str(), &_score1);
		}
		// Scores.
		else if(csIndex == CS_SCORES2)
		{
			ReadScore(configString.c_str(), &_score2);
		}
	}
	// Chat messages.
	else if(strstr(command, "chat") == command)
	{
		_tokenizer.Tokenize(command);
		if(_tokenizer.argc() >= 2 && strcmp(_tokenizer.argv(0), "chat") == 0)
		{
			const int currentTime = GetVirtualInputTime();
			const char* const chatMsg = _tokenizer.argv(1);
			_chatMessages[currentTime] = chatMsg;
		}
		output = command;
	}
	else
	{
		output = command;
	}
}

void Demo73::ProtocolAnalyzeSnapshot(const clSnapshot_t* oldSnap, const clSnapshot_t* newSnap)
{
	AnalyzeSnapshotT<Demo73, entityState_73_t>(oldSnap, newSnap);
}

void Demo73::AnalyzePlayerInfo(int clientNum, const std::string& configString)
{
	if(clientNum < 0 || clientNum > MAX_CLIENTS)
	{
		return;
	}

	PlayerInfoPers* const player = &_players[clientNum];
	if(configString.find("\"\"") != std::string::npos)
	{
		Q_strncpyz(player->Info.Name, "<empty_slot>", sizeof(player->Info.Name));
		_players[clientNum].Valid = false;
		return;
	}

	std::string name, clan, country;
	ExtractPlayerNameFromConfigString(name, configString);
	GetVariable(clan, configString, "xcn");
	GetVariable(country, configString, "c");

	player->Valid = true;
	Q_strncpyz(player->Info.Name, name.c_str(), sizeof(player->Info.Name));
	Q_strncpyz(player->Info.Clan, clan.c_str(), sizeof(player->Info.Clan));
	Q_strncpyz(player->Info.Country, country.c_str(), sizeof(player->Info.Country));
	TryGetVariable(&player->Info.Handicap, configString, "hc");
	TryGetVariable(&player->Info.Team, configString, "t");
	TryGetVariable(&player->Info.BotSkill, configString, "l"); // @TODO: Correct?
}