/// \brief
#pragma once
#include <oct/cJSON.h>
#include "Structs.h"

void input_init(cJSON *load_from);
void input_save(cJSON *serialize_into);
void input_quit();
void input_update();
bool input_get(Input input);
