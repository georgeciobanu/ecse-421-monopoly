using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    class Railroad : TitleDeed {
        public Railroad(Tile name, int cost, int[] rent, Set set)
            : base(name, cost, rent, set) {
        }

        #region Properties
        public override int TaxValue {
            get {
                return _cost;
            }
        }
        public override int CashValue {
            get {
                return _isMortgaged ? 0 : _mortgage;
            }
        }
        #endregion Properties

        public override int getRent(int roll) {

            if (_isMortgaged) {
                return 0;
            }

            LinkedList<TitleDeed> setList = Property.getSet(_set);
            int railroadsOwned = Property.ownedInSet(_owner, _set);

            if (railroadsOwned == 0) {
                return 0;
            } else {
                return _rent[railroadsOwned - 1];
            }
        }
    }
}
