using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public static class UnhandledDebtList {
        private static Dictionary<Player, UnhandledDebt> _unhandledDebtList = new Dictionary<Player, UnhandledDebt>();

        #region Properties
        public static int Count {
            get {
                return _unhandledDebtList.Count;
            }
        }
        public static List<Player> Players {
            get {
                return _unhandledDebtList.Keys.ToList();
            }
        }

        public static bool hasPlayer(Player player) {
            return _unhandledDebtList.ContainsKey(player);
        }

        public static int totalDebt(Player player) {
            if (hasPlayer(player)) {
                return _unhandledDebtList[player].totalDebt();
            } else {
                return 0;
            }
        }

        public static void addPropertyDebt(Player player, TitleDeed titleDeed) {
            if (!hasPlayer(player)) {
                UnhandledDebt unhandledDebt = new UnhandledDebt(player);
                _unhandledDebtList.Add(player, unhandledDebt);

            }

            _unhandledDebtList[player].addPropertyDebt(titleDeed);
        }

        public static void addCashDebt(Player player, int amount) {
            if (amount <= 0) {
                return;
            }

            if (!hasPlayer(player)) {
                UnhandledDebt unhandledDebt = new UnhandledDebt(player);
                _unhandledDebtList.Add(player, unhandledDebt);

            }

            _unhandledDebtList[player].addCashDebt(amount);
        }

        public static void addPlayerDebt(Player payer, Player payee, int amount) {
            if (!hasPlayer(payer)) {
                UnhandledDebt unhandledDebt = new UnhandledDebt(payer);
                _unhandledDebtList.Add(payer, unhandledDebt);

            }

            _unhandledDebtList[payer].addPlayerDebt(payee, amount);
        }

        public static void addTaxesDebt(Player player) {
            if (!hasPlayer(player)) {
                UnhandledDebt unhandledDebt = new UnhandledDebt(player);
                _unhandledDebtList.Add(player, unhandledDebt);

            }

            _unhandledDebtList[player].addTaxDebt();
        }
        
        public static UnhandledDebt getUnhandledDebt(Player player) {
            if (hasPlayer(player)) {
                return _unhandledDebtList[player];
            } else {
                return null;
            }
        }

        public static void remove(Player player) {
            if (hasPlayer(player)) {
                _unhandledDebtList.Remove(player);
            }

            foreach (UnhandledDebt unhandledDebt in _unhandledDebtList.Values) {
                if (unhandledDebt.hasPlayerDebt(player)) {
                    unhandledDebt.removePlayerDebt(player);
                }
            }
        }

        public static void clearAllDebt() {
            _unhandledDebtList.Clear();
        }

        #endregion Properties
    }
}
