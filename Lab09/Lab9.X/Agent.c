/* 
 * File:   Agent.c
 * Author: Ryan Dong (rfdong@ucsc.edu)
 *
 * Created on December 5, 2023, 11:54 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Agent.h"
#include "Ascii.h"
#include "Message.h"
#include "BattleBoats.h"
#include "BOARD.h"
#include "Buttons.h"
#include "CircularBuffer.h"
#include "FieldOled.h"
#include "Oled.h"
#include "OledDriver.h"
#include "Uart1.h"
#include "Negotiation.h"
#include "Field.h"

struct Agent {
    AgentState state;
    Message msg;
    NegotiationData secret;
    NegotiationData hash;
    Field own_field;
    Field opp_field;
    
};

static struct Agent agent;
static int turnCount = 0;
static char *newGameMsg = "Press BTN4 to start\n";
static char *errorMsg;
static char *cheatMsg = "Cheating detected: sending to end screen.";
static char *defeatMsg = "Defeated! You lost.";
static char *victoryMsg = "Victory! You won";
static FieldOledTurn gameTurn;
#define RANDSIZE 0xFFFFF
#define BOATSSUNK 0b00000000
/*
 * 
 */
void AgentInit(void) {
    agent.state = AGENT_STATE_START;
    gameTurn = FIELD_OLED_TURN_NONE
    turnCount = 0;
    OledClear(OLED_COLOR_BLACK);
    OledDrawString(newGameMsg);
    OledUpdate();
}

Message AgentRun(BB_Event event) {
    switch (event.type) {
        case BB_EVENT_START_BUTTON:
            if (agent.state == AGENT_STATE_START) {
                //generate A, #a
                agent.secret = rand() & RANDSIZE;
                //send CHA
                agent.msg.param0 = agent.secret;
                agent.msg.type = MESSAGE_CHA
                //initialize fields
                FieldInit(&agent.own_field, &agent.opp_field);
                //place own boats
                FieldAIPlaceAllBoats(&agent.own_field, &agent.opp_field);
                agent.state = AGENT_STATE_CHALLENGING;
            }
            break;
        case BB_EVENT_RESET_BUTTON:
            //reset all data
            AgentInit();
            break;
        case BB_EVENT_ACC_RECEIVED:
            if (agent.state == AGENT_STATE_CHALLENGING) {
                //send REV
                //go to heads or tails
                agent.msg.type = MESSAGE_REV;
                agent.msg.param0 = agent.secret;
                NegotiationOutcome coinToss = NegotiateCoinFlip(agent.secret, event.param0);
                if (coinToss == HEADS) {
                    agent.state = AGENT_STATE_WAITING_TO_SEND;
                }
                else if (coinToss == TAILS) {
                    agent.state = AGENT_STATE_DEFENDING;
                }
            }
            break;
        case BB_EVENT_CHA_RECEIVED:
            if (agent.state == AGENT_STATE_START) {
                //generate B
                agent.secret = rand() & RANDSIZE;
                agent.msg.param0 = agent.secret;
                agent.msg.type = MESSAGE_ACC;        
                //send ACC
                
                
                //initialize fields
                FieldInit(&agent.own_field, &agent.opp_field);
                //place own boats
                FieldAIPlaceAllBoats(&agent.own_field);
                agent.state = AGENT_STATE_ACCEPTING;
            }
            break;
        case BB_EVENT_REV_RECEIVED:
            if (agent.state == AGENT_STATE_ACCEPTING) {
                //detect cheating
                NegotiationOutcome coinToss = NegotiateCoinFlip(agent.secret, event.param0);
                if (NegotiationVerify(event.param0, agent.hash) == FALSE) {
                    OledDrawString(cheatMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    break;
                }
                
                if (coinToss == TAILS) {
                    GuessData gData = FieldAIDecideGuess(&agent.opp_field);
                    agent.msg.type = MESSAGE_SHO;
                    agent.msg.param0 = gData.row;
                    agent.msg.param1 = gData.col;
                    agent.state = AGENT_STATE_ATTACKING;
                }
                else if (coinToss == HEADS) {
                    agent.state = AGENT_STATE_DEFENDING;
                }
                //go to heads or tails
            }
            break;
        case BB_EVENT_SHO_RECEIVED:
            if (agent.state == AGENT_STATE_DEFENDING) {
                //update own field
                GuessData gData;
                gData.row = event.param0;
                gData.col = event.param1;
                FieldRegisterEnemyAttack(&agent.own_field, &gData);
                //send RES
                agent.msg.type = MESSAGE_RES;
                agent.msg.param0 = event.param0;
                agent.msg.param1 = event.param1;
                agent.msg.param2 = gData.result;
                uint8_t boatStates = FieldGetBoatStates(&agent.own_field);       
                //defeat
                if (boatStates == BOATSSUNK) {
                    agent.msg.type = MESSAGE_NONE;
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(defeatMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                }
                else {
                    agent.state = AGENT_STATE_WAITING_TO_SEND;
                }
                            
            }
            break;
        case BB_EVENT_MESSAGE_SENT:
            if (agent.state == AGENT_STATE_WAITING_TO_SEND) {
                turnCount++;
                //decide guess
                GuessData gData = FieldAIDecideGuess(&agent.opp_field);
                //send SHO
                agent.msg.type = MESSAGE_SHO;
                agent.msg.param0 = gData.row;
                agent.msg.param1 = gData.col;
                agent.state = AGENT_STATE_ATTACKING;
            }
            break;
        case BB_EVENT_RES_RECEIVED:
            if (agent.state == AGENT_STATE_ATTACKING) {
                //update record of enemy field
                
                GuessData gData;
                gData.row = event.param0;
                gData.col = event.param1;
                gData.result = event.param2;
                //check for victory
                FieldUpdateKnowledge(&agent.opp_field, &gData);
                uint8_t boatStates = FieldGetBoatStates(&agent.opp_field);
                if (boatStates == BOATSSUNK) {
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(victoryMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                }
                else {
                    
                    agent.state = AGENT_STATE_DEFENDING;
                }
            }
            break;
        case BB_EVENT_ERROR:
            //display appropriate message to user
            switch (event.param0) {
                case BB_ERROR_BAD_CHECKSUM:
                    errorMsg = "ERROR: BAD CHECKSUM";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    break;
                    //msg
                case BB_ERROR_PAYLOAD_LEN_EXCEEDED:
                    errorMsg = "ERROR: PAYLOD LENGTH EXCEEDED";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    break;
                    //msg
                case BB_ERROR_CHECKSUM_LEN_EXCEEDED: 
                    errorMsg = "ERROR: CHECKSUM LENGTH EXCEEDED";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    break;
                case BB_ERROR_CHECKSUM_LEN_INSUFFICIENT:
                    errorMsg = "ERROR: CHECKSUM LENGTH INSUFFICIENT";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    break;
                case BB_ERROR_INVALID_MESSAGE_TYPE:
                    errorMsg = "ERROR: INVALID MESSAGE TYPE";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    break;
                case BB_ERROR_MESSAGE_PARSE_FAILURE:
                    errorMsg = "ERROR: MESSAGE PARSE FAILURE";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMsg);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    break;
            }
            
            OledClear(OLED_COLOR_BLACK);
            FieldOledDrawScreen(&agent.own_field, &agent.opp_field, gameTurn, turnCount);
            OledUpdate();
            return agent.msg;
    }
}

AgentState AgentGetState(void) {
    return agent.state;

}
void AgentSetState(AgentState newState) {
    agent.state = newState;
}