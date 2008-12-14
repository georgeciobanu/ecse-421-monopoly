#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include "irc_lines.hpp"

#include <deque>

#include <cassert>

#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <asio.hpp>


struct irc_lines_state {
    asio::io_service io_service;
    asio::ip::tcp::socket socket;

    std::deque<std::string> line_queue;
    std::string partial_line;

    irc_lines_state()
     : socket(io_service) {}
     
   void sleep(unsigned ms) {
        asio::deadline_timer t(io_service,
                               boost::posix_time::millisec(ms));
//std::cerr << "Sleeping for " << ms << "ms...";
        t.wait();
//std::cerr << "Done\n";
   }
};

irc_lines::irc_lines(std::string const &alias,
                     std::string const &server,
                     std::string const &port) {

    state = new irc_lines_state;
    try {

        asio::ip::tcp::resolver resolver(state->io_service);
        asio::ip::tcp::resolver::query query(server,
                                             port, 
                                             asio::ip::tcp::resolver::query::numeric_service);
        asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        asio::ip::tcp::resolver::iterator end;

        asio::error_code error = asio::error::host_not_found;
        while (error && endpoint_iterator != end) {
            state->socket.close();
            state->socket.connect(*endpoint_iterator++, error);
        }
        if (error) throw asio::system_error(error);

        while (!has()) {
            state->sleep(100);
            poll();
        }

        put( "NICK " + alias );
        put( "USER " + alias + " dumbirc irc.freenode.net :Unknown" );
        
        for (;;) {
            while (!has()) {
                state->sleep(100);
                poll();
            }
            std::string s = get();
//        std::cerr << "Got: " << s << std::endl;
            if (s.find(alias) == s.npos) continue;
//        std::cerr << "Testing: " << s << std::endl;
            if (s.find("already in use") == s.npos) {
                break;
            } else {
                throw nickname_in_use();
            }
        }
        
    } catch (...) {
        // dtr won't do this, since the object isn't complete,
        // and auto_ptr can't be used with pImpl
        delete state;
    }                         
}

irc_lines::~irc_lines() {
//    put( "QUIT :Destructor" );
    delete state;
}

void 
irc_lines::poll() {
    if ( state->socket.available() == 0 ) {
        return;
    }

    boost::array<char, 128> buf;
    asio::error_code error;

    size_t len = state->socket.read_some(asio::buffer(buf), error);

    if (error == asio::error::eof) {
        // Connection closed cleanly by peer.
        //done = true;
        return;
    } else if (error) {
        // Some other error.
        throw asio::system_error(error);
    }
    
    for (size_t i = 0; i < len; ++i) {
        if (buf[i] == '\r') buf[i] = '\n';
    }
    
    size_t n = 0;
    for (;;) {
        size_t pastn = n;
        n = std::find( buf.begin()+pastn, buf.begin()+len, '\n' ) - buf.begin();
        
        assert( n >= pastn );
        
        if (n >= len) {
            assert( n == len );
            state->partial_line.append( buf.begin()+pastn, buf.begin()+len );
            break;
        } else {
            state->partial_line.append( buf.begin()+pastn, buf.begin()+n );
            if (!state->partial_line.empty()) {
                state->line_queue.push_back(state->partial_line);
                state->partial_line.clear();
            }
        }
        ++n;
    }
    
    // Might have more to do
    poll();

}

bool 
irc_lines::has() const {
    return !state->line_queue.empty();
}

std::string
irc_lines::peek() const {
    return state->line_queue.front();
}

std::string
irc_lines::get() {
    std::string s = peek();
    state->line_queue.pop_front();
//std::cout << " -> " << s << std::endl;
    return s;
}

void
irc_lines::put(std::string const &s) {
//std::cout << " <- " << s << std::endl;
    std::string s_ = s;
    s_ += "\r\n";
    asio::write( state->socket, asio::buffer(s_) );
//std::cerr << "Wrote: " << s << std::endl;    
}

void 
irc_lines::sleep(unsigned ms) {
    state->sleep(ms);
}

