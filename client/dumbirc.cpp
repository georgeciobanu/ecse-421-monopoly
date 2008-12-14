
#include <iostream>
#include <string>

#include "irc_lines.cpp"

int main() {

    try {

        irc_lines lines("dumbirc");

        lines.put("JOIN #monopoly-bot-testing");
        
        for (;;) {
            {
                lines.poll();
                while (lines.has()) {
                    std::string s = lines.get();
                    if (s.empty()) continue;
                    std::cout << "Server Said: " << s << std::endl;    
                }
            }
            {              
                std::cout << "Say something: ";
                std::string s;
                if (!std::getline( std::cin, s, '\n' )) {
                    break;
                }
                if (s.empty()) continue;
                lines.put(s);            
            }
        }

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    std::cout << std::endl;

    return 0;

}

