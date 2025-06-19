/* 
 * File:   FieldTest.c
 * Author: Ryan Dong (rfong@ucsc.edu)
 * Created on December 5, 2023, 11:53 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "Field.h"
#include "Uart1.h"
#include "BOARD.h"
#include "FieldOled.h"
static Field testOwnField;
static Field testOppField;
static GuessData gData;
static uint8_t testCol = 4;
static uint8_t testRow = 5;
/*
 * 
 */
int main() {
    BOARD_Init();
    int resCount = 0;
    printf("Welcome to rfdong's Field.c Test!\n");
    
    printf("Now testing FieldInit()\n");
    
    FieldInit(&testOwnField, &testOppField);
    
    FieldPrint_UART(&testOwnField, &testOppField);
    
    printf("Expected output: 6x10 grid with empty squares, 6x10 grid with unknown squares\n");
    printf("Now testing FieldGetSquareStatus()\n");
    
    if(FieldGetSquareStatus(&testOwnField, 1, 2) == FIELD_SQUARE_EMPTY) {
        resCount++;
    }
    if(FieldGetSquareStatus(&testOppField, 2, 5) == FIELD_SQUARE_UNKNOWN) {
        resCount++;
    }
    if(FieldGetSquareStatus(&testOppField, -1, 5) == FIELD_SQUARE_INVALID) {
        resCount++;
    }
    
    if(resCount == 3) {
        printf("PASSED: 3/3 TESTS PASSED\n");
    } else {
        printf("FAILED: %d/3 TESTS PASSED\n", resCount);
    }
    resCount = 0;
    printf("Now testing FieldSetSquareStatus()\n");
    FieldSetSquareStatus(&testOwnField, 1, 2, FIELD_SQUARE_LARGE_BOAT);
    FieldSetSquareStatus(&testOwnField, 0, 0, FIELD_SQUARE_SMALL_BOAT);
    
    if(FieldGetSquareStatus(&testOwnField, 1, 2) == FIELD_SQUARE_LARGE_BOAT) {
        resCount++;
    }
    if(FieldGetSquareStatus(&testOwnField, 0, 0) == FIELD_SQUARE_SMALL_BOAT) {
        resCount++;
    }
    if(resCount == 2) {
        printf("PASSED: 2/2 TESTS PASSED\n");
    } else {
        printf("FAILED: %d/2 TESTS PASSED\n");
    }
    resCount = 0;
    FieldInit(&testOwnField, &testOppField);
    printf("Now testing FieldAddBoat()\n");
    printf("Expected output: 6x10 grid with small boat at (0,0), large boat at (1, 2), and huge boat at (0, 7)\n");
    FieldAddBoat(&testOwnField, 0, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_SMALL);
    FieldAddBoat(&testOwnField, 1, 2, FIELD_DIR_EAST, FIELD_BOAT_TYPE_LARGE);
    FieldAddBoat(&testOwnField, 0, 7, FIELD_DIR_SOUTH, FIELD_BOAT_TYPE_HUGE);
    FieldPrint_UART(&testOwnField, &testOppField);
    if(FieldGetSquareStatus(&testOwnField, 0, 0) == FIELD_SQUARE_SMALL_BOAT) {
        resCount++;
    }
    if(FieldGetSquareStatus(&testOwnField, 1, 2) == FIELD_SQUARE_LARGE_BOAT) {
        resCount++;
    }
    if(FieldGetSquareStatus(&testOwnField, 0, 7) == FIELD_SQUARE_HUGE_BOAT) {
        resCount++;
    }
    if(FieldAddBoat(&testOwnField, 0, -7, FIELD_DIR_SOUTH, FIELD_BOAT_TYPE_HUGE) == STANDARD_ERROR) {
        resCount++;
    }
    if (resCount == 4) {
        printf("PASSED: 4/4 TESTS PASSED\n");
    } else {
        printf("FAILED: %d/4 TESTS PASSED\n");
    }
    resCount = 0;
    printf("Now Testing FieldRegisterEnemyAttack()\n");
    gData.col = 0
    gData.row = 0;        
    FieldRegisterEnemyAttack(&testOwnField, &gData);
    

    BOARD_End();
    while(1);
} 

