#if !defined(INPUT_H)
#define INPUT_H

enum InputValue {
    NONE, UP, DOWN, LEFT, RIGHT, BUTTON
};

void input_init();
enum InputValue input_next();

#endif
