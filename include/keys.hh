#pragma once

#define K_CTRL(k) ((k) & 0x1f)

enum keys {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
};