using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    class Land : TitleDeed {
        int _buildCost;

        public Land(Tile name, int cost, int buildCost, int[] rent, Set set)
            : base(name, cost, rent, set) {
                _buildCost = buildCost;
        }

        #region Properties
        public int BuildCost {
            get {
                return _buildCost;
            }
        }

        public override int TaxValue {
            get {
                return _cost + (int)_level * _buildCost;
            }
        }

        public override int CashValue {
            get {
                return _isMortgaged ? 0 : _mortgage + ((int)_level * _buildCost) / 2;
            }
        }
        #endregion Properties

        public override int getRent(int roll) {

            if (_isMortgaged) {
                return 0;
            }

            if (_level == Level.Default) {

                if (Property.hasFullSet(_owner, _set)) {
                    return _rent[(int)_level] * 2;
                } else {
                    return _rent[(int)_level];
                }
            }

            return _rent[(int)_level];
        }

        public int hotelsInSet() {
            LinkedList<TitleDeed> titleDeedSet = Property.getSet(Set);
            int count = 0;
            foreach (TitleDeed titleDeed in titleDeedSet) {
                if (titleDeed.Level == Level.Hotel) {
                    ++count;
                }
            }
            return count;
        }
    }
}
