#pragma once

#include "arena.h"
#include "fat_pointer.h"
#include "tokens.h"
#include "vector.h"

#include <stdio.h>

typedef struct Tokenizer {
    FILE* input;
    Vector(Tokens) tokens;
} Tokenizer;
