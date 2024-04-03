#pragma once

#include <cstdint>

namespace ttyd::database {
	
struct DatabaseDefinition
{
	const char *name;
	int32_t id;
} ;

extern "C" {

void setupDataLoad(const char *mapName);
int32_t setupDataCheck();
void setupDataBase(const char *areaName, const char *mapName);

}

}