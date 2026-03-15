#include "Character.h"

// In case I want to change the parameters of these callbacks later
#define ITEM_USE_CALLBACK(name) bool name(Character *c)
#define ENTER_INVENTORY_CALLBACK(name) bool name(Character *c)
#define EXIT_INVENTORY_CALLBACK(name) bool name(Character *c)

ITEM_USE_CALLBACK(small_potion_callback) {
    c->current_hp += 5;
    create_label("+5 hp!", c->pos, (Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f, }, false);
    return true;
}
