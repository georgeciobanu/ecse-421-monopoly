using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public class Card {        

        private const int _numChanceCards = 16;
        private const int _numCommunityChestCards = 16;
        private static ChanceCards _chanceCard;
        private static CommunityChestCards _communityChestCard;
        private static Queue<Player> _players = null;
        private static Random random = null;

        public Card(Queue<Player> playersList) {
            _players = playersList;
            random = new Random();
        }

        #region Chance Cards
        public static int GetChanceCard( Player player ) {
            do {
                _chanceCard = (ChanceCards) random.Next(1, _numChanceCards + 1);
            } while ( GetOutOfJailFreeCard.ChanceOwner != null & _chanceCard == ChanceCards.GetOutOfJailCard );
            return (int)_chanceCard;
        }
        public static void RunChanceCard( Player player ) {
            int location = 0;
            int move = 0;
         
            switch ( _chanceCard ) {
                case ChanceCards.AdvanceGo: player.moveAndEval(Move.Absolute, (int) Tile.Go); break;
                case ChanceCards.AdvanceBoardWalk: player.moveAndEval(Move.Absolute, (int) Tile.Boardwalk); break;
                case ChanceCards.AdvanceIllinoisAve: player.moveAndEval(Move.Absolute, (int) Tile.IllinoisAvenue); break;
                case ChanceCards.AdvanceNearestRailroad: {
                        location = player.Location;
                        location %= 10;

                        if ( location < 5 ) {
                            move = 5 - location;
                        } else {
                            move = 15 - location;
                        }

                        player.simpleMove(Move.Relative, move);

                        TitleDeed titleDeed = Property.getTitleDeed((Tile) player.Location);
                        Player railroadOwner = titleDeed.Owner;
                        if ( ( railroadOwner != player ) && ( railroadOwner != null ) ) {
                            //for RailRoad the parameter passed to getRent is ignored
                            player.payPlayer(( (Railroad) titleDeed ).getRent(-1) * 2, railroadOwner);
                        } else if ( railroadOwner == null ) {
                            Irc.Output(Irc.getChannel(), ".offer " + player.Name + " " + (int) titleDeed.Name + " " + titleDeed.Cost);
                        }
                    }; break;
                case ChanceCards.AdvanceNearestRailroad2: {
                        location = player.Location;
                        location %= 10;

                        if ( location < 5 ) {
                            move = 5 - location;
                        } else {
                            move = 15 - location;
                        }

                        player.simpleMove(Move.Relative, move);

                        TitleDeed titleDeed = Property.getTitleDeed((Tile) player.Location);
                        Player railroadOwner = titleDeed.Owner;
                        if ( ( railroadOwner != player ) && ( railroadOwner != null ) ) {
                            //for RailRoad the parameter passed to getRent is ignored
                            player.payPlayer(( (Railroad) titleDeed ).getRent(-1) * 2, railroadOwner);
                        } else if ( railroadOwner == null ) {
                            Irc.Output(Irc.getChannel(), ".offer " + player.Name + " " + (int) titleDeed.Name + " " + titleDeed.Cost);
                        }
                    }; break;
                case ChanceCards.AdvanceNearestUtility: {
                        if ( ( player.Location < (int) Tile.ElectricCompany ) || ( player.Location >= (int) Tile.WaterWorks ) ) {
                            player.simpleMove(Move.Absolute, (int) Tile.ElectricCompany);
                        } else {
                            player.simpleMove(Move.Absolute, (int) Tile.WaterWorks);
                        }

                        TitleDeed titleDeed = Property.getTitleDeed((Tile) player.Location);
                        Player utilityOwner = titleDeed.Owner;
                        if ( ( utilityOwner != player ) && ( utilityOwner != null ) ) {
                            int die1, die2;
                            die1 = Die.Roll();
                            die2 = Die.Roll();

                            player.payPlayer(( (Utility) titleDeed ).getRent(( die1 + die2 ) * 10), utilityOwner);
                        } else if ( utilityOwner == null ) {
                            Irc.Output(Irc.getChannel(), ".offer " + player.Name + " " + (int) titleDeed.Name + " " + titleDeed.Cost);
                        }
                    }; break;
                case ChanceCards.AdvanceStCharles: player.moveAndEval(Move.Absolute, (int) Tile.StCharlesPlace); break;
                case ChanceCards.Back3: player.moveAndEval(Move.Relative, -3); break;
                case ChanceCards.BankDivident: player.addCash(50); break;
                case ChanceCards.BuildingAndLoanMatures: player.addCash(150); break;
                case ChanceCards.ChairmanOfBoard: {
                    foreach ( Player otherPlayer in _players ) {
                        if ( otherPlayer != player ) {
                            if ( !otherPlayer.Bankrupt && !player.Bankrupt) {
                                player.payPlayer(50, otherPlayer);
                            }
                        }
                    }
                }; break;
                case ChanceCards.GeneralRepairs: {
                    player.payFlatFee(player.Hotels * 100 + player.Houses * 25);
                } break;
                case ChanceCards.GetOutOfJailCard: GetOutOfJailFreeCard.ChanceOwner = player; break;
                case ChanceCards.JailNoGo: player.sendToJail(); break;
                case ChanceCards.PoorTax: player.payFlatFee(15); break;
                case ChanceCards.RideOnReading: player.moveAndEval(Move.Absolute, (int)Tile.ReadingRailroad); break;
            }            
        }
        #endregion 


        #region Community Chest Cards
        public static int GetCommunityChestCard( Player player ) {
            do {
                _communityChestCard = (CommunityChestCards) random.Next(1, _numCommunityChestCards + 1);
            } while ( GetOutOfJailFreeCard.CommunityChestOwner != null & _communityChestCard == CommunityChestCards.GetOutOfJailCard );
            return (int)_communityChestCard;
        }

        public static void RunCommunityChestCard( Player player ) {
            //the number is generated in the Get function
            switch ( _communityChestCard ) {
                case CommunityChestCards.AdvanceGo: player.moveAndEval(Move.Absolute, (int) Tile.Go); break;
                case CommunityChestCards.BankError: player.addCash(200); break;
                case CommunityChestCards.LifeInsuranceMatures: player.addCash(100); break;
                case CommunityChestCards.DoctorFee: player.payFlatFee(50); break;
                case CommunityChestCards.FromSaleOfStock: player.addCash(45); break;
                case CommunityChestCards.GetOutOfJailCard: GetOutOfJailFreeCard.CommunityChestOwner = player; break; 
                case CommunityChestCards.GoJailNoCollect: player.sendToJail(); break;
                case CommunityChestCards.GrandOperaOpening: {
                    foreach ( Player otherPlayer in _players ) {
                        if ( otherPlayer != player ) {
                            if ( !otherPlayer.Bankrupt && !player.Bankrupt) {
                                otherPlayer.payPlayer(50, player);
                            }
                        }
                    }
                }; break;
                case CommunityChestCards.IncomeTaxRefund: player.addCash(20); break;
                case CommunityChestCards.Inherit: player.addCash(100); break;
                case CommunityChestCards.PayHospital: player.payFlatFee(100); break;
                case CommunityChestCards.ReceiveForServices: player.addCash(25); break;
                case CommunityChestCards.SchoolTax: player.payFlatFee(150); break;
                case CommunityChestCards.SecondPrizeBeautyContest: player.addCash(10); break;
                case CommunityChestCards.StreetRepairs: {
                    player.payFlatFee(player.Hotels * 115 + player.Houses * 40);
                }; break;
                case CommunityChestCards.XMASFundMatures: player.addCash(100); break;
            }            
        }
        #endregion
    }
}
