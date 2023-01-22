#include <ncurses.h>
#include <stdio.h>  // STDOUT_FILENO
#include <time.h>
#include <unistd.h>  // write
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <signal.h>
#include <cstdlib>

const int MIN_BET = 2;
const int MAX_BET = 10000;
const int INITIAL_BANK = 500;

const std::string labels[13] = {"A", "2", "3",  "4", "5", "6", "7",
                                "8", "9", "10", "J", "Q", "K"};
const int values[13] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10};
const int INITIAL_DECK[52] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

class Game {
 public:
  Game(int nDecks) {
    decks = nDecks;
    mode = M_BANK;
    bank = INITIAL_BANK;
    bet = MIN_BET;
    gameQuit = false;
    reshuffle();
  }
  bool gameQuit;

  void reshuffle() {
    int nDecks = decks;
    deck.clear();
    while (nDecks-- > 0)
      deck.insert(deck.end(), INITIAL_DECK, INITIAL_DECK + 52);
  }

  void dealPlayer() { dealHand(playerHand); }
  void dealDealer() { dealHand(dealerHand); }

  bool isPlayerBust() {
    std::vector<int> playerValues = getHandValues(playerHand);
    return playerValues.size() == 0;
  }
  bool isDealerBust() {
    std::vector<int> dealerValues = getHandValues(dealerHand);
    return dealerValues.size() == 0;
  }
  bool isDealerWin() {
    std::vector<int> playerValues = getHandValues(playerHand);
    std::vector<int> dealerValues = getHandValues(dealerHand);
    if (playerValues.size() == 0)
      return true;
    if (dealerValues.size() == 0)
      return false;
    int maxPlayerScore = playerValues.at(playerValues.size() - 1);
    int maxDealerScore = dealerValues.at(dealerValues.size() - 1);
    return maxDealerScore > 16 && maxDealerScore > maxPlayerScore;
  }
  bool isTie() {
    std::vector<int> playerValues = getHandValues(playerHand);
    std::vector<int> dealerValues = getHandValues(dealerHand);
    if (playerValues.size() == 0 && dealerValues.size() == 0)
      return true;
    if (playerValues.size() == 0)
      return false;
    int maxPlayerScore = playerValues.at(playerValues.size() - 1);
    int maxDealerScore = dealerValues.at(dealerValues.size() - 1);
    return maxDealerScore > 16 && maxDealerScore == maxPlayerScore;
  }
  bool isPlayerWin() {
    std::vector<int> playerValues = getHandValues(playerHand);
    if (playerValues.size() == 0)
      return false;
    std::vector<int> dealerValues = getHandValues(dealerHand);
    if (dealerValues.size() == 0) {
      return true;
    }
    int maxPlayerScore = playerValues[playerValues.size() - 1];
    int maxDealerScore = dealerValues[dealerValues.size() - 1];
    return maxDealerScore > 16 && maxPlayerScore > maxDealerScore;
  }
  bool isPlayer21() {
    std::vector<int> playerHandValues = getHandValues(playerHand);
    if (playerHandValues.size() == 0)
      return false;
    auto q =
        std::search_n(playerHandValues.begin(), playerHandValues.end(), 1, 21);
    return q != playerHandValues.end();
  }

  std::string labelDeck() {
    std::string result = "Deck: ";
    for (auto& i : deck) {
      int subIndex = i % 13;
      result.append(labels[subIndex]);
      result.append(" ");
    }
    std::cout << std::endl;
    return result;
  }
  std::string labelBet() {
    std::string result = "Bet: ";
    char buf[10]{0};
    sprintf(buf, "%d", bet);
    result.append(buf);
    return result;
  }
  std::string labelBank() {
    std::string result = "Bank: ";
    char buf[10]{0};
    sprintf(buf, "%d", bank);
    result.append(buf);
    return result;
  }
  std::string labelDealerHidden() {
    std::string result = "Dealer: ";
    int firstCardSubIndex = dealerHand[0] % 13;
    result.append(labels[firstCardSubIndex]);
    result.append(" * ");
    int firstCardValue = values[firstCardSubIndex];
    char buf[3]{0};
    sprintf(buf, "(%d", firstCardValue);
    result.append(buf);
    if (firstCardValue == 1) {
      result.append("/10)");
    } else {
      result.append(")");
    }
    return result;
  }
  std::string labelDealer() {
    std::string result = "Dealer: ";
    return result.append(labelHand(dealerHand));
  }
  std::string labelPlayer() {
    std::string result = "Player: ";
    return result.append(labelHand(playerHand));
  }
  std::string labelGame() {
    if (isPlayerBust())
      return "BUST!";
    if (isPlayerWin()) {
      return "You Win!";
    }
    if (isDealerWin()) {
      return "Dealer Wins!";
    }
    if (isTie()) {
      return "Tie!";
    }
    return "";
  }
  std::string labelBankScreen() {
    std::string result = "";
    result.append(labelBet());
    result.append("\n");
    result.append(labelBank());
    result.append("\n\n");
    result.append("Deal(j) / Increase Bet(up) / Decrease Bet(down) / Quit(q)");
    result.append("\n");
    return result;
  }
  std::string labelPlayScreen() {
    std::string result = "";
    result.append(labelBet());
    result.append("\n");
    result.append(labelBank());
    result.append("\n");
    result.append(labelDealerHidden());
    result.append("\n");
    result.append(labelPlayer());
    result.append("\n\n");
    result.append("Hit(j) / Stand(k)\n");
    return result;
  }
  std::string labelGameOverScreen() {
    std::string result = "";
    result.append(labelBet());
    result.append("\n");
    result.append(labelBank());
    result.append("\n");
    result.append(labelDealer());
    result.append("\n");
    result.append(labelPlayer());
    result.append("\n");
    result.append(labelGame());
    result.append("\n");
    result.append("Deal(j) / Bank(k)\n");
    return result;
  }

  enum Key { KK_UP, KK_DOWN, KK_J, KK_K, KK_Q, KK_COUNT };
  Key getKeyPress() {
    /*
  up 27 91 65
  down 27 91 66
  j 106
  k 107
  q 113
  */
    int buf[10]{0};
    while (true) {
      int c = -1;
      int bufSize = 0;
      while ((c = getch()) != -1) {
        buf[bufSize++] = c;
      }
      switch (buf[0]) {
        case 106:
          return KK_J;
        case 107:
          return KK_K;
        case 113:
          return KK_Q;
        case 27:  // escape sequence
          if (buf[1] == 91) {
            switch (buf[2]) {
              case 65:
                return KK_UP;
              case 66:
                return KK_DOWN;
            }
          }
      }
    }
    return KK_COUNT;
  }
  void playBank() {
    printw(labelBankScreen().c_str());
    Key k = getKeyPress();
    switch (k) {
      case KK_UP:
        bet += 1;
        if (bet > MAX_BET)
          bet = MAX_BET;
        if (bet > bank)
          bet = bank;
        return;
      case KK_DOWN:
        bet -= 1;
        if (bet < MIN_BET)
          bet = MIN_BET;
        return;
      case KK_J:
        mode = M_GAME;
        return;
      case KK_Q:
        gameQuit = true;
        return;
      default:
        return;
    }
  }
  void playGame() {
    if (dealerHand.size() == 0) {
      // new hand
      dealDealer();
      dealDealer();
      dealPlayer();
      dealPlayer();
    }
    printw(labelPlayScreen().c_str());
    Key k = getKeyPress();
    std::vector<int> dealerHandValues;
    int maxDealerValue = 0;
    switch (k) {
      case KK_J:
        dealPlayer();
        if (!isPlayerBust() && !isPlayer21()) {
          // continue playing
          return;
        }
        [[fallthrough]];
      case KK_K:
        // play out dealer
        dealerHandValues = getHandValues(dealerHand);
        maxDealerValue = dealerHandValues[dealerHandValues.size() - 1];
        while (dealerHandValues.size() > 0 && maxDealerValue <= 16) {
          dealDealer();
          dealerHandValues = getHandValues(dealerHand);
          if (dealerHandValues.size() == 0) {
            break;
          }
          maxDealerValue = dealerHandValues[dealerHandValues.size() - 1];
        }
        mode = M_END;
        return;
      default:
        return;
    }
  }
  void playEnd() {
    printw(labelGameOverScreen().c_str());
    Key k = getKeyPress();
    if (k == KK_J || k == KK_K) {
      if (isPlayerWin()) {
        bank += bet;
      } else if (!isTie()) {
        bank -= bet;
      }
      playerHand.clear();
      dealerHand.clear();
      reshuffle();
    }
    switch (k) {
      case KK_J:
        mode = M_GAME;
        return;
      case KK_K:
        mode = M_BANK;
        return;
      default:
        return;
    }
  }

  void gameLoop() {
    erase();
    move(0, 0);
    switch (mode) {
      case M_BANK:
        playBank();
        break;
      case M_GAME:
        playGame();
        break;
      case M_END:
        playEnd();
        break;
    }
  }

 private:
  enum MODES { M_BANK, M_GAME, M_END };
  MODES mode;
  int decks;
  int bet;
  int bank;
  std::vector<int> deck;
  std::vector<int> dealerHand;
  std::vector<int> playerHand;
  void dealHand(std::vector<int>& hand) {
    int randIndex = (1.0 * std::rand() / RAND_MAX) * deck.size();
    int card = deck[randIndex];
    deck.erase(deck.begin() + randIndex);
    hand.push_back(card);
  }
  std::vector<int> getHandValues(std::vector<int> hand) {
    // std::unordered_set<int> handValues;
    std::vector<int> handValues;
    handValues.push_back(0);
    // assemble all possible hand values
    for (int i = 0; i < (int)hand.size(); i++) {
      // for each card in the hand
      int subIndex = hand[i] % 13;
      int value = values[subIndex];
      // add the value to each of the existing handValues, size of
      // handValues double when we encounter an ace
      int size = (int)handValues.size();
      for (int j = 0; j < size; j++) {
        if (value == 1) {
          handValues.push_back(handValues[j] + 10);
        }
        handValues[j] += value;
      }
    }
    // find duplicates and out of bound values
    std::vector<int> remove;
    for (int i = 0; i < (int)handValues.size(); i++) {
      if (i > 0 && handValues[i] == handValues[i - 1]) {
        remove.push_back(i);
      } else if (handValues[i] > 21) {
        remove.push_back(i);
      }
    }
    // remove duplicates and out of bound values
    for (int i = 0; i < (int)remove.size(); i++) {
      int removeIndex = remove[i] - i;
      handValues.erase(handValues.begin() + removeIndex);
    }
    return handValues;
  }
  std::string labelHand(std::vector<int> hand) {
    std::string result = "";
    for (int i = 0; i < (int)hand.size(); i++) {
      int subIndex = hand[i] % 13;
      result.append(labels[subIndex]);
      result.append(" ");
    }
    result.append("(");
    std::vector<int> handValues = getHandValues(hand);
    if (handValues.size() == 0) {
      result.append("BUST)");
      return result;
    }
    for (int i = 0; i < (int)handValues.size(); i++) {
      char buf[3]{0};
      sprintf(buf, "%d", handValues[i]);
      result.append(buf);
      if (i < (int)handValues.size() - 1) {
        result.append("/");
      }
    }
    result.append(")");
    return result;
  }
};

void signal_callback_handler(int signum) {
  endwin();
  exit(signum);
}

int main() {
  srand(clock());
  signal(SIGINT, signal_callback_handler);
  // setup ncurses
  WINDOW* w = initscr();
  cbreak();  // get input as chars are typed
  noecho();
  nonl();               // no newline translation
  intrflush(w, false);  // help ncurses with interrupts
  // halfdelay(1); //TODO slow in windows, shouldn't be slower
  nodelay(w, true);
  Game g(1);
  while (!g.gameQuit) {
    g.gameLoop();
  }

  // destroy ncurses
  endwin();
}
