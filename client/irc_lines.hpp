#ifndef IRC_LINES_HPP
#define IRC_LINES_HPP

#include <string>

struct irc_lines_state;

struct nickname_in_use {};

class irc_lines {
    irc_lines_state *state;
  public:
    irc_lines(std::string const &alias,
              std::string const &server = "irc.freenode.net",
              std::string const &port = "6667");
    ~irc_lines();
    
    void poll();
    bool has() const;
    std::string peek() const;
    std::string get();
    void put(std::string const &line);
    
    void sleep(unsigned ms);
};

#endif

