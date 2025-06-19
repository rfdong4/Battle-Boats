/* 
 * File:   Field.c
 * Author: Ryan Dong (rfdong@ucsc.edu)
 *
 * Created on December 6, 2023, 1:13 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "Field.h"
#include "Board.h"
#include "FieldOled.h"
/*
 * .
 */
void FieldPrint_UART(Field *own_field, Field * opp_field) {
    int i = 0;
    int j = 0;
    //for loop to print ith and jth rows
    for (i = 0; i < FIELD_ROWS; i++) {
        for (j = 0; j < FIELD_COLS; j++) {
            if (own_field->grid[i][j] == FIELD_SQUARE_EMPTY) {
                printf(" [ ]");
               
            } else if (own_field->grid[i][j] == FIELD_SQUARE_UNKNOWN) {
                printf(" [|]");
            } else if (own_field->grid[i][j] == FIELD_SQUARE_HIT) {
                printf(" [x]");
                
            } else if (own_field->grid[i][j] == FIELD_SQUARE_MISS) {
                printf(" [M]");
            }
            else {
                printf(" [0]");
            }
        }
        printf("\n");
    }
    printf("\n");
    for (i = 0; i < FIELD_ROWS; i++) {
        for (j = 0; j < FIELD_COLS; j++) {
            if (opp_field->grid[i][j] == FIELD_SQUARE_EMPTY) {
                printf(" [ ]");
            } else if (opp_field->grid[i][j] == FIELD_SQUARE_UNKNOWN) {
                printf(" [|]");
            }
        }
        printf("\n");
    }
}
/**
 * FieldInit() will initialize two passed field structs for the beginning of play.
 * Each field's grid should be filled with the appropriate SquareStatus (
 * FIELD_SQUARE_EMPTY for your own field, FIELD_SQUARE_UNKNOWN for opponent's).
 * Additionally, your opponent's field's boatLives parameters should be filled
 *  (your own field's boatLives will be filled when boats are added)
 * 
 * FieldAI_PlaceAllBoats() should NOT be called in this function.
 * 
 * @param own_field     //A field representing the agents own ships
 * @param opp_field     //A field representing the opponent's ships
 */
void FieldInit(Field *own_field, Field * opp_field) {
    int i, j;
    for (i = 0; i < FIELD_ROWS; i++) {
        for (j = 0; j < FIELD_COLS; j++) {
            own_field->grid[i][j] = FIELD_SQUARE_EMPTY;
        }
    }
    for (i = 0; i < FIELD_ROWS; i++) {
        for (j = 0; j < FIELD_COLS; j++) {
            opp_field->grid[i][j] = FIELD_SQUARE_UNKNOWN;
        }
    }
    opp_field->smallBoatLives = FIELD_BOAT_SIZE_SMALL;
    opp_field->mediumBoatLives = FIELD_BOAT_SIZE_MEDIUM;
    opp_field->largeBoatLives = FIELD_BOAT_SIZE_LARGE;
    opp_field->hugeBoatLives = FIELD_BOAT_SIZE_HUGE;
}
/**
 * Retrieves the value at the specified field position.
 * @param f     //The Field being referenced
 * @param row   //The row-component of the location to retrieve
 * @param col   //The column-component of the location to retrieve
 * @return  FIELD_SQUARE_INVALID if row and col are not valid field locations
 *          Otherwise, return the status of the referenced square 
 */
SquareStatus FieldGetSquareStatus(const Field *f, uint8_t row, uint8_t col) {
    if ((row > FIELD_ROWS) || (col > FIELD_COLS)) {
        return FIELD_SQUARE_INVALID;
    }
    if ((row < 0) || (col < 0)) {
        return FIELD_SQUARE_INVALID;
    }
    return f->grid[row][col];
    
}
/**
 * This function provides an interface for setting individual locations within a Field struct. This
 * is useful when FieldAddBoat() doesn't do exactly what you need. For example, if you'd like to use
 * FIELD_SQUARE_CURSOR, this is the function to use.
 * 
 * @param f The Field to modify.
 * @param row The row-component of the location to modify
 * @param col The column-component of the location to modify
 * @param p The new value of the field location
 * @return The old value at that field location
 */
SquareStatus FieldSetSquareStatus(Field *f, uint8_t row, uint8_t col, SquareStatus p) {
    SquareStatus status = f->grid[row][col];
    f->grid[row][col] = p;
    return status;
}
/**
 * FieldAddBoat() places a single ship on the player's field based on arguments 2-5. Arguments 2, 3
 * represent the x, y coordinates of the pivot point of the ship.  Argument 4 represents the
 * direction of the ship, and argument 5 is the length of the ship being placed. 
 * 
 * All spaces that
 * the boat would occupy are checked to be clear before the field is modified so that if the boat
 * can fit in the desired position, the field is modified as SUCCESS is returned. Otherwise the
 * field is unmodified and STANDARD_ERROR is returned. There is no hard-coded limit to how many
 * times a boat can be added to a field within this function.
 * 
 * In addition, this function should update the appropriate boatLives parameter of the field.
 *
 * So this is valid test code:
 * {
 *   Field myField;
 *   FieldInit(&myField,FIELD_SQUARE_EMPTY);
 *   FieldAddBoat(&myField, 0, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_TYPE_SMALL);
 *   FieldAddBoat(&myField, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_TYPE_MEDIUM);
 *   FieldAddBoat(&myField, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_TYPE_HUGE);
 *   FieldAddBoat(&myField, 0, 6, FIELD_BOAT_DIRECTION_SOUTH, FIELD_BOAT_TYPE_SMALL);
 * }
 *
 * should result in a field like:
 *      0 1 2 3 4 5 6 7 8 9
 *     ---------------------
 *  0 [ 3 3 3 . . . 3 . . . ]
 *  1 [ 4 4 4 4 . . 3 . . . ]
 *  2 [ . . . . . . 3 . . . ]
 *  3 [ . . . . . . . . . . ]
 *  4 [ . . . . . . . . . . ]
 *  5 [ . . . . . . . . . . ]
 *     
 * @param f The field to grab data from.
 * @param row The row that the boat will start from, valid range is from 0 and to FIELD_ROWS - 1.
 * @param col The column that the boat will start from, valid range is from 0 and to FIELD_COLS - 1.
 * @param dir The direction that the boat will face once places, from the BoatDirection enum.
 * @param boatType The type of boat to place. Relies on the FIELD_SQUARE_*_BOAT values from the
 * SquareStatus enum.
 * @return SUCCESS for success, STANDARD_ERROR for failure
 */
uint8_t FieldAddBoat(Field *own_field, uint8_t row, uint8_t col, BoatDirection dir, BoatType boat_type) {
    if ((row >= FIELD_ROWS) || (col >= FIELD_COLS)) {
        return STANDARD_ERROR;
    }
    if ((row < 0) || (col < 0)) {
        return STANDARD_ERROR;
    }
    int boatLength = 0;
    SquareStatus boatType;
    if (boat_type == FIELD_BOAT_TYPE_SMALL) {
        boatLength = FIELD_BOAT_SIZE_SMALL;
        boatType = FIELD_SQUARE_SMALL_BOAT;
    }
    else if (boat_type == FIELD_BOAT_TYPE_MEDIUM) {
        boatLength = FIELD_BOAT_SIZE_MEDIUM;
        boatType = FIELD_SQUARE_MEDIUM_BOAT;
    }
    else if (boat_type == FIELD_BOAT_TYPE_LARGE) {
        boatLength = FIELD_BOAT_SIZE_LARGE;
        boatType = FIELD_SQUARE_LARGE_BOAT;
    }
    else if (boat_type == FIELD_BOAT_TYPE_HUGE)
    {
        boatLength = FIELD_BOAT_SIZE_HUGE;
        boatType = FIELD_SQUARE_HUGE_BOAT;
    }
    
    if (dir == FIELD_DIR_EAST) {
        if (col + boatLength - 1 >= FIELD_COLS) {
            return STANDARD_ERROR;
        }
        for (int i = 0; i < boatLength; i++) {
            if (own_field->grid[row][col + i] != FIELD_SQUARE_EMPTY) {
                return STANDARD_ERROR;
            }
        }
        for (int i = 0; i < (boatLength); i++) {
            own_field->grid[row][col + i] = boatType;
        }
    }
    else if (dir == FIELD_DIR_SOUTH) {
        if (row + boatLength - 1 >= FIELD_ROWS) {
            return STANDARD_ERROR;
        }
        for (int i = 0; i < boatLength; i++) {
            if (own_field->grid[row + i][col] != FIELD_SQUARE_EMPTY) {
                return STANDARD_ERROR;
            }
        }
        for (int i = 0; i < (boatLength); i++) {
            own_field->grid[row + i][col] = boatType;
        }
    }
    else {
        return STANDARD_ERROR;
    }
    switch (boatType) {
        case FIELD_BOAT_TYPE_SMALL:
            own_field->smallBoatLives = FIELD_BOAT_SIZE_SMALL;
            break;
        case FIELD_BOAT_TYPE_MEDIUM:
            own_field->mediumBoatLives = FIELD_BOAT_SIZE_MEDIUM;
            break;
        case FIELD_BOAT_TYPE_LARGE:
            own_field->largeBoatLives = FIELD_BOAT_SIZE_LARGE;
            break;
        case FIELD_BOAT_TYPE_HUGE:
            own_field->hugeBoatLives = FIELD_BOAT_SIZE_HUGE;
            break;
    }
    if (boat_type == FIELD_BOAT_TYPE_SMALL || boat_type == FIELD_BOAT_SIZE_MEDIUM || boat_type == FIELD_BOAT_TYPE_LARGE || boat_type == FIELD_BOAT_TYPE_HUGE) {
        ;
    }
    else {
        return STANDARD_ERROR;
    }
    return SUCCESS;
}
/**
 * This function registers an attack at the gData coordinates on the provided field. This means that
 * 'f' is updated with a FIELD_SQUARE_HIT or FIELD_SQUARE_MISS depending on what was at the
 * coordinates indicated in 'gData'. 'gData' is also updated with the proper HitStatus value
 * depending on what happened AND the value of that field position BEFORE it was attacked. Finally
 * this function also reduces the lives for any boat that was hit from this attack.
 * @param f The field to check against and update.
 * @param gData The coordinates that were guessed. The result is stored in gData->result as an
 *               output.  The result can be a RESULT_HIT, RESULT_MISS, or RESULT_***_SUNK.
 * @return The data that was stored at the field position indicated by gData before this attack.
 */
SquareStatus FieldRegisterEnemyAttack(Field *own_field, GuessData *opp_guess) {
    uint8_t col = opp_guess->col;
    uint8_t row = opp_guess->row;
    SquareStatus squareStatus = FieldGetSquareStatus(own_field, row, col);
    if (squareStatus != FIELD_SQUARE_EMPTY && squareStatus != FIELD_SQUARE_HIT && squareStatus != FIELD_SQUARE_MISS) {
        own_field->grid[row][col] == FIELD_SQUARE_HIT;
        switch (squareStatus) {
            case FIELD_SQUARE_SMALL_BOAT:
                if ((own_field->smallBoatLives - 1) == 0) {
                    own_field->smallBoatLives -= 1;
                    opp_guess->result = RESULT_SMALL_BOAT_SUNK;
                }
                else {
                    own_field->smallBoatLives -= 1;
                    opp_guess->result = RESULT_HIT;
                }
                break;
            case FIELD_SQUARE_MEDIUM_BOAT:
                if ((own_field->mediumBoatLives - 1) == 0) {
                    own_field->mediumBoatLives -= 1;
                    opp_guess->result = RESULT_MEDIUM_BOAT_SUNK;
                }
                else {
                    own_field->mediumBoatLives -= 1;
                    opp_guess->result = RESULT_HIT;
                }
                break;
            case FIELD_SQUARE_LARGE_BOAT:
                if ((own_field->largeBoatLives - 1) == 0) {
                    own_field->largeBoatLives -= 1;
                    opp_guess->result = RESULT_LARGE_BOAT_SUNK;
                }
                else {
                    own_field->largeBoatLives -= 1;
                    opp_guess->result = RESULT_HIT;
                }
                break;
            case FIELD_SQUARE_HUGE_BOAT:
                if ((own_field->hugeBoatLives - 1) == 0) {
                    own_field->hugeBoatLives -= 1;
                    opp_guess->result = RESULT_HUGE_BOAT_SUNK;
                }
                else {
                    own_field->hugeBoatLives -= 1;
                    opp_guess->result = RESULT_HIT;
                }
                break;
        }   
    }
    else if (own_field->grid[row][col] == FIELD_SQUARE_EMPTY) {
        own_field->grid[row][col] == FIELD_SQUARE_MISS;
        opp_guess->result = RESULT_MISS;
    }
    return squareStatus;    
    }
    
   
/**
 * This function updates the FieldState representing the opponent's game board with whether the
 * guess indicated within gData was a hit or not. If it was a hit, then the field is updated with a
 * FIELD_SQUARE_HIT at that position. If it was a miss, display a FIELD_SQUARE_EMPTY instead, as
 * it is now known that there was no boat there. The FieldState struct also contains data on how
 * many lives each ship has. Each hit only reports if it was a hit on any boat or if a specific boat
 * was sunk, this function also clears a boats lives if it detects that the hit was a
 * RESULT_*_BOAT_SUNK.
 * @param f The field to grab data from.
 * @param gData The coordinates that were guessed along with their HitStatus.
 * @return The previous value of that coordinate position in the field before the hit/miss was
 * registered.
 */
SquareStatus FieldUpdateKnowledge(Field *opp_field, const GuessData *own_guess) {
    int col = own_guess->col;
    int row = own_guess->row;
    SquareStatus squareStatus = FieldGetSquareStatus(opp_field, row, col);
    switch (own_guess->result) {
        case RESULT_HIT:
            opp_field->grid[row][col] = FIELD_SQUARE_HIT;
            break;
        case RESULT_SMALL_BOAT_SUNK:
            opp_field->grid[row][col] = FIELD_SQUARE_HIT;
            opp_field->smallBoatLives = 0;
            break;
        case RESULT_MEDIUM_BOAT_SUNK:
            opp_field->grid[row][col] = FIELD_SQUARE_HIT;
            opp_field->mediumBoatLives = 0;
            break;
        case RESULT_LARGE_BOAT_SUNK:
            opp_field->grid[row][col] = FIELD_SQUARE_HIT;
            opp_field->largeBoatLives = 0;
            break;
        case RESULT_HUGE_BOAT_SUNK:
            opp_field->grid[row][col] = FIELD_SQUARE_HIT;
            opp_field->hugeBoatLives = 0;
            break;
        case RESULT_MISS:
            opp_field->grid[row][col] = FIELD_SQUARE_EMPTY;
            break;
    }
    return squareStatus;
}
/**
 * This function returns the alive states of all 4 boats as a 4-bit bitfield (stored as a uint8).
 * The boats are ordered from smallest to largest starting at the least-significant bit. So that:
 * 0b00001010 indicates that the small boat and large boat are sunk, while the medium and huge boat
 * are still alive. See the BoatStatus enum for the bit arrangement.
 * @param f The field to grab data from.
 * @return A 4-bit value with each bit corresponding to whether each ship is alive or not.
 */
uint8_t FieldGetBoatStates(const Field *f) {
    uint8_t boatStates = 0;
    if (f->smallBoatLives > 0) {
        boatStates |= FIELD_BOAT_STATUS_SMALL;
    }
    if (f->mediumBoatLives > 0) {
        boatStates |= FIELD_BOAT_STATUS_MEDIUM;
    }
    if (f->largeBoatLives > 0) {
        boatStates |= FIELD_BOAT_STATUS_LARGE;
    }
    if (f->hugeBoatLives > 0) {
        boatStates |= FIELD_BOAT_STATUS_HUGE;
    }
    return boatStates;
}

uint8_t FieldAIPlaceAllBoats(Field *own_field) {
    uint8_t row;
    uint8_t col;
    uint8_t direction;
    int placedCount = 0;
    int smallPlaced = FALSE;
    int mediumPlaced = FALSE;
    int largePlaced = FALSE;
    int hugePlaced = FALSE;
    while (placedCount < 4) {
        direction = rand() % 2;
        row = rand() % FIELD_ROWS;
        col = rand() % FIELD_COLS;
        if (hugePlaced == FALSE) {
            
            if (FieldAddBoat(own_field, row, col, direction, FIELD_BOAT_TYPE_HUGE) == SUCCESS) {
                hugePlaced = TRUE;
                placedCount++;
            }        
        }
        else if (largePlaced == FALSE && hugePlaced == TRUE) {
            
            if (FieldAddBoat(own_field, row, col, direction, FIELD_BOAT_TYPE_LARGE) == SUCCESS) {
                largePlaced = TRUE;
                placedCount++;
            }        
        }
        else if (mediumPlaced == FALSE && largePlaced == TRUE && hugePlaced == TRUE) {
            
            if (FieldAddBoat(own_field, row, col, direction, FIELD_BOAT_TYPE_MEDIUM) == SUCCESS) {
                mediumPlaced = TRUE;
                placedCount++;
            }        
        }
        else if (smallPlaced == FALSE && mediumPlaced == TRUE && largePlaced == TRUE && hugePlaced == TRUE) {
            
            if (FieldAddBoat(own_field, row, col, direction, FIELD_BOAT_TYPE_SMALL) == SUCCESS) {
                smallPlaced = TRUE;
                placedCount++;
            }        
        }
        
    }
    if (placedCount == 4) {
        return SUCCESS;
    }
    else {
        return STANDARD_ERROR;
    }
        
}
/**
 * Given a field, decide the next guess.
 *
 * This function should not attempt to shoot a square which has already been guessed.
 *
 * You may wish to give this function static variables.  If so, that data should be
 * reset when FieldInit() is called.
 * 
 * @param f an opponent's field.
 * @return a GuessData struct whose row and col parameters are the coordinates of the guess.  The 
 *           result parameter is irrelevant.
 */
GuessData FieldAIDecideGuess(const Field *opp_field) {
    int row = 0;
    int col = 0;
    int guessFound = FALSE;
    while(guessFound == FALSE) {
        row = rand() % FIELD_ROWS;
        col = rand() % FIELD_COLS;
        if (opp_field->grid[row][col] == FIELD_SQUARE_UNKNOWN) {
            GuessData own_guess;
            own_guess->col = col;
            own_guess->row = row;
            guessFound = TRUE;
            return own_guess;
        }
    }
   
}
