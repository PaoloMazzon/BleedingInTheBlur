#include "Character.h"

// In case I want to change the parameters of these callbacks later
#define ITEM_USE_CALLBACK(name) bool name(Character *c)
#define ENTER_INVENTORY_CALLBACK(name) bool name(Character *c)
#define EXIT_INVENTORY_CALLBACK(name) bool name(Character *c)

ITEM_USE_CALLBACK(small_potion_callback) {
    c->current_hp += 5;
    return true;
}
