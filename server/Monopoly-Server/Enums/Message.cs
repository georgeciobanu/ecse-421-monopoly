using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Monopoly_Server {
    public enum Message {
        fail, 
        join,
        nick,
        players,
        tell,
        roll,
        rollagain,
        turn,
    }
}
