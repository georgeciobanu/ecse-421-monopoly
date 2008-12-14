using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public class ActivePlayer {
        private int _consecutiveDoubles = 0;
        private Player _player = null;
        private bool _canRoll = false;
        private bool _canBuy = false;
        private bool _endedTurn = false;
        private bool _frozen = false;
        private int _backupRoll = 0;

        #region Properties
        public bool CanBuy {
            get {
                return _canBuy;
            }
            set {
                _canBuy = value;
            }
        }
        public bool CanRoll {
            get {
                return _canRoll;
            }
            set {
                _canRoll = value;
            }
        }
        public bool EndedTurn {
            get {
                return _endedTurn;
            }
            set {
                _endedTurn = value;
            }
        }
        public Player Player {
            get {
                return _player;
            }
        }
        public int Doubles {
            get {
                return _consecutiveDoubles;
            }
            set {
                _consecutiveDoubles = value;
            }
        }
        public bool Frozen {
            get {
                return _frozen;
            }
        }
        #endregion Properties

        public void set(Player player) {
            _player = player;
            _consecutiveDoubles = 0;
            _canRoll = true;
            _canBuy = false;
            _endedTurn = false;
        }

        public void freeze(int roll) {
            _backupRoll = roll;
            _frozen = true;
        }

        public int unfreeze() {
            if (!UnhandledDebtList.hasPlayer(_player)) {
                int temp = _backupRoll;
                _backupRoll = 0;
                _frozen = false;
                return temp;
            } else {
                return 0;
            }
        }
    }
}
