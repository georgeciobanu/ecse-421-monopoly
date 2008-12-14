using System;
using System.Collections;
using System.Threading;
using Meebey.SmartIrc4net;
using Monopoly_Server;
// This is an VERY basic example how your IRC application could be written
// its mainly for showing how to use the API, this program just connects sends
// a few message to a channel and waits for commands on the console
// (raw RFC commands though! it's later explained).
// There are also a few commands the IRC bot/client allows via private message.
public class Irc {
    // make an instance of the high-level API
    public static IrcClient _irc = new IrcClient();
    public static Game _monopolyGame = new Game();
    public static String _channel = "#monopoly-bot-testing";

    // this method we will use to analyse queries (also known as private messages)
    public static void OnQueryMessage( object sender, IrcEventArgs e ) {
        switch ( e.Data.MessageArray[0] ) {
            // debug stuff
            case "dump_channel":
                string requested_channel = e.Data.MessageArray[1];
                // getting the channel (via channel sync feature)
                Channel channel = _irc.GetChannel(requested_channel);

                // here we send messages
                _irc.SendMessage(SendType.Message, e.Data.Nick, "CHANNEL'" + requested_channel + "'");
                _irc.SendMessage(SendType.Message, e.Data.Nick, "Name:'" + channel.Name + "'");
                _irc.SendMessage(SendType.Message, e.Data.Nick, "Topic:'" + channel.Topic + "'");
                _irc.SendMessage(SendType.Message, e.Data.Nick, "Mode:'" + channel.Mode + "'");
                _irc.SendMessage(SendType.Message, e.Data.Nick, "Key:'" + channel.Key + "'");
                _irc.SendMessage(SendType.Message, e.Data.Nick, "UserLimit: '" + channel.UserLimit + "'");
                // here we go through all users of the channel and show their
                // hashtable key and nickname 
                string nickname_list = "";
                nickname_list += "Users: ";
                IDictionaryEnumerator it = channel.Users.GetEnumerator();
                while ( it.MoveNext() ) {
                    string key = (string) it.Key;
                    ChannelUser channeluser = (ChannelUser) it.Value;
                    nickname_list += key + " => " + channeluser.Nick + ", ";
                }
                _irc.SendMessage(SendType.Message, e.Data.Nick, nickname_list);
                _irc.SendMessage(SendType.Message, e.Data.Nick, "");
                break;
            case "gc":
                GC.Collect();
                break;
            // typical commands
            case "join":
                _irc.RfcJoin(e.Data.MessageArray[1]);
                break;
            case "part":
                _irc.RfcPart(e.Data.MessageArray[1]);
                break;
            case "die":
                Exit();
                break;
        }
    }
    // this method handles when we receive "ERROR" from the IRC server
    public static void OnError( object sender, ErrorEventArgs e ) {
        System.Console.WriteLine("Error: " + e.ErrorMessage);
        Exit();
    }

    // this method will get all IRC messages
    public static void OnRawMessage( object sender, IrcEventArgs e ) {
        System.Console.WriteLine("<" + e.Data.Nick + "> " + e.Data.Message);
        parseIRC(e);

    }

    public static void parseIRC( IrcEventArgs e ) {
        try {
            if (e.Data.Type == ReceiveType.Part || e.Data.Type == ReceiveType.Quit) {
                _monopolyGame.process(e.Data.Nick, "!quit");
            } else if (e.Data.Type == ReceiveType.NickChange) {
                _monopolyGame.process(e.Data.Nick, "!nick " + e.Data.Message);
                //i'm assuming the actual message is the nick - need to check
            } else if (e.Data.Type == ReceiveType.Join && !_irc.IsMe(e.Data.Nick)) {
                _monopolyGame.process(e.Data.Nick, "!enter");
                //i'm assuming the actual message is the nick - need to check
            } else if (e.Data.Nick != null && e.Data.Message != null && !_irc.IsMe(e.Data.Nick)) {
                _monopolyGame.process(e.Data.Nick, e.Data.Message);
            } else {
                //default case
            }
        } catch (Exception exception) {
            Console.WriteLine("error: " + exception.StackTrace);
        }

    }




    public static void Main( string[] args ) {
        Thread.CurrentThread.Name = "Main";


        // wait time between messages, we can set this lower on own irc servers
        _irc.SendDelay = 200;

        // we use channel sync, means we can use irc.GetChannel() and so on
        _irc.ActiveChannelSyncing = true;

        // here we connect the events of the API to our written methods
        // most have own event handler types, because they ship different data
        _irc.OnQueryMessage += new IrcEventHandler(OnQueryMessage);
        _irc.OnError += new ErrorEventHandler(OnError);
        _irc.OnRawMessage += new IrcEventHandler(OnRawMessage);
        string[] serverlist;
        // the server we want to connect to, could be also a simple string
        serverlist = new string[] { "irc.freenode.com" };
        int port = 6667;

        try {
            // here we try to connect to the server and exceptions get handled
            _irc.Connect(serverlist, port);
        }
        catch ( ConnectionException e ) {
            // something went wrong, the reason will be shown
            System.Console.WriteLine("couldn't connect! Reason: " + e.Message);
            Exit();
        }

        try {
            // here we logon and register our nickname and so on 
            _irc.Login("MonopolyServer", "SmartIrc4net Test Bot");
            // join the channel
            _irc.RfcJoin(_channel);

            // spawn a new thread to read the stdin of the console, this we use
            // for reading IRC commands from the keyboard while the IRC connection
            // stays in its own thread
            //new Thread(new ThreadStart(ReadCommands)).Start();

            // here we tell the IRC API to go into a receive mode, all events
            // will be triggered by _this_ thread (main thread in this case)
            // Listen() blocks by default, you can also use ListenOnce() if you
            // need that does one IRC operation and then returns, so you need then 
            // an own loop 
            _irc.Listen();

            // when Listen() returns our IRC session is over, to be sure we call
            // disconnect manually
            _irc.Disconnect();
        }
        catch ( ConnectionException ) {
            // this exception is handled becaused Disconnect() can throw a not
            // connected exception
            Exit();
        }
        catch ( Exception e ) {
            // this should not happen by just in case we handle it nicely
            System.Console.WriteLine("Error occurred! Message: " + e.Message);
            System.Console.WriteLine("Exception: " + e.StackTrace);
            Exit();
        }
    }
    /*
    public static void ReadCommands() {
        // here we read the commands from the stdin and send it to the IRC API
        // WARNING, it uses WriteLine() means you need to enter RFC commands
        // like "JOIN #test" and then "PRIVMSG #test :hello to you"
        while (true) {
            _irc.WriteLine(System.Console.ReadLine());
        }
    }
    */

    public static void Exit() {
        // we are done, lets exit...
        System.Console.WriteLine("Exiting...");
        System.Environment.Exit(0);
    }

    public static string getChannel() {
        return _channel;
    }

    public static void Output( string channel, string output ) {
        _irc.SendMessage(SendType.Message, channel, output);
        System.Console.WriteLine("<" + _irc.Nickname + "> " + output);
    }
}