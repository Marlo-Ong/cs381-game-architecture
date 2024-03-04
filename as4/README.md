# AS4: Custom Ad Hoc Game: BLACKJACK!

Authored by Marlo Ongkingco for CS381 (Game Engine Architecture)

## Instructions

In order to run this assignment:

1. In a terminal, move into a directory that you want to copy this assignment in.
2. Run the following command to clone this repository:

   > git clone https://github.com/Marlo-Ong/cs381-game-architecture.git
   >
3. Move into the as4 and fetch the raylib submodule:

   > cd cs381-game-architecture/as4

   > git submodule update --init --recursive

4. Create a build folder, move into it, and compile:

   > mkdir build

   > cd build

   > cmake ..

   > make

5. If it compiled successfully (this may take a bit), an executable should be created, which you can run with the following command:

   > ./as4

6. Once the executable is run, a window should open to the title screen.

## Controls

- Use directional arrow keys (UP, DOWN, LEFT, RIGHT) to move
   - In the title screen, use UP and DOWN to change your wager amount.
- Pressing SPACEBAR performs an action:
   - When over the deck, draws (picks up) a card.
   - When holding a card, places the card at the player's spot.
      - Place the card over its corresponding colored slot to play it.
      - Or, place the card over the "sell card" spot to get some money.
   - When over an ACE card, toggles its value between 1 and 11 (yellow ACE means 11).

## Gameplay

Opening the window starts you at the title screen, where you can change your initial wager amount for the game.
Pressing SPACE starts the game. Just like normal blackjack, the sum of your cards' values must to get as close to 21 as possible without going over (busting). However, there are some twists:
   - You must draw and place down your own cards from your deck.
   - A timer ticks down from 10 seconds. Each match ends when the timer hits zero, and your placed cards are evaluated.
   - Payouts are given automatically and games then restart immediately every 10 seconds.
   - You can also sell cards you don't want to use to regain some money.
   - You win if you get a higher value than the dealer without busting.

Congrats, you can now gamble your savings away! Here are your payouts:
   - Normal Win: + x0.5 of the wager
   - Blackjack Win: + x1.5 of the wager
   - Loss: - x1 of the wager
   - Draw: + x0 of the wager

TIP: Don't piss off the dealer or things might get physical!

## Credits

- Background Music: "Luigi's Casino" by Koji Kondo
- Sound Effects: Various by Pixabay
- Card and Icon Assets: Board by Game-icons.net (https://game-icons.net/tags/board.html)
