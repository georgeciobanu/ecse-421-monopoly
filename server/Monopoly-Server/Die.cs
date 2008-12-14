using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public class Die {
    private static Random _die = new Random();

    public static int Roll() {
        return _die.Next(1,7);
    }
}