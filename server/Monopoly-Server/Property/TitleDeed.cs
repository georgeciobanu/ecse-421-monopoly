using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public abstract class TitleDeed {

        protected Tile _name;
        protected Level _level = Level.Default;

        protected int _cost;
        protected int _mortgage;
        protected bool _isMortgaged = false;

        protected int[] _rent;

        protected Set _set;

        protected Player _owner = null;

        #region Properties
        public int Cost {
            get {
                return _cost;
            }
        }
        public int Mortgage {
            get {
                return _mortgage;
            }
        }
        public Set Set {
            get {
                return _set;
            }
        }
        public Tile Name {
            get {
                return _name;
            }
        }
        public Player Owner {
            get {
                return _owner;
            }
            set {
                _owner = value;
            }
        }
        public Level Level {
            get {
                return _level;
            }
        }
        public abstract int TaxValue {
            get;
        }
        public abstract int CashValue {
            get;
        }
        public bool Mortgaged {
            get {
                return _isMortgaged;
            }
        }
        #endregion Properties

        public TitleDeed(Tile name, int cost, int[] rent, Set set) {
            _mortgage = cost / 2;
            _name = name;
            _cost = cost;
            _rent = rent;
            _set = set;
        }

        public void mortgage() {
            if (_isMortgaged) {
                Irc.Output(Irc.getChannel(), ".fail " + Owner.Name + " " + _name + " is already mortgaged.");
            } else {
                LinkedList<TitleDeed> titleDeedList = Property.getSet(_set);
                foreach (TitleDeed titleDeed in titleDeedList) {
                    if (titleDeed.getLevel() != Level.Default) {
                        Irc.Output(Irc.getChannel(), ".fail " + Owner.Name + " Entire set isn't at base level.");
                        return;
                    }
                }

                _isMortgaged = true;
                Irc.Output(Irc.getChannel(), ".mortgage " + Owner.Name + " " + (int)_name);
                _owner.addCash(_cost / 2);
            }
        }

        public void unmortgage() {
            if (_isMortgaged) {
                //TODO: don't allow unmortgaging if you have outstanding debt.
                int fee = (_mortgage * 110) / 100;
                if (Owner.Cash >= fee) {
                    Owner.payFlatFee(fee);
                    _isMortgaged = false;
                    Irc.Output(Irc.getChannel(), ".unmortgage " + Owner.Name + " " + (int)_name);
                } else {
                    Irc.Output(Irc.getChannel(), ".fail " + Owner.Name + " insufficient funds, need " + fee + " have " + Owner.Cash);
                }
            } else {
                return;
            }
        }


        //TODO: is there a better way of doing this?
        public abstract int getRent(int roll);

        public void resetLevel() {
            _level = Level.Default;
        }

        public void reset() {
            _level = Level.Default;
            _isMortgaged = false;
            _owner = null;
        }

        public Level getLevel() {
            return _level;
        }

        public void upgrade() {
            if (_level == Level.Hotel) {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " already fully upgraded.");
                return;
            } else if (this.GetType() != typeof(Land)) {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " can only upgrade land.");
                return;
            } else if (!Property.hasFullSet(_owner, _set)) {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " can't upgrade without owning the full set.");
                return;
            } else if (((Land)this).BuildCost > _owner.Cash) {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " insufficient funds to do the building.");
                return;
            }

            LinkedList<TitleDeed> setList = Property.getSet(_set);
            int minLevel = Int32.MaxValue;

            foreach (TitleDeed td in setList) {
                if (td.Mortgaged) {
                    Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " when upgrading all titledeeds in set must be unmortgaged.");
                    return;
                } else {
                    minLevel = (int)Math.Min(minLevel, (int)td.getLevel());
                }
            }

            if (minLevel == (int)_level) {
                if ((_level < Level.Houses4) && (Property.Houses == 0)) {
                    Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " no houses left in the bank.");
                } else if ((_level == Level.Houses4) && (Property.Hotels == 0)) {
                    Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " no hotels left in the bank.");
                } else {
                    ++_level;
                    if (_level == Level.Hotel) {
                        Property.Hotels--;
                        Property.Houses += 4;
                    } else {
                        Property.Houses--;
                    }
                    _owner.payFlatFee(((Land)this).BuildCost);
                    Irc.Output(Irc.getChannel(), ".rentLevel " + (int)_name + " " + (int)_level);
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " upgrading must be done uniformly.");
            }

        }

        public void downgrade() {
            if (_level == Level.Default) {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " already fully downgraded.");
                return;
            } else if (this.GetType() != typeof(Land)) {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " can only downgrade land.");
                return;
            } else if (!Property.hasFullSet(_owner, _set)) {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " can't downgrade without owning the full set.");
                return;
            }

            LinkedList<TitleDeed> setList = Property.getSet(_set);
            int maxLevel = Int32.MinValue;

            foreach (TitleDeed td in setList) {
                maxLevel = (int)Math.Max(maxLevel, (int)td.getLevel());
            }

            if (maxLevel == (int)_level) {
                if ((_level == Level.Hotel) && (Property.Houses < 4)) {
                    Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " insufficient houses left in the bank, to downgrade the hotel.");
                } else {
                    if (_level == Level.Hotel) {
                        Property.Hotels++;
                        Property.Houses -= 4;
                    } else {
                        Property.Houses++;
                    }
                    --_level;
                    _owner.addCash(((Land)this).BuildCost >> 1);
                    Irc.Output(Irc.getChannel(), ".rentLevel " + (int)_name + " " + (int)_level);
                }
            } else {
                Irc.Output(Irc.getChannel(), ".fail " + _owner.Name + " downgrading must be done uniformly.");
            }

        }
    }
}
