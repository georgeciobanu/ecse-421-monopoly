using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public class UnhandledDebt {
        private Player _player;
        private bool _incomeTax = false;
        private Dictionary<Tile, TitleDeed> _propertyDebt = new Dictionary<Tile, TitleDeed>();
        private Dictionary<Player, int> _playerDebt = new Dictionary<Player, int>();
        private int _cashDebt = 0; 

     #region Properties
        public Player Player {
            get {
                return _player;
            }
        }
        public bool IncomeTax {
            get {
                return _incomeTax;
            }
        }
        public int CashDebt {
            get {
                return _cashDebt;
            }
        }
        public int Creditors {
            get {
                return _playerDebt.Count;
            }
        }
        #endregion Properties

        public UnhandledDebt(Player player) {
            _player = player;
        }

        #region Cash Debt
        public void addCashDebt(int amount) {
            if (amount > 0) {
                _cashDebt += amount;
                Irc.Output(Irc.getChannel(), ".cashDebt " + _player.Name + " " + _cashDebt);
            }
        }

        public bool hasCashDebt() {
            return (_cashDebt > 0);
        }

        public void payCashDebt() {
            if (hasCashDebt() && _player.Cash >= _cashDebt) {
                _player.payFlatFee(_cashDebt);
                _cashDebt = 0;
            }
        }

        public void removeCashDebt() {
            _cashDebt = 0;
        }
        #endregion Cash Debt

        #region Property Debt
        public void addPropertyDebt(TitleDeed titleDeed) {
            if (titleDeed.Mortgaged && (titleDeed.Owner == _player)) {
                _propertyDebt.Add(titleDeed.Name, titleDeed);
            }
        }

        public bool hasPropertyDebt(Tile tile) {
            return _propertyDebt.ContainsKey(tile);
        }

        public bool hasPropertyDebt() {
            return _propertyDebt.Count > 0;
        }

        public void payPropertyDebt(Tile tile) {
            if (hasPropertyDebt(tile)) {

                TitleDeed titleDeed = Property.getTitleDeed(tile);
                int owed = (int)Math.Ceiling(titleDeed.Mortgage / 10f);

                if (_player.Cash >= owed) {
                    _player.payFlatFee(owed);
                    _propertyDebt.Remove(tile);
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + _player.Name + " insufficient funds to repay debt.");
                }

            }
        }

        public void removePropertyDebt(Tile tile) {
            if (hasPropertyDebt(tile)) {
                _propertyDebt.Remove(tile);
            }
        }
        #endregion Property Debt

        #region Player Debt
        public void addPlayerDebt(Player payee, int amount) {
            if ((payee != null) && (!payee.Bankrupt) && (payee != _player)) {
                _playerDebt.Add(payee, amount);
                Irc.Output(Irc.getChannel(), ".newPlayerDebt " + _player.Name + " " + payee.Name + " " + amount);
            }
        }

        public bool hasPlayerDebt(Player player) {
            return _playerDebt.ContainsKey(player);
        }

        public bool hasPlayerDebt() {
            return _playerDebt.Count > 0;
        }

        public void payPlayerDebt(Player player) {
            if (hasPlayerDebt(player)) {
                int owed = _playerDebt[player];
                if (_player.Cash >= owed) {
                    _player.payPlayer(owed, player);
                    _playerDebt.Remove(player);
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + _player.Name + " insufficient funds to repay debt.");
                }
            }
        }

        public void removePlayerDebt(Player player) {
            if (hasPlayerDebt(player)) {
                _playerDebt.Remove(player);
            }
        }
        #endregion Player Debt

        #region Tax Debt
        public void addTaxDebt() {
            Irc.Output(Irc.getChannel(), ".incomeTaxes " + Player.Name);

            int networthCash = _player.networthCash();
            int networthTax = _player.networthTax();


            if ((networthCash < (int)Amounts.IncomeTaxFlat) && (networthCash < (int)Math.Ceiling(networthTax * (int)Amounts.IncomeTaxPercent / 100f))) {
                _player.declareBankruptcy(null);
            }

            _incomeTax = true;
        }

        public bool hasTaxDebt() {
            return _incomeTax; ;
        }

        public void payTaxDebt(Amounts fee) {
            if (_incomeTax) {
                _incomeTax = false;

                if (fee == Amounts.IncomeTaxFlat) {
                    _player.payFlatFee((int)fee);
                    Irc.Output(Irc.getChannel(), ".paidTax " + Player.Name);
                } else if (fee == Amounts.IncomeTaxPercent) {
                    _player.payPercentTax((int)fee);
                    Irc.Output(Irc.getChannel(), ".paidTax " + Player.Name);
                } else {
                    _incomeTax = true;
                }
            }    
        }
        #endregion Tax Debt 

        public bool hasDebt() {
            return hasPropertyDebt() || _incomeTax || hasPlayerDebt() || hasCashDebt();
        }

        public int totalDebt() {
            int taxes = 0;

            if (_incomeTax) {
                int flatTax = (int)Amounts.IncomeTaxFlat;
                int percentTax = (int)Math.Ceiling((_player.networthTax() * (int)Amounts.IncomeTaxPercent) / 100f);
                
                taxes = Math.Min(flatTax, percentTax);
            }


            int mortgage = 0;

            foreach (TitleDeed titleDeed in _propertyDebt.Values) {
                mortgage += (int)Math.Ceiling(titleDeed.Mortgage / 10f);
            }

            int playerDebt = 0;

            foreach (int i in _playerDebt.Values) {
                playerDebt += i;
            }
            
            return mortgage + taxes + playerDebt + _cashDebt;
        }

        public Player getSoleCreditor() {
            if (_playerDebt.Count != 0) {
                return null;
            } else {
                return _playerDebt.Keys.ToArray()[0];
            }
        }
        
        public void viewDebt() {
            if (_incomeTax) {
                Irc.Output(Irc.getChannel(), ".taxDebt " + _player.Name + " incomeTax"); 
            }

            if (_propertyDebt.Count != 0) {
                string output = ".propertyDebt " + _player.Name;
                foreach (Tile tile in _propertyDebt.Keys) {
                    output += " " + (int)tile;
                }
                Irc.Output(Irc.getChannel(), output); 
            }

            if (_playerDebt.Count != 0) {
                string output = ".playerDebt " + _player.Name;
                foreach (KeyValuePair<Player,int> pair in _playerDebt) {
                    output += " " + pair.Key.Name + " " + pair.Value;
                }
                Irc.Output(Irc.getChannel(), output);
            }

            if (_cashDebt > 0) {
                Irc.Output(Irc.getChannel(), ".cashDebt " + _player.Name + " " + _cashDebt);
            }

        }

    }
}
