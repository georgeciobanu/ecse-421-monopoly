using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public static class GetOutOfJailFreeCard {
        private static Player _chanceOwner = null;
        private static Player _communityChestOwner = null;
        public static Player ChanceOwner {
            get {
                return _chanceOwner;
            }
            set {
                if ( _chanceOwner != null )
                    Irc.Output(Irc.getChannel(), ".loseCard " + _chanceOwner.Name + " chance");

                _chanceOwner = value;

                if ( value != null ) {
                    Irc.Output(Irc.getChannel(), ".getCard " + _chanceOwner.Name + " chance");
                }
            }
        }

        public static Player CommunityChestOwner {
            get {
                return _communityChestOwner;
            }
            set {
                if (_communityChestOwner != null)
                    Irc.Output(Irc.getChannel(), ".loseCard " + _communityChestOwner.Name + " communityChest");

                _communityChestOwner = value;

                if (_communityChestOwner != null)
                    Irc.Output(Irc.getChannel(), ".getCard " + _communityChestOwner.Name + " communityChest");
            }        
        }
    }
}

            
