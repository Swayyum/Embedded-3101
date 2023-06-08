#define RED_MASK 0x4
#define GREEN_MASK 0x2
#define BLUE_MASK 0x1

#define RED_SHIFT 2
#define GREEN_SHIFT 1
#define BLUE_SHIFT 0

// represents a state machine that cycles between these colors:
// |-> black -> red -> purple -> green -> cyan -> blue -> yellow -> white |-> black

// the colors are presented as a 3-bit binary number corresponding to "RGB"

char state = 0; // starting state is RGB = 000, which is black or no light

// returns the next state of the mealy state machine
// if button is pressed (aka = 1), switch to the next state
char nextState(char currState, bool button){
    bool currRed = currState & RED_MASK;
    bool currGreen = currState & GREEN_MASK;
    bool currBlue = currState & BLUE_MASK;

    // logic for the next state
    bool newRed = (currRed && !(currBlue & button)) || (!currRed && !currGreen & button);
    bool newGreen = (currGreen && !(currBlue & button)) || (!currGreen && currBlue && button);
    bool newBlue = (currBlue && !button) + (!currRed && currGreen && button) || (currRed && !currBlue && button);

    char newState = (newRed << RED_SHIFT) | (newGreen << GREEN_SHIFT) | (newBlue << BLUE_SHIFT);

    return newState;
}

// returns the next state of the mealy state machine, assumes the button is already pressed
// char nextState(char currState){
//     return nextState(currState, true);
// }
