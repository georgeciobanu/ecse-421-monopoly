using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public class Player {
        private string _name;
        
        private int _cash = 1500;
        private int _location = 0;
        private JailStatus _jailStatus = JailStatus.Outside;
        private bool _isBankrupt = false;
        private Dictionary<Tile, TitleDeed> _inventory = new Dictionary<Tile, TitleDeed>();

        #region Properties
        public int Cash {
            get {
                return _cash;
            }
        }
        public int Location {
            get {
                return _location;
            }
        }
        public string Name {
            get {
                return _name;
            }
            set {
                _name = value;
            }
        }
        public JailStatus JailStatus {
            get {
                return _jailStatus;
            }
        }

        public bool Bankrupt {
            get {
                return _isBankrupt;
            }
        }

        public int Hotels {
            get {
                int numHotels = 0;
                foreach ( TitleDeed titleDeed in _inventory.Values ) {
                    if ( titleDeed.Level == Level.Hotel ) {
                        ++numHotels;
                    }
                }
                return numHotels;
            }
        }

        public int Houses {
            get {
                int numHouses = 0;
                foreach (TitleDeed titleDeed in _inventory.Values) {
                    if (titleDeed.Level != Level.Hotel) {
                        numHouses += (int)titleDeed.Level;
                    }
                }
                return numHouses;
            }
        }
        #endregion Properties

        public Player(String name) {
            _name = name;
        }

        #region Networth
        public int networthTax() {
            int networth = _cash;

            foreach (TitleDeed titleDeed in _inventory.Values) {
                networth += titleDeed.TaxValue;
            }

            return networth;
        }

        public int networthCash() {
            int networth = _cash;
            int availableHouses = Property.Houses;

            if (availableHouses >= 12) {
                foreach (TitleDeed titleDeed in _inventory.Values) {
                    networth += titleDeed.CashValue;
                }
                return networth;
            }

            LinkedList<Land>[] propertyLevel = new LinkedList<Land>[4];

            for (int i = 0; i < 4; ++i) {
                propertyLevel[i] = new LinkedList<Land>();
            }

            int numHotels;

            foreach (TitleDeed titleDeed in _inventory.Values) {
                if (titleDeed.GetType() == typeof(Land)) {
                    numHotels = ((Land)titleDeed).hotelsInSet();

                    propertyLevel[numHotels].AddLast((Land)titleDeed);

                } else {
                    networth += titleDeed.CashValue;
                }
            }
            //properties with no hotels count
            foreach (Land titleDeed in propertyLevel[0]) {
                networth += titleDeed.CashValue;
            }
            //have enough houses so that properties with at most 1 hotel count
            if (availableHouses >= 4) {
                foreach (Land titleDeed in propertyLevel[1]) {
                    availableHouses += (int)titleDeed.Level;
                    networth += titleDeed.CashValue;
                }
            }

            //have enough houses so that properties with at most 2 hotel count
            if (availableHouses >= 8) {
                foreach (Land titleDeed in propertyLevel[2]) {
                    availableHouses += (int)titleDeed.Level;
                    networth += titleDeed.CashValue;
                }
            }

            //have enough houses so that properties with at most 3 hotel count
            if (availableHouses >= 12) {
                foreach (Land titleDeed in propertyLevel[3]) {
                    availableHouses += (int)titleDeed.Level;
                    networth += titleDeed.CashValue;
                }
            //have enough houses so that we can turn 2 hotels into sets of four houses, but not three hotels. (So switch 2 most expensive hotels from sets of 3 hotels.)
            } else if ((availableHouses >= 8) && (availableHouses < 12)) {
                int mostExpensiveHotel = 0;
                int secondExpensiveHotel = 0;
                
                foreach (Land titleDeed in propertyLevel[2]) {
                    
                    if (titleDeed.Level == Level.Hotel) {
                        
                        if (titleDeed.BuildCost > secondExpensiveHotel) {
                            
                            if (titleDeed.BuildCost >= mostExpensiveHotel) {
                                secondExpensiveHotel = mostExpensiveHotel;
                                mostExpensiveHotel = titleDeed.BuildCost;
                            } else {
                                secondExpensiveHotel = titleDeed.BuildCost;
                            }
                        }
                    }
                }

                networth += (mostExpensiveHotel + secondExpensiveHotel) / 2;

            //have enough houses so that we can only turn 1 hotel into a set of four houses. (So switch most expensive hotels from sets of 2 or 3 hotels.)
            } else if ((availableHouses >= 4) && (availableHouses < 8)) {
                int mostExpensiveHotel = 0;
                foreach (Land titleDeed in propertyLevel[2]) {
                    if (titleDeed.Level == Level.Hotel) {
                        mostExpensiveHotel = Math.Max(mostExpensiveHotel, titleDeed.BuildCost);
                    }
                }
                foreach (Land titleDeed in propertyLevel[3]) {
                    if (titleDeed.Level == Level.Hotel) {
                        mostExpensiveHotel = Math.Max(mostExpensiveHotel, titleDeed.BuildCost);
                    }
                }
                networth += mostExpensiveHotel / 2;
            }

            return networth;
        }

        public int[] networthBuilding() {
            int[] buildings = new int[] {0, 0};

            foreach (TitleDeed titleDeed in _inventory.Values) {
                if (titleDeed.GetType() == typeof(Land)) {
                    int level = (int)(((Land)titleDeed).Level);
                    buildings[0] += level % 5;
                    buildings[1] += level / 5;
                }
            }

            return buildings;
        }
        #endregion Networth

        #region Property Transactions
        public void addTitleDeed(TitleDeed titleDeed) {
            if (titleDeed != null) {
                _inventory.Add(titleDeed.Name, titleDeed);
                Irc.Output(Irc.getChannel(), ".acquired " + _name + " " + (int)titleDeed.Name);
            }
        }
        
        public void removeTitleDeed(TitleDeed titleDeed) {
            if (_inventory.ContainsKey(titleDeed.Name)) {
                _inventory.Remove(titleDeed.Name);
            }
        }
        
        public void inheritInventory(Dictionary<Tile, TitleDeed> inventory) {
            string newDebt = "";
            string acquired = "";
            
            foreach (TitleDeed titleDeed in inventory.Values) {
                titleDeed.Owner = this;
                titleDeed.resetLevel();
                _inventory.Add(titleDeed.Name, titleDeed);

                if (titleDeed.Mortgaged) {
                    newDebt += " " + (int)titleDeed.Name;
                    UnhandledDebtList.addPropertyDebt(this, titleDeed);
                } else {
                    acquired += " " + (int)titleDeed.Name;
                }

            }

            if (newDebt.Length > 0) {
                Irc.Output(Irc.getChannel(), ".newPropertyDebt " + _name + newDebt);
            }
            if (acquired.Length > 0) {
                Irc.Output(Irc.getChannel(), ".acquired " + _name + acquired);
            }

            int unhandledDebt = UnhandledDebtList.totalDebt(this);
            if (unhandledDebt > networthCash()) {
                declareBankruptcy(null);
            }
        }
        #endregion Property Transactions

        #region Cash Flow
        public void addCash(int money) {
            if (money > 0) {
                _cash += money;
                Irc.Output(Irc.getChannel(), ".balance " + _name + " " + _cash);
            }
        }

        public void payPlayer(int amount, Player payee) {
            if (amount <= _cash) {
                payFlatFee(amount);
                payee.addCash(amount);
            } else if (amount > networthCash()) {
                declareBankruptcy(payee);
            } else {
                UnhandledDebtList.addPlayerDebt(this, payee, amount);
            }
        }

        public void payFlatFee(int amount) {
            if (_cash >= amount) {
                _cash -= amount;
                Irc.Output(Irc.getChannel(), ".balance " + _name + " " + _cash);
            } else if (amount > networthCash()) {
                declareBankruptcy(null);
            } else {
                UnhandledDebtList.addCashDebt(this, amount);
            }
        }

        public void payPercentTax(int percent) {
            int networth = networthTax();
            payFlatFee(networth * percent / 100);
        }

        public void payConstructionTax(int houses, int hotels) {
            int[] construction = networthBuilding();
            payFlatFee(construction[0] * houses + construction[1] * hotels);
        }

        #endregion Cash Flow

        #region Jail Behavior

        public void sendToJail() {
            Irc.Output(Irc.getChannel(), ".move " + _name + " jail teleport");
            _jailStatus = JailStatus.TurnOne;
            moveAndEval(Move.Teleport, (int)Tile.Jail);
        }

        public void releaseFromJail() {
            if (_jailStatus != JailStatus.Outside) {
                Irc.Output(Irc.getChannel(), ".move " + _name + " " + (int)Tile.Jail);
                _jailStatus = JailStatus.Outside;
            }
        }

        public void releaseFromJail(Amounts fee) {
            if (_jailStatus != JailStatus.Outside) {
                Irc.Output(Irc.getChannel(), ".move " + _name + " " + (int)Tile.Jail);
                _jailStatus = JailStatus.Outside;
                payFlatFee((int)fee);
            }
        }

        public void increaseJailTime() {
            if (_jailStatus == JailStatus.TurnOne) {
                _jailStatus = JailStatus.TurnTwo;
            } else if (_jailStatus == JailStatus.TurnTwo) {
                _jailStatus = JailStatus.TurnThree;
            }
        }

        public bool hasChanceOutOfJailFreeCard() {
            return GetOutOfJailFreeCard.ChanceOwner == this;
        }

        public bool hasCommunityChestOutOfJailFreeCard() {
            return GetOutOfJailFreeCard.CommunityChestOwner == this;
        }

        public void releaseChanceOutOfJailCard() {
            if (GetOutOfJailFreeCard.ChanceOwner == this)
                GetOutOfJailFreeCard.ChanceOwner = null;
        }

        public void releaseCommunityChestOutOfJailCard() {
            if (GetOutOfJailFreeCard.CommunityChestOwner == this)
                GetOutOfJailFreeCard.CommunityChestOwner = null;
        }

        #endregion Jail Behavior

        #region Bankruptcy
        
        public void declareBankruptcy(Player bankrupter) {
            if (bankrupter == null) {
                foreach (TitleDeed titleDeed in _inventory.Values) {
                    titleDeed.reset();
                }
                if ( hasChanceOutOfJailFreeCard() ) {
                    releaseChanceOutOfJailCard();
                }

                if ( hasChanceOutOfJailFreeCard() ) {
                    releaseCommunityChestOutOfJailCard();
                }

                Irc.Output(Irc.getChannel(), ".bankrupt " + _name);
            } else {
                if ( hasChanceOutOfJailFreeCard() ) {
                    GetOutOfJailFreeCard.ChanceOwner = bankrupter;                    
                }

                if ( hasCommunityChestOutOfJailFreeCard() ) {
                    GetOutOfJailFreeCard.CommunityChestOwner = bankrupter;                    
                }

                Irc.Output(Irc.getChannel(), ".bankrupt " + _name + " " + bankrupter.Name);
                payPlayer(_cash, bankrupter);
                bankrupter.inheritInventory(_inventory);
            }
            _isBankrupt = true;
            _inventory.Clear();
            UnhandledDebtList.remove(this);
        }

        #endregion

        #region Board Movement

        /*
         * There are three types of moves:
         * - A relative move. This is usually a forward move when you roll the dice, but can be a back move due to the "move back 3 spaces" chance card.
         * - An absolute move. Occurs when you draw a card that says to go to a specific tile. You can STILL pass go.
         * - A teleport. You go to a specific tile, but cannot pass go. ("Go directly to jail.")
         */
        public void moveAndEval(Move move, int value) {

            simpleMove(move, value);
            Board.evaluateLocation(this, value);
        }

        public void simpleMove(Move move, int value) {
            int oldLocation = _location;

            if (Move.Relative == move) {    //Relative Move
                _location = (_location + value);
                if (_location >= 0) {
                    _location %= Board.Size;
                } else {
                    _location = Board.Size - (Math.Abs(_location) % Board.Size);
                }
            } else {                        //Absolute or Teleport move
                _location = value;
            }

            string output = ".move " + _name + " " + _location;

            if (move == Move.Teleport) {
                output += " teleport";
                _location = value;
            } else if (value < 0) {
                output += " backward";
            }

            if (_jailStatus != JailStatus.TurnOne) {
                Irc.Output(Irc.getChannel(), output);
            }

            if (_location < oldLocation) {
                if ((move == Move.Teleport) && (value == 10)) {
                    return;
                }
                addCash((int)Amounts.PassGo);
            }
        }
        #endregion

        public void tellInventory(Player player) {
            string output = ".inventory " + player.Name + " " + _name + " " + _cash;

            foreach (TitleDeed titleDeed in _inventory.Values) {
                output += " " + (int)titleDeed.Name;

                if (titleDeed.Mortgaged) {
                    output += "m";
                }
                
            }

            Irc.Output(Irc.getChannel(), output);

        }

    }
}
