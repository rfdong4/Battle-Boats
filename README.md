# BattleBoats

A networked battleship game implementation for the PIC32 microcontroller.

## Overview

BattleBoats is a two-player battleship game where players place ships on a 6x10 grid and take turns guessing coordinates to sink their opponent's fleet. The game features:

- **4 different ship types**: Small (3 squares), Medium (4 squares), Large (5 squares), and Huge (6 squares)
- **Networked gameplay**: Players communicate via UART serial connection
- **Fair turn determination**: Uses cryptographic negotiation to determine who goes first
- **AI opponents**: Includes both AI and human player modes
- **OLED display**: Visual game interface on the PIC32's OLED screen

## Game Rules

1. **Setup Phase**: Each player places 4 ships of different sizes on their 6x10 grid
2. **Negotiation Phase**: Players use cryptographic negotiation to fairly determine who goes first
3. **Battle Phase**: Players take turns guessing coordinates on their opponent's grid
4. **Victory Condition**: First player to sink all opponent's ships wins

### Ship Types
- **Small Boat**: 3 squares
- **Medium Boat**: 4 squares  
- **Large Boat**: 5 squares
- **Huge Boat**: 6 squares

## Project Structure

```
battleboats/
├── Agent.c/h              # Main game agent and state machine
├── Field.c/h              # Game board logic and ship placement
├── Message.c/h            # Network message encoding/decoding
├── Negotiation.c/h        # Cryptographic turn negotiation
├── Lab09_main.c           # Main application entry point
├── Makefile               # Build configuration
├── agent.py               # Python testing tool for human vs AI games
└── [various support libraries]
```

## Building and Running

### Prerequisites
- MPLAB X IDE with XC32 compiler
- PIC32 microcontroller board
- OLED display module
- UART serial connection capability

### Build Instructions

1. Open the project in MPLAB X IDE
2. Ensure the correct PIC32 target is selected
3. Build the project using the provided Makefile:
   ```bash
   cd battleboats
   make clean
   make build
   ```

### Running the Game

1. **Flash the compiled binary** to your PIC32 board
2. **Connect via UART** (typically 115200 baud rate)
3. **Press BTN4** to start a new game
4. **Use the Python agent tool** for human vs AI gameplay:
   ```bash
   python agent.py
   ```

## Game Modes

### AI vs AI
- Both players use the built-in AI for automatic gameplay
- Useful for testing and demonstration

### Human vs AI
- Use the provided `agent.py` tool to play against the PIC32 AI
- Connect via serial port and use the GUI interface

### Human vs Human
- Two PIC32 boards can play against each other
- Connect via UART crossover cable

## Testing Tools

### Python Agent Tool (`agent.py`)
A comprehensive testing and gameplay interface that provides:
- Serial communication with PIC32
- Message analysis and debugging
- Negotiation tools for turn determination
- Manual message sending capabilities
- Real-time game state monitoring

### Test Files
- `FieldTest.c`: Unit tests for field operations
- `AgentTest.c`: Agent state machine tests
- `MessageTest.c`: Message encoding/decoding tests
- `NegotiationTest.c`: Cryptographic negotiation tests

## Technical Details

### Communication Protocol
The game uses a custom message protocol over UART:
- Messages are checksum-protected
- Format: `$PAYLOAD*CHECKSUM`
- Message types: CHA (Challenge), ACC (Accept), REV (Reveal), SHO (Shot), RES (Result)

### State Machine
The agent implements a 7-state finite state machine:
1. **START**: Initial state, waiting for game start
2. **CHALLENGING**: Initiating turn negotiation
3. **ACCEPTING**: Responding to turn negotiation
4. **ATTACKING**: Taking shots at opponent
5. **DEFENDING**: Receiving shots from opponent
6. **WAITING_TO_SEND**: Waiting for message transmission
7. **END_SCREEN**: Game over display

### Cryptographic Negotiation
Uses a fair coin flip protocol:
1. Challenger generates secret A and sends hash(A)
2. Accepter generates secret B and sends B
3. Challenger reveals A
4. Both compute parity(A ⊕ B) to determine turn order

## Development Notes

- **Author**: Ryan Dong (rfdong@ucsc.edu)
- **Language**: C (PIC32) and Python (testing tools)
- **Hardware**: PIC32 microcontroller with OLED display

## Troubleshooting

### Common Issues
1. **Serial Connection**: Ensure correct COM port and baud rate (115200)
2. **Build Errors**: Check XC32 compiler installation and PIC32 target selection
3. **Game Hangs**: Verify UART connections and message protocol compliance

### Debug Features
- Enable `TRACE_MODE` in `Lab09_main.c` for event logging
- Use `UNSEEDED_MODE` for repeatable testing scenarios
- Monitor UART output for debugging information


