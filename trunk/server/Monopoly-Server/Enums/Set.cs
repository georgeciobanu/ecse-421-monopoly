using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public enum Set {
        Blue = 1<<1,
        Cyan = 1<<2,
        Green = 1<<3,
        Magenta = 1<<4,
        Orange = 1<<5,
        Purple = 1<<6,
        Red = 1<<7, 
        Yellow = 1<<8, 

        Utility = 1<<9,
        RailRoad = 1<<10,
    }
}
