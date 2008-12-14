using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public static class Board {
        #region Members
        private static Tile[] _theBoard = {
                Tile.Go, Tile.MediterraneanAvenue, Tile.CommunityChest,
                Tile.BalticAvenue, Tile.IncomeTax, Tile.ReadingRailroad,
                Tile.OrientalAvenue, Tile.Chance, Tile.VermontAvenue,
                Tile.ConnecticutAvenue, Tile.Jail, Tile.StCharlesPlace,
                Tile.ElectricCompany, Tile.StatesAvenue, Tile.VirginiaAvenue,
                Tile.PennsylvaniaRailroad, Tile.StJamesPlace, Tile.CommunityChest,
                Tile.TennesseeAvenue, Tile.NewYorkAvenue, Tile.FreeParking,
                Tile.KentuckyAvenue, Tile.Chance, Tile.IndianaAvenue,
                Tile.IllinoisAvenue, Tile.BORailroad, Tile.AtlanticAvenue,
                Tile.VentnorAvenue, Tile.WaterWorks, Tile.MarvinGardens,
                Tile.GoToJail, Tile.PacificAvenue, Tile.NorthCarolinaAvenue,
                Tile.CommunityChest, Tile.PennsylvaniaAvenue, Tile.ShortLineRailroad,
                Tile.Chance, Tile.ParkPlace, Tile.LuxuryTax, Tile.Boardwalk
            };
        
        #endregion Members

        #region Properties
        public static int Size {
            get {
                return _theBoard.Length;
            }
        }
        #endregion Properties

        public static void evaluateLocation(Player player, int roll) {
            Tile location = _theBoard[player.Location];
            
            if (location == Tile.GoToJail) {
                player.sendToJail();
            } else if (location == Tile.LuxuryTax) {
                player.payFlatFee((int)Amounts.LuxuryTax);
            } else if (location == Tile.IncomeTax) {
                UnhandledDebtList.addTaxesDebt(player);
            } else if (location == Tile.Chance) {
                int ChestCard = Card.GetChanceCard(player);
                Irc.Output(Irc.getChannel(), ".chance " + player.Name + " " + ChestCard.ToString());
                Card.RunChanceCard(player);
            } else if (location == Tile.CommunityChest) {
                int card = Card.GetCommunityChestCard(player);
                Irc.Output(Irc.getChannel(), ".communityChest " + player.Name + " " + card.ToString());
                Card.RunCommunityChestCard(player);
            } else if ((location == Tile.FreeParking) || (location == Tile.Jail) || (location == Tile.Go)) {
                return;
            } else {
                TitleDeed titleDeed = Property.getTitleDeed(location);
                if (titleDeed != null) {
                    Player owner = titleDeed.Owner;

                    if (owner == null) {
                        Irc.Output(Irc.getChannel(), ".offer " + player.Name + " " + (int)titleDeed.Name + " " + titleDeed.Cost);
                    } else if (owner != player) {
                        int rent = titleDeed.getRent(roll);
                        if (rent != 0) {
                            player.payPlayer(rent, owner);
                        }
                    }
                }
            }
        }

        public static Tile getTile(int index) {
            index %= _theBoard.Length;
            if (index < 0) {
                index += _theBoard.Length;
            }
            return _theBoard[index];
        }

    }
}
