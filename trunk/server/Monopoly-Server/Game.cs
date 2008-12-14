using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public class Game {

        private ActivePlayer _activePlayer;
        private Dictionary<string, Player> _players;
        private Queue<Player> _playerQueue;
        private Card _cards;

        private int _totalPlayers = 0;
        private bool _started = false;
        private const int MINIMUM_PLAYERS = 2;
        private const int MAXIMUM_PLAYERS = 8;

        public Game() {
            _players = new Dictionary<string, Player>();
            _activePlayer = new ActivePlayer();
            _playerQueue = new Queue<Player>();            
        }

        #region Player Validation
        private bool isValidPlayer(string player) {
            return (_players.Count > 0) ? _players.ContainsKey(player) : false;
        }

        private bool isActivePlayer(string playerName) {
            return (_activePlayer.Player != null) ? playerName.Equals(_activePlayer.Player.Name) : false;
        }
        #endregion Player Validation

        #region Game Start

        private void joinGame(string playerName) {
            if (!_started) {
                addPlayer(playerName);

                _totalPlayers++;
                Irc.Output(Irc.getChannel(), ".join " + playerName);

                if (_totalPlayers == MAXIMUM_PLAYERS) {
                    startGame();
                }

            }
        }

        //If player hasn't already joined, add them to the player list
        private void addPlayer(string player) {
            bool exists = isValidPlayer(player);
            if (!exists) {
                _players.Add(player, new Player(player));
            }
        }

        //Randomly pick who will be the first player.
        private void startGame() {

            Irc.Output(Irc.getChannel(), ".tell * New game starting.");

            Player tempPlayer;
            Player[] allPlayers = _players.Values.ToArray();
            Random random = new Random();
            _cards = new Card(_playerQueue);
            GetOutOfJailFreeCard.ChanceOwner = null;
            GetOutOfJailFreeCard.CommunityChestOwner = null;

            _started = true;
            int randValue = random.Next(0, _totalPlayers);

            String playerSequence = ".players";

            for (int i = 0; i < _totalPlayers; i++) {
                tempPlayer = allPlayers[(i + randValue) % _totalPlayers];
                playerSequence += " " + tempPlayer.Name;
                _playerQueue.Enqueue(tempPlayer);
            }

            Irc.Output(Irc.getChannel(), playerSequence);
            
            setNextPlayer();
        }
        #endregion Game Start

        #region Turn Handling

        private void setNextPlayer() {
            Player player;
            do {
                player = _playerQueue.Dequeue();
            } while (player.Bankrupt && (_playerQueue.Count != 0));

            _playerQueue.Enqueue(player);
            _activePlayer.set(player);

            Irc.Output(Irc.getChannel(), ".turn " + _activePlayer.Player.Name);
        }

        private void endTurn() {
            Player player = _activePlayer.Player;

            if (player.Bankrupt) {
                killPlayer(player);
            } else if (UnhandledDebtList.hasPlayer(player)) {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " you have outstanding debt.");
            } else if (!_activePlayer.CanRoll) {
                if (UnhandledDebtList.Count == 0) {
                    setNextPlayer();
                } else {
                    _activePlayer.EndedTurn = true;
                    Irc.Output(Irc.getChannel(), ".endedTurn " + player.Name);

                    string waitingOn = "";
                    foreach (Player p in UnhandledDebtList.Players) {
                        waitingOn += " " + p.Name;
                    }
                    Irc.Output(Irc.getChannel(), ".waiting " + waitingOn + " to handle their debt.");
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " you still need to roll.");
            }

        }
        #endregion Turn Handling

        #region Game End

        private void endGame() {
            Irc.Output(Irc.getChannel(), ".win " + _players.ToArray()[0].Value.Name);

            _totalPlayers = 0;
            _activePlayer = new ActivePlayer();
            _players.Clear();
            _playerQueue.Clear();
            Property.resetAll();
            UnhandledDebtList.clearAllDebt();
            GetOutOfJailFreeCard.ChanceOwner = null;            

            _started = false;
        }

        private void killPlayer(string playerName) {
            Player player = null;            
            if (isValidPlayer(playerName)) {
                player = _players[playerName];
                _players.Remove(playerName);
                _totalPlayers--;
            }
            
            if (_started) {
                if (_totalPlayers == 1) {
                    endGame();
                } else {
                    
                    UnhandledDebtList.remove(player);

                    string players = ".players";
                    Player tempPlayer;

                    for (int i = 0; i < _playerQueue.Count; ++i) {
                        tempPlayer = _playerQueue.Dequeue();
                        if (!tempPlayer.Bankrupt) {
                            players += " " + tempPlayer.Name;
                            _playerQueue.Enqueue(tempPlayer);
                        } else {
                            --i;
                        }
                    }

                    Irc.Output(Irc.getChannel(), players);

                    if (_activePlayer.Player == player) {
                        if (UnhandledDebtList.Count == 0) {
                            setNextPlayer();
                        } else {
                            _activePlayer.EndedTurn = true;
                        }
                    }
                }
            }
            
        }

        private void killPlayer(Player player) {
            killPlayer(player.Name);
        }

        #endregion Game End

        #region Command Processor
        public void process(string playerName, string command) {

            #region Impossible Events
            if (!command.StartsWith("!")) {
                return;
            }
            #endregion Impossible Events

            bool validPlayer = isValidPlayer(playerName);

            #region New Player Commands
            if (!validPlayer) {
                if (command.Equals("!quit")) {
                    return;
                }
                
                if (_started) {
                    Irc.Output(Irc.getChannel(), ".gameInProgress " + playerName);
                } else {
                    if (command.Equals("!join")) {
                        joinGame(playerName);
                    } else if (command.Equals("!enter")) {
                        Irc.Output(Irc.getChannel(), ".numPlayers " + playerName + " " + _totalPlayers + " of " + MINIMUM_PLAYERS + " to " + MAXIMUM_PLAYERS);
                    } else {
                        Irc.Output(Irc.getChannel(), ".fail " + playerName + " you need to join a game!");
                    }
                }
                return;
            }
            #endregion New Player Commands

            Player player = _players[playerName];

            #region Anytime Commands
            if (command.Equals("!quit")) {
                player.declareBankruptcy(null);
                Irc.Output(Irc.getChannel(), ".part " + playerName);
                killPlayer(player);
                return;
            } else if (command.StartsWith("!nick")) {
                //extract new nickname
                string nick = command.Substring(6);

                //remove player from dictionary
                _players.Remove(player.Name);

                //update play info and re-add to dictionary
                player.Name = nick;
                _players.Add(nick, player);

                Irc.Output(Irc.getChannel(), ".nick " + playerName + " " + nick);
                return;
            } else if (command.StartsWith("!inventory")) {
                viewInventory(player, command);
                return;
            } else
            #endregion Anytime Commands

            #region Game Start
            if (!_started) {
                if (command.Equals("!start")) {
                     if (_players.Count >= MINIMUM_PLAYERS) {
                        startGame();
                     } else {
                        Irc.Output(Irc.getChannel(),".fail " + playerName + " A minimum of " + MINIMUM_PLAYERS + " players are needed to start the game. (Currently have: " + _players.Count + ")");
                     }
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + playerName + " The game hasn't started. Start one by saying !start");
                }
                return;
            }
            #endregion Game Start

            #region Requires Debt Commands
            if (UnhandledDebtList.hasPlayer(player)) {
                if (command.StartsWith("!mortgage")) {
                    mortgage(player, command);
                    return;
                } else if (command.StartsWith("!unmortgage")) {
                    unmortgage(player, command);
                    return;
                } else if (command.StartsWith("!payPropertyDebt")) {
                    payPropertyDebt(player, command);
                    return;
                } else if (command.Equals("!taxFlat")) {
                    payTaxes(player, Amounts.IncomeTaxFlat);
                    return;
                } else if (command.Equals("!taxPercent")) {
                    payTaxes(player, Amounts.IncomeTaxPercent);
                    return;
                } else if (command.StartsWith("!payPlayer")) {
                    payPlayer(player, command);
                    return;
                } else if (command.Equals("!payCashDebt")) {
                    payCashDebt(player);
                    return;
                } else if (command.Equals("!viewDebt")) {
                    UnhandledDebtList.getUnhandledDebt(player).viewDebt();
                    return;
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + playerName + " invalid command.");
                    return;
                }
            }
            #endregion

            #region Validate Player
            //Game started, but active player isn't issuing command.
            if (player != _activePlayer.Player) {
                //TODO: when implementing trading the non-active player might need to do a command
                Irc.Output(Irc.getChannel(), ".fail " + playerName + " You must wait your turn.");
                return;
            }
            #endregion

            if (UnhandledDebtList.hasPlayer(player)) {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " you must first resolve your debt.");
                return;
            } else if (_activePlayer.EndedTurn) {
                Irc.Output(Irc.getChannel(), ".fail " + _activePlayer.Player.Name + " you ended your turn. We're waiting on debt to be resolved.");
                return;
            }

            if (command.Equals("!start")) {
                Irc.Output(Irc.getChannel(), ".fail " + playerName + " The game already started, and you are in it!");
            } else  if (command.Equals("!endTurn")) {
                endTurn();
            } else if (command.Equals("!payJail")) {
                if ( _activePlayer.Player.JailStatus != JailStatus.Outside ) {
                    if ( _activePlayer.Player.hasChanceOutOfJailFreeCard() ) {
                        _activePlayer.Player.releaseChanceOutOfJailCard();
                        _activePlayer.Player.releaseFromJail();
                    } else if ( _activePlayer.Player.hasCommunityChestOutOfJailFreeCard() ) {
                        _activePlayer.Player.releaseCommunityChestOutOfJailCard();
                        _activePlayer.Player.releaseFromJail();
                    } else if ( _activePlayer.CanRoll ) {
                        player.releaseFromJail(Amounts.Jail);
                    }
                }

            } else if (command.Equals("!roll")) {
                if (!_activePlayer.CanRoll) {
                    Irc.Output(Irc.getChannel(), ".fail " + playerName + " you already rolled");
                } else if (_activePlayer.CanRoll) {
                    rollDice();
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + playerName + " can't roll until outstanding debt is resolved.");
                }                    
            } else if (command.Equals("!buy") && _activePlayer.CanBuy) {
                Tile tile = Board.getTile(player.Location);
                TitleDeed titleDeed = Property.getTitleDeed(tile);
                purchase(player, titleDeed);
            } else if (command.StartsWith("!mortgage")) {
                mortgage(player, command);
            } else if (command.StartsWith("!unmortgage")) {
                unmortgage(player, command);
            } else if (command.StartsWith("!upgrade")) {
                upgrade(player, command);
            } else if (command.StartsWith("!downgrade")) {
                downgrade(player, command);
            }
        }

        /*
 *  There are multiple distinct dice rolls to consider.
 *
 *  When both dice show the same value, you rolled "doubles"
 *  - You get an extra turn
 *  - If you roll doubles three times in a row, you go to jail without getting the extra turn.
 *  - If you are in jail, then you get out for free. (And you get an extra turn.)
 * 
 *  When the dice show distinct values
 *  - If you are in jail, and this isn't your third turn in jail, you stay in jail.
 *  - If it was your third turn in jail, you pay $50 and advance by the roll.
 *  - If you aren't in jail, you advance by the roll. 
 */
        private void rollDice() {
            _activePlayer.CanRoll = false;
            _activePlayer.CanBuy = true;

            int dice1 = Die.Roll();
            int dice2 = Die.Roll();
            string name = _activePlayer.Player.Name;
      
            Irc.Output(Irc.getChannel(), ".roll " + name + " " + dice1 + " " + dice2);
            Player player = _activePlayer.Player;

            if (dice1 != dice2) {
                _activePlayer.Doubles = 0;
                JailStatus jailStatus = player.JailStatus;

                if (jailStatus == JailStatus.Outside) {
                    //Unmatched dice, outside jail
                    movePlayer(name, Move.Relative, dice1 + dice2);
                } else if (jailStatus != JailStatus.TurnThree) {
                    //Unmatched dice, failed to escape jail through doubles (but not yet thrice)
                    player.increaseJailTime();
                } else {
                    player.releaseFromJail(Amounts.Jail);

                    if (player.Bankrupt) {
                        killPlayer(player);
                    } else {
                        if (UnhandledDebtList.hasPlayer(player)) {
                            _activePlayer.freeze(dice1 + dice2);
                        } else {
                            movePlayer(name, Move.Relative, dice1 + dice2);
                        }
                    }
                }

                
            } else {

                _activePlayer.Doubles++;

                if (player.JailStatus != JailStatus.Outside) {
                    //Doubles from within jail
                    player.releaseFromJail();
                }

                if (_activePlayer.Doubles == 3) {
                    //Third consecutive doubles
                    Irc.Output(Irc.getChannel(), ".tell * " + name + " rolled doubles three times in a row");
                    player.sendToJail();
                } else {
                    movePlayer(name, Move.Relative, dice1 + dice2);

                    if (!player.Bankrupt) {
                        _activePlayer.CanRoll = true;
                        Irc.Output(Irc.getChannel(), ".rollagain " + name);
                    }
                }
            }
        }

        private void movePlayer(string player, Move move, int value) {
            if (isValidPlayer(player)) {
                _players[player].moveAndEval(move, value);
            }

            if (_activePlayer.Player.Bankrupt) {
                killPlayer(_activePlayer.Player);
            }
        }

        private void purchase(Player player, TitleDeed titleDeed) {
            if (titleDeed == null) {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " Unpurchaseable property.");
            } else if (titleDeed.Owner != null) {
                if (titleDeed.Owner == player) {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " You already own " + titleDeed.Name + ".");
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " " + titleDeed.Owner.Name + " already owns " + titleDeed.Name + ".");
                }
            } else {
                if (player.Cash >= titleDeed.Cost) {
                    player.payFlatFee(titleDeed.Cost);
                    player.addTitleDeed(titleDeed);
                    titleDeed.Owner = player;
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " insufficient funds");
                }
            }
        }

        #endregion Command Processor

        private int atoi(String s) {
            int result = 0;
            char c;
            for (int i = 0; i < s.Length; ++i) {
                c = s[i];
                if (c >= '0' && c <= '9') {
                    result = (result * 10) + (c - '0');
                } else {
                    return result;
                }
            }

            return result;
        }

        #region Unhandled Debt Methods

        private void resolvedDebt(Player player) {
            UnhandledDebt unhandledDebt = UnhandledDebtList.getUnhandledDebt(player);

            if (unhandledDebt != null && !unhandledDebt.hasDebt()) {
                UnhandledDebtList.remove(player);
            }

            if (UnhandledDebtList.Count == 0) {
                if (player.Bankrupt) {
                    killPlayer(player);
                } else if (_activePlayer.EndedTurn) {
                    setNextPlayer();
                }
            }
        }

        private void payPropertyDebt(Player player, string command) {
            
            if (command.Length > 17) {
                int tileIndex = atoi(command.Substring(17));
                Tile tile = Board.getTile(tileIndex);
                TitleDeed titleDeed = Property.getTitleDeed(tile);
                if ((titleDeed != null) && (titleDeed.Owner == player)) {
                    UnhandledDebt unhandledDebt = UnhandledDebtList.getUnhandledDebt(player);

                    if (unhandledDebt != null) {
                        
                        if (unhandledDebt.hasPropertyDebt(tile)) {
                            unhandledDebt.payPropertyDebt(tile);
                        }

                        resolvedDebt(player);
                    }
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
            }
        }

        private void payTaxes(Player player, Amounts fee) {
            if ((player != _activePlayer.Player) || ((fee != Amounts.IncomeTaxPercent) && (fee != Amounts.IncomeTaxFlat))) {
                return;
            }

            UnhandledDebt unhandledDebt = UnhandledDebtList.getUnhandledDebt(player);

            if ((unhandledDebt == null) || (!unhandledDebt.IncomeTax)) {
                return;
            } else {
                unhandledDebt.payTaxDebt(fee);
            }

            resolvedDebt(player);
        }
        #endregion Unhandled Debt Methods

        #region Mortgage Related Command Handling
        private void mortgage(Player player, string command) {
            if (command.Length > 10) {
                int tileIndex = atoi(command.Substring(10));
                Tile tile = Board.getTile(tileIndex);
                TitleDeed titleDeed = Property.getTitleDeed(tile);
                if ((titleDeed != null) && (titleDeed.Owner == player) && (!titleDeed.Mortgaged)) {
                    titleDeed.mortgage();
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
            }
        }

        private void unmortgage(Player player, string command) {
            if (command.Length > 12) {
                int tileIndex = atoi(command.Substring(12));
                Tile tile = Board.getTile(tileIndex);
                TitleDeed titleDeed = Property.getTitleDeed(tile);
                if ((titleDeed != null) && (titleDeed.Owner == player) && (titleDeed.Mortgaged)) {
                    UnhandledDebt unhandledDebt = UnhandledDebtList.getUnhandledDebt(player);

                    if (unhandledDebt == null) {
                        titleDeed.unmortgage();
                    } else {
                        if (unhandledDebt.hasPropertyDebt(tile)) {
                            titleDeed.unmortgage();
                            
                            if (!titleDeed.Mortgaged) {
                                unhandledDebt.removePropertyDebt(tile);
                            }

                            resolvedDebt(player);
                        } else {
                            Irc.Output(Irc.getChannel(), ".fail " + player.Name + " you need to resolve your debt.");
                        }
                    }           
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
            }
        }
        #endregion Mortgage Related Command Handling

        private void viewInventory(Player requester, string command) {
            if (command.Length > 11) {
                string playerName = command.Substring(11);
                
                if (!isValidPlayer(playerName)) {
                    Irc.Output(Irc.getChannel(), ".fail " + requester.Name);
                    return;
                }

                Player requestee = _players[playerName];
                requestee.tellInventory(requester);
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + requester.Name + " invalid command.");
            }
        }

        private void payPlayer(Player payer, string command) {
            if (command.Length > 11) {
                string playerName = command.Substring(11);

                if (!isValidPlayer(playerName)) {
                    Irc.Output(Irc.getChannel(), ".fail " + payer.Name + " invalid command.");
                    return;
                }

                Player payee = _players[playerName];
                
                UnhandledDebt unhandledDebt = UnhandledDebtList.getUnhandledDebt(payer);
                if (unhandledDebt == null) {
                    return;
                }

                if (!unhandledDebt.hasPlayerDebt(payee)) {
                    Irc.Output(Irc.getChannel(), ".fail " + payer.Name + " you don't have debt to " + payee + ".");
                    return;
                }

                unhandledDebt.payPlayerDebt(payee);
                resolvedDebt(payee);
                
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + payer.Name + " invalid command.");
            }
        }

        private void payCashDebt(Player player) {
            if (player == null) {
                return;
            }

            UnhandledDebt unhandledDebt = UnhandledDebtList.getUnhandledDebt(player);

            if ((unhandledDebt == null) || (!unhandledDebt.hasCashDebt())) {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
                return;
            } if (player.Cash < unhandledDebt.CashDebt) {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " insufficient funds to pay debt.");
                return;
            }

            unhandledDebt.payCashDebt();
            resolvedDebt(player);

            if ((!unhandledDebt.hasCashDebt()) && (player == _activePlayer.Player) && (_activePlayer.Frozen)) {
                int roll = _activePlayer.unfreeze();
                movePlayer(player.Name, Move.Relative, roll);
            }
        }

        private void upgrade(Player player, string command) {
            if (command.Length > 9) {
                int tileIndex = atoi(command.Substring(9));
                Tile tile = Board.getTile(tileIndex);
                TitleDeed titleDeed = Property.getTitleDeed(tile);
                if ((titleDeed != null) && (titleDeed.Owner == player)) {
                    titleDeed.upgrade();
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " you can't upgrade specified tile.");
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
            }
        }

        private void downgrade(Player player, string command) {
            if (command.Length > 11) {
                int tileIndex = atoi(command.Substring(11));
                Tile tile = Board.getTile(tileIndex);
                TitleDeed titleDeed = Property.getTitleDeed(tile);
                if ((titleDeed != null) && (titleDeed.Owner == player)) {
                    titleDeed.downgrade();
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + player.Name + " you can't downgrade specified tile.");
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + player.Name + " invalid command.");
            }
        }

    }
}
