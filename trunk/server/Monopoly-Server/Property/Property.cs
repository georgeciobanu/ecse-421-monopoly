using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public static class Property {
        private static Dictionary<Tile, TitleDeed> _properties;
        private static Dictionary<Set, LinkedList<TitleDeed>> _set;
        private static int _houses = 32;
        private static int _hotels = 12;

        #region Properties
        public static int Houses {
            get {
                return _houses;
            }
            set {
                _houses = value;
            }
        }
        public static int Hotels {
            get {
                return _hotels;
            }
            set {
                _hotels = value;
            }
        }
        #endregion Properties

        static Property() {
            _properties = new Dictionary<Tile, TitleDeed>();
            _set = new Dictionary<Set, LinkedList<TitleDeed>>();
            int[] rent;
            LinkedList<TitleDeed> setList;
            TitleDeed titleDeed;

            #region Land
            #region Purple Tiles
            setList = new LinkedList<TitleDeed>();

            //Mediterranean Avenue
            rent = new int[] { 2, 10, 30, 90, 160, 250 };
            titleDeed = new Land(Tile.MediterraneanAvenue, 60, 50, rent, Set.Purple);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.MediterraneanAvenue, titleDeed);

            //Baltic Avenue
            rent = new int[] { 4, 20, 60, 180, 320, 450 };
            titleDeed = new Land(Tile.BalticAvenue, 60, 50, rent, Set.Purple);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.BalticAvenue, titleDeed);

            _set.Add(Set.Purple, setList);
            #endregion Purple Tiles

            #region Cyan Tiles
            setList = new LinkedList<TitleDeed>();

            //Oriental Avenue
            rent = new int[] { 6, 30, 90, 270, 400, 550 };
            titleDeed = new Land(Tile.OrientalAvenue, 100, 50, rent, Set.Cyan);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.OrientalAvenue, titleDeed);

            //Vermont Avenue
            titleDeed = new Land(Tile.VermontAvenue, 100, 50, rent, Set.Cyan);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.VermontAvenue, titleDeed);

            //Connecticut Avenue
            rent = new int[] { 8, 40, 100, 300, 450, 600 };
            titleDeed = new Land(Tile.ConnecticutAvenue, 120, 50, rent, Set.Cyan);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.ConnecticutAvenue, titleDeed);

            _set.Add(Set.Cyan, setList);
            #endregion Cyan Tiles

            #region Magenta Tiles
            setList = new LinkedList<TitleDeed>();

            //St. Charles Place
            rent = new int[] { 10, 50, 150, 450, 625, 750 };
            titleDeed = new Land(Tile.StCharlesPlace, 140, 100, rent, Set.Magenta);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.StCharlesPlace, titleDeed);

            //States Avenue
            titleDeed = new Land(Tile.StatesAvenue, 140, 100, rent, Set.Magenta);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.StatesAvenue, titleDeed);

            //Virginia Avenue
            rent = new int[] { 12, 60, 180, 500, 700, 900 };
            titleDeed = new Land(Tile.VirginiaAvenue, 160, 100, rent, Set.Magenta);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.VirginiaAvenue, titleDeed);

            _set.Add(Set.Magenta, setList);
            #endregion Magenta Tiles

            #region Orange Tiles
            setList = new LinkedList<TitleDeed>();

            //St. James Place
            rent = new int[] { 14, 70, 200, 550, 750, 950 };
            titleDeed = new Land(Tile.StJamesPlace, 180, 100, rent, Set.Orange);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.StJamesPlace, titleDeed);

            //Tennessee Avenue
            titleDeed = new Land(Tile.TennesseeAvenue, 180, 100, rent, Set.Orange);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.TennesseeAvenue, titleDeed);

            //New York Avenue
            rent = new int[] { 16, 80, 220, 600, 800, 1000 };
            titleDeed = new Land(Tile.NewYorkAvenue, 160, 100, rent, Set.Orange);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.NewYorkAvenue, titleDeed);

            _set.Add(Set.Orange, setList);
            #endregion Orange Tiles

            #region Red Tiles
            setList = new LinkedList<TitleDeed>();

            //Kentucky Avenue
            rent = new int[] { 18, 90, 250, 700, 875, 1050 };
            titleDeed = new Land(Tile.KentuckyAvenue, 220, 150, rent, Set.Red);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.KentuckyAvenue, titleDeed);

            //Indiana Avenue
            titleDeed = new Land(Tile.IndianaAvenue, 220, 150, rent, Set.Red);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.IndianaAvenue, titleDeed);

            //Illinois Avenue
            rent = new int[] { 20, 100, 300, 750, 925, 1010 };
            titleDeed = new Land(Tile.IllinoisAvenue, 240, 150, rent, Set.Red);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.IllinoisAvenue, titleDeed);

            _set.Add(Set.Red, setList);
            #endregion Red Tiles

            #region Yellow Tiles
            setList = new LinkedList<TitleDeed>();

            //Atlantic Avenue
            rent = new int[] { 22, 110, 330, 800, 975, 1150 };
            titleDeed = new Land(Tile.AtlanticAvenue, 260, 150, rent, Set.Yellow);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.AtlanticAvenue, titleDeed);

            //Ventnor Avenue
            titleDeed = new Land(Tile.VentnorAvenue, 260, 150, rent, Set.Yellow);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.VentnorAvenue, titleDeed);

            //Marvin Gardins
            rent = new int[] { 24, 120, 360, 850, 1025, 1200 };
            titleDeed = new Land(Tile.MarvinGardens, 280, 150, rent, Set.Yellow);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.MarvinGardens, titleDeed);

            _set.Add(Set.Yellow, setList);
            #endregion Yellow Tiles

            #region Green Tiles
            setList = new LinkedList<TitleDeed>();

            //Pacific Avenue
            rent = new int[] { 26, 130, 390, 900, 1100, 1275 };
            titleDeed = new Land(Tile.PacificAvenue, 300, 200, rent, Set.Green);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.PacificAvenue, titleDeed);

            //North Caroline Avenue
            titleDeed = new Land(Tile.NorthCarolinaAvenue, 300, 200, rent, Set.Green);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.NorthCarolinaAvenue, titleDeed);

            //Pennsylvania Avenue
            rent = new int[] { 28, 150, 450, 1000, 1200, 1400 };
            titleDeed = new Land(Tile.PennsylvaniaAvenue, 320, 200, rent, Set.Green);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.PennsylvaniaAvenue, titleDeed);

            _set.Add(Set.Green, setList);
            #endregion Green Tiles

            #region Blue Tiles
            setList = new LinkedList<TitleDeed>();

            //Park Place
            rent = new int[] { 35, 175, 500, 1100, 1300, 1500 };
            titleDeed = new Land(Tile.ParkPlace, 350, 200, rent, Set.Blue);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.ParkPlace, titleDeed);

            //Boardwalk
            rent = new int[] { 50, 200, 600, 1400, 1700, 2000 };
            titleDeed = new Land(Tile.Boardwalk, 400, 200, rent, Set.Blue);
            setList.AddLast(titleDeed);
            _properties.Add(Tile.Boardwalk, titleDeed);

            _set.Add(Set.Blue, setList);
            #endregion Blue Tiles
            #endregion Land

            #region Utilities
            rent = new int[] { 4, 10 };
            setList = new LinkedList<TitleDeed>();

            #region Electric Company
            titleDeed = new Utility(Tile.ElectricCompany, 150, rent, Set.Utility);
            _properties.Add(Tile.ElectricCompany, titleDeed);
            setList.AddLast(titleDeed);
            #endregion Electric Company

            #region Water Works
            titleDeed = new Utility(Tile.WaterWorks, 150, rent, Set.Utility);
            _properties.Add(Tile.WaterWorks, titleDeed);
            setList.AddLast(titleDeed);
            #endregion Water Works

            _set.Add(Set.Utility, setList);
            #endregion Utilities

            #region Rail Roads
            rent = new int[] { 25, 50, 100, 200 };
            setList = new LinkedList<TitleDeed>();

            #region Reading Railroad
            titleDeed = new Railroad(Tile.ReadingRailroad, 200, rent, Set.RailRoad);
            _properties.Add(Tile.ReadingRailroad, titleDeed);
            setList.AddLast(titleDeed);
            #endregion Reading Railroad

            #region Pennsylvania Railroad
            titleDeed = new Railroad(Tile.PennsylvaniaRailroad, 200, rent, Set.RailRoad);
            _properties.Add(Tile.PennsylvaniaRailroad, titleDeed);
            setList.AddLast(titleDeed);
            #endregion Pennsylvania Railroad

            #region B&O Railroad
            titleDeed = new Railroad(Tile.BORailroad, 200, rent, Set.RailRoad);
            _properties.Add(Tile.BORailroad, titleDeed);
            setList.AddLast(titleDeed);
            #endregion B&O Railroad

            #region Short Line
            titleDeed = new Railroad(Tile.ShortLineRailroad, 200, rent, Set.RailRoad);
            _properties.Add(Tile.ShortLineRailroad, titleDeed);
            setList.AddLast(titleDeed);
            #endregion Short Line

            _set.Add(Set.RailRoad, setList);
            #endregion Rail Roads

        }

        public static TitleDeed getTitleDeed(Tile tile) {
            if (_properties.ContainsKey(tile)) {
                return _properties[tile];
            } else {
                return null;
            }
        }

        public static LinkedList<TitleDeed> getSet(Set set) {
            if (_set.ContainsKey(set)) {
                return _set[set];
            } else {
                return null;
            }
        }

        public static bool hasFullSet(Player player, Set set) {
            if (player == null) {
                return false;
            }
            
            if (_set.ContainsKey(set)) {
                LinkedList<TitleDeed> titleDeedList = _set[set];

                foreach (TitleDeed titleDeed in titleDeedList) {
                    if (titleDeed.Owner != player) {
                        return false;
                    }
                }

                return true;
            } else {
                return false;
            }
        }

        public static void resetAll() {
            foreach (TitleDeed titleDeed in _properties.Values) {
                titleDeed.reset();
            }
        }

        public static int ownedInSet(Player player, Set set) {
            if (player == null) {
                return 0;
            }

            if (_set.ContainsKey(set)) {
                int sum = 0;
                LinkedList<TitleDeed> titleDeedList = _set[set];

                foreach (TitleDeed titleDeed in titleDeedList) {
                    if (titleDeed.Owner == player) {
                        sum++;
                    }
                }
                
                return sum;
            } else {
                return 0;
            }
        }
    }
}
