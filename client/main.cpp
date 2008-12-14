
//
// This code based off of Richard Campbell's 1999 port to GLUT of 
// NeHe Lesson #7 created by Jeff Molofee '99
//

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI (3.14159265358979)
#endif

#include <GL/glut.h>
#include "Model_3DS.h"

#include "irc_lines.hpp"
#include "property_coordinates.hpp"
#include "vectors.hpp"

#define BOOST_DATE_TIME_NO_LIB
//#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::posix_time::second_clock;
using boost::posix_time::microsec_clock;
ptime get_now() {
    return microsec_clock::universal_time();
}
using boost::posix_time::seconds;
using boost::posix_time::millisec;
using boost::posix_time::microsec;

#include <boost/bind.hpp>

///////////////////////////////////////////////////////////////////////////////

vector3ub const white_color = {{255,255,255}};
vector3ub const black_color = {{0,0,0}};

vector3ub const red_color = {{255,0,0}};
vector3ub const green_color = {{0,255,0}};
vector3ub const blue_color = {{0,255,0}};

vector3ub const fg_color = {{200,244,222}};
vector3ub const bg_color = {{22,0,44}};

void gl_color(vector3ub const &c) {
    glColor3ubv( c.v() );
}
void gl_color_alpha(vector3ub const &c, unsigned char a) {
    glColor4ub( c.x(), c.y(), c.z(), a );
}

vector3ub const player_colors[8] = {
{{255,255,0}},
{{63,255,63}},
{{63,255,255}},
{{191,191,255}},
{{255,127,127}},
{{255,127,0}},
{{255,255,255}},
{{191,191,191}} };


///////////////////////////////////////////////////////////////////////////////

#define AUTOPLAY 'a'
#define ROLL 'r'
#define BUY 'b'
#define UPGRADE 'u'
#define DOWNGRADE 'd'
#define PAY 'p' // resolve debt
#define GOJ 'g' // get out of jail
#define PERCENTTAX 'f' // fraction
#define FLATTAX 'c' // constant
#define MORTGAGE 'm'
#define UNMORTGAGE 'n'
#define ENDTURN 'e'
#define JOIN 'j'
#define START 's'
#define CONCEDE 'q'
#define BACKSPACE 8
#define TYPE 13 // ENTER
#define QUIT 27 // ESCAPE

std::string message = "";
std::deque<std::string> messagequeue;
std::deque<ptime> messagetimes;

///////////////////////////////////////////////////////////////////////////////

std::vector<std::string> owners(40, "");
std::vector<bool> mortgaged(40, false);
bool purchasable[40] = {
 0, 1, 0, 1, 0, 1, 1, 0, 1, 1,
 0, 1, 1, 1, 1, 1, 1, 0, 1, 1,
 0, 1, 0, 1, 1, 1, 1, 1, 1, 1,
 0, 1, 1, 0, 1, 1, 0, 1, 0, 1 };
std::vector<int> rent_levels(40, 0);

///////////////////////////////////////////////////////////////////////////////

// For later use in project/unproject
GLdouble modelview_matrix[16];
GLdouble projection_matrix[16];
GLint viewport_values[4];

///////////////////////////////////////////////////////////////////////////////

std::string userim; // with can["type"]

///////////////////////////////////////////////////////////////////////////////
Model_3DS *models;
//////////////////////////////////////////////////////////////////////////////

std::string alias;
std::string channel = "#monopoly-bot-testing";
std::auto_ptr<irc_lines> lines;

std::pair<std::string, std::string>
channel_get() {
    if (!lines->has()) lines->poll();
    if (lines->has()) {
        std::string const s = lines->get();
        do {
            std::istringstream ss(s);
            if (ss.get() != ':') break;
            std::string nick;
            std::getline(ss, nick, '!');
            ss.ignore(0x7777, ':');
            if (ss.eof()) break;
//            if (ss.get() != '-') break;
            std::string text;
            std::getline(ss, text, '\n');
std::cout << "<" << nick << "> " << text << std::endl;            
            return std::make_pair( nick, text );
        } while (false);
//        std::cout << s << std::endl;
    }
    return std::make_pair( std::string(), std::string() );
}

void
channel_put(std::string const &text) {
std::cout << "<> " << text << std::endl;
    lines->put( "PRIVMSG " + channel + " :" + text );
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
float
far_along(T const &a, T const &b, T const &c) {
    return float(b-a)/(c-a);
}

float
far_along(ptime const &a, ptime const &b, ptime const &c) {
    return float( (b-a).ticks() ) / (c-a).ticks();
}

float square(float x) {
    return x*x;
}

float smooth(float x) {
    return (1./3)*square(x) * ( square(3*x-4) + 2 );
}
template <typename T>
T identity(T t) {
    return t;
}

template <typename T>
std::string stringify(T const &val){
    std::stringstream out;
    out << val;
    return out.str();
}

std::string money_stringify(int d) {
    if (d < 0) {
        return "(" + money_stringify(-d) + ")";
    } else {
        return "$" + stringify(d);
    }
}

vector3ub const &money_color(int d) {
    return d > 0 ? green_color : red_color;
}

template <typename T, typename U>
T
interp(T const &a, T const &b, U const &s) {
    //return a + s*(b-a);
    return (1-s)*a + s*b;
}

template <typename T, typename U>
T
quad_spline(T const &a, T const &m, T const &b, U const &s) {
    //return a + s*(b-a);
    return square(1-s)*a + (1-s)*(s)*m + square(s)*b;
}

///////////////////////////////////////////////////////////////////////////////

struct movement {
    vector2f now_pos;
    
    virtual void
    update(ptime const &t) = 0;
    
    virtual vector2f const &
    target() = 0;

    vector2f const &
    now() const {
        return now_pos;
    }
    
    virtual bool
    done(ptime const &) const {
        return true;
    }
    
    virtual ~movement() {}
};

struct constant_movement : movement {
    virtual void
    update(ptime const &/*t*/) {
    }
    virtual vector2f const &
    target() {
        return now_pos;
    }
};
struct direct_movement : constant_movement {
    ptime start_time, end_time;
    vector2f start_pos, end_pos;
    float (*smoother)(float);
    
    direct_movement(float (*s)(float) = identity<float>)
     : smoother(s) {}
    
    virtual bool
    done(ptime const t) const {
        return t >= end_time;
    }

    virtual void
    update(ptime const &t) {
        if (t > end_time) {
            now_pos = end_pos;
            return;
        }
        now_pos = interp( start_pos, end_pos,
                          smoother( far_along( start_time, t, end_time ) ) );
    }
    virtual vector2f const &
    target() {
        return end_pos;
    }
};
struct indirect_movement : direct_movement {
    vector2f via;

    indirect_movement(float (*s)(float) = identity<float>)
     : direct_movement(s) {}

    virtual void
    update(ptime const &t) {
        if (t > end_time) {
            now_pos = end_pos;
            return;
        }
        now_pos = quad_spline( start_pos, via, end_pos,
                               smoother( far_along( start_time, t, end_time ) ) );
    }
};

std::auto_ptr<constant_movement>
camera_constify(std::auto_ptr<constant_movement> m, 
         ptime const &now) {
    if (direct_movement *p = dynamic_cast<direct_movement*>(m.get())) {
        if (p->end_time <= now) {
            std::auto_ptr<constant_movement> n( new constant_movement );
            n->now_pos = p->end_pos;
            return n;
        }
        return m;
    }
    if (indirect_movement *p = dynamic_cast<indirect_movement*>(m.get())) {
        if (p->end_time <= now) {
            std::auto_ptr<constant_movement> n( new constant_movement );
            n->now_pos = p->end_pos;
            return n;
        }
        return m;
    }
    return m;
}

/*
template <typename T>
T &
unconst(T const &t) {
    return const_cast<T&>(t);
}
*/

///////////////////////////////////////////////////////////////////////////////

struct playerdata {
    playerdata() 
     : moneydelta(), last_square(0), balance(1500) {}

    int moneydelta;
    ptime moneydelta_time;
    vector3ub color;
	int model_index;
    std::deque<constant_movement*> movements;
    int last_square;
    int balance;
    vector2f const &end() {
        return movements.back()->target();
    }
    vector2f const &pos() {
        return movements.front()->now();
    }
    void update_movements(ptime const &now) {
        assert( !movements.empty() );
        movements.front()->update(now);
        while ( movements.size() > 1 ) {
            std::auto_ptr<constant_movement> m( movements.front() );
            m = camera_constify( m, now );
            movements.front() = m.release();
            if (dynamic_cast<direct_movement*>(movements.front())
             || dynamic_cast<indirect_movement*>(movements.front())) {
                // still have work to do
                break; 
            }
            direct_movement *p = static_cast<direct_movement*>(movements[1]);
            p->end_time = now + (p->end_time - p->start_time);
            p->start_time = now;
            p->now_pos = p->start_pos = movements.front()->now();
            
            delete movements.front();
            movements.pop_front();
            movements.front()->update(now);
        }
    }
    void add_direct_movement(int square) {
std::cerr << "Direct movement to " << square << " from (" << last_square << ").\n";
        ptime const now = get_now();

        std::auto_ptr<direct_movement> n( new direct_movement );

        n->smoother = smooth;

        n->now_pos = n->start_pos = end();
        n->end_pos = ~square ? property_coordinates(square)
                             : jail_coordinates();

        n->end_pos.x() += (rand()%10-10)/1024.;
        n->end_pos.y() += (rand()%10-10)/1024.;


        n->start_time = now;
        n->end_time = now + microsec(unsigned( 1000000*(
                                length(n->end_pos - n->start_pos)
                            ) ));
        movements.push_back( n.release() );
        last_square = square;
        if ( last_square != ~0 ) {
            last_square %= 40;
            if ( last_square < 0 ) last_square += 40;
        }
    }
    void add_forward_movement(int square) {
std::cerr << "Forward movement to " << square << " from (" << last_square << ").\n";
        if ( square == last_square ) {
            return;
        }   
    
        if (last_square == ~0) {
            assert( square == 10 );
            add_direct_movement( square );
            return;
        }
        square %= 40;
        last_square %= 40;
        if ( square < 0 ) square += 40;
        if ( last_square < 0 ) last_square += 40;
        /*
        if (square < last_square) {
            add_forward_movement(square+40);
            return;
        }
        */        
        if (square > last_square && square/10 == last_square/10) {
            add_direct_movement(square);
        } else {
            add_direct_movement( (last_square/10+1)*10 );
            add_forward_movement(square);
        }
    }
    void add_backward_movement(int square) {
std::cerr << "Backward movement to " << square << " from (" << last_square << ").\n";
        if ( square == last_square ) {
            return;
        }   
    
        if (last_square == ~0) {
            assert( square == 10 );
            add_direct_movement( square );
            return;
        }
        square %= 40;
        last_square %= 40;
        if ( square < 0 ) square += 40;
        if ( last_square < 0 ) last_square += 40;
        /*
        if (square > last_square) {
            last_square += 40;
            add_backward_movement(square-40);
            return;
        }
        */
        if (square < (last_square?last_square:40) && square/10 == ((last_square-1+40)%40)/10) {
            add_direct_movement(square);
        } else {
            int step = last_square - 1;
            if ( step < 0 ) step += 40;
            step /= 10;
            step *= 10;
            add_direct_movement( step );
            add_backward_movement(square);
        }
    }
    void add_teleport_movement(int square) {
std::cerr << "Teleport movement to " << square << " from (" << last_square << ").\n";
        add_direct_movement(square);
    }
};
typedef std::map<std::string, playerdata> players_type;
players_type players;
void add_player(std::string const &n) {
    if (players.find(n) != players.end()) return;
    playerdata &d = players[n];
    d.color = player_colors[ (players.size()-1)%8 ];
	d.model_index = (players.size()-1)%7;
    d.movements.push_back( new constant_movement );
    d.movements.back()->now_pos = property_coordinates(d.last_square);
    d.movements.back()->now_pos.x() += (rand()%10-10)/1024.;
    d.movements.back()->now_pos.y() += (rand()%10-10)/1024.;
    
    std::cout << "Added player: " << n << std::endl;
}

std::string active_player = "";

///////////////////////////////////////////////////////////////////////////////

std::map<std::string, bool> can;
void init_can() {
    can.clear();
    can["join"] = true;
    can["roll"] = false;
    can["quit"] = true;
    can["end"] = false;
}

int window;

int viewport_width = 800, viewport_height = 600;


std::auto_ptr<constant_movement> camera_angle;
std::auto_ptr<constant_movement> camera_zoom;
void init_camera() {
    camera_angle.reset( new constant_movement );
    camera_angle->now_pos.x() = 0;
    camera_zoom.reset( new constant_movement );
    camera_zoom->now_pos.x() = 0;
}
void update_camera(ptime const &now) {
    camera_angle->update(now);
    camera_angle = camera_constify( camera_angle, now );
    camera_zoom->update(now);
    camera_zoom = camera_constify( camera_zoom, now );
}
void move_camera_angle(float x) {
    while (x > camera_angle->now().x() + 180) camera_angle->now_pos.x() += 360;
    while (x < camera_angle->now().x() - 180) camera_angle->now_pos.x() -= 360;

    ptime const &now = get_now();

    std::auto_ptr<direct_movement> p( new direct_movement );
    p->start_pos.x() = p->now_pos.x() = camera_angle->now().x();
    p->start_time = now;
    p->end_pos.x() = x;
    float const delta = p->end_pos.x() - p->start_pos.x();
    using std::abs;
    p->end_time = now + microsec(unsigned( 1000000*( abs(delta)*2/90 ) ));
    camera_angle = p;
}
void move_camera_zoom(float y) {
    if (y >  0.3f*2) y =  0.3f*2;
    if (y < -0.3f*4) y = -0.3f*4;

    ptime const &now = get_now();

    std::auto_ptr<direct_movement> p( new direct_movement );
    p->start_pos.x() = p->now_pos.x() = camera_zoom->now().x();
    p->start_time = now;
    p->end_pos.x() = y;
    float const delta = p->end_pos.x() - p->start_pos.x();
    using std::abs;
    p->end_time = now + microsec(unsigned( 1000000*( abs(delta)/* *10 */ ) ));
    camera_zoom = p;
}

///////////////////////////////////////////////////////////////////////////////

int active_deed = -1; // with can["see deed"]

direct_movement deed_center(smooth);
direct_movement deed_size(smooth);

GLuint deed_fronts[40];
GLuint deed_backs[40];

///////////////////////////////////////////////////////////////////////////////

GLuint card_textures[32];
GLuint *chance_cards = card_textures + 0;
GLuint *chest_cards = card_textures + 16;

int active_card = -1;

direct_movement card_tl(smooth);
direct_movement card_tr(smooth);
direct_movement card_bl(smooth);
direct_movement card_br(smooth);

vector2f chance_tl = {{ 616/1024., 1 - 752/1024.}};
vector2f chance_tr = {{ 752/1024., 1 - 616/1024.}};
vector2f chance_bl = {{ 703/1024., 1 - 839/1024.}};
vector2f chance_br = {{ 839/1024., 1 - 705/1024.}};

vector3ub chance_color = {{220,47,20}};

vector2f chest_tl = {{ 416/1024., 1 - 273/1024.}};
vector2f chest_tr = {{ 280/1024., 1 - 408/1024.}};
vector2f chest_bl = {{ 327/1024., 1 - 185/1024.}};
vector2f chest_br = {{ 192/1024., 1 - 321/1024.}};

vector3ub chest_color = {{251,180,5}};

///////////////////////////////////////////////////////////////////////////////

struct image {
    unsigned long width;
    unsigned long height;
    unsigned long bytes_per_pixel;
    std::vector<char> data;
    
    bool load_raw(std::string const &filename) {
        std::ifstream infile(filename.c_str(), std::ios::binary);
        data.resize( width * height * bytes_per_pixel );
        infile.read( &data[0], data.size() );
        return !infile.fail();
    }
    
    image() {}
    image(unsigned long w,
          unsigned long h,
          unsigned long bytes_pp,
          std::string const &filename) 
     : width(w), height(h), bytes_per_pixel(bytes_pp) {
        if ( !load_raw(filename) ) throw filename;
    }

    void to_texture(GLuint &tex) {
    
        glGenTextures(1, &tex);

        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 
                     0, bytes_per_pixel, width, height, 
                     0, bytes_per_pixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, &(data[0]));
        gluBuild2DMipmaps(GL_TEXTURE_2D, 
                          bytes_per_pixel, width, height, 
                          bytes_per_pixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, &(data[0]));
    }
};

GLuint textures[7];
int dice1, dice2;
ptime diceupdate;

void gl_load_textures() {
    image images[7];

    dice1 = 1;
    dice2 = 1;

    images[0] = image(2048, 2048, 4, "../data/board_2048x2048_RGBA.raw");
    images[0].to_texture(textures[0]);    
    
    for (int i = 1; i <= 6; i++){
        std::string test = "../data/die" + stringify(i) + "_128x128_RGB.raw";
        images[i] = image(128,128,3, test);
        images[i].to_texture(textures[i]);
    }
    
    for (unsigned i = 0; i < 40; ++i) {
        if (purchasable[i]) {
            image( 512, 512, 3, "../data/square_deeds/p" + stringify(i) + ".raw" )
             .to_texture( deed_fronts[i] );
        }
    }
    
    for (unsigned i = 0; i < 16; ++i) {
        std::string s = stringify(i+1);
        if ( s.size() == 1 ) s = "0" + s;
        image( 512, 256, 3, "../data/Chance and Community Chest/c" + s + ".raw" )
         .to_texture( chance_cards[i] );
    }
    
    for (unsigned i = 0; i < 16; ++i) {
        std::string s = stringify(i+1);
        if ( s.size() == 1 ) s = "0" + s;
        image( 512, 256, 3, "../data/Chance and Community Chest/cc" + s + ".raw" )
         .to_texture( chest_cards[i] );
    }
}

///////////////////////////////////////////////////////////////////////////////
void gl_load_models(){
	models = new Model_3DS[9];
	models[0].Load("3d/battleship.3DS");
	models[1].Load("3d/cannon.3DS");
	models[2].Load("3d/car.3DS");
	models[3].Load("3d/hat.3DS");
	models[4].Load("3d/iron.3DS");
	models[5].Load("3d/thimble.3DS");
	models[6].Load("3d/wheelbarrow.3DS");
	models[7].Load("3d/house.3DS");
	models[8].Load("3d/hotel.3DS");
}
///////////////////////////////////////////////////////////////////////////////

void gl_resize(GLsizei width, GLsizei height) {

    viewport_width = width;
    viewport_height = height;

std::cout << "Viewport: " << width << "x" << height << std::endl;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width/(height+1), 0.1f, 100.0f);

    // For later use in project/unproject
    glGetDoublev( GL_PROJECTION_MATRIX, projection_matrix );
    glGetIntegerv( GL_VIEWPORT, viewport_values );

    glMatrixMode(GL_MODELVIEW);

}

void gl_go_ortho() {

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        // The 0.5 shift puts the coordinates centered in the pixels 
        // rather than on borders
        gluOrtho2D( -0.5, viewport_width - 0.5,
                    -0.5, viewport_height - 0.5 );

    glMatrixMode(GL_MODELVIEW);

}
void gl_un_ortho() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

}

void gl_init() {

    try {
        gl_load_textures();
    } catch (std::string const &s) {
        std::cerr << "Couldn't load " << s << "!\n"
                  << "Continuing, but don't expect anything to show up.\n";
    }

	gl_load_models();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    
    gl_resize(viewport_width, viewport_height);

}

void key_press(unsigned char key, int /*x*/, int /*y*/);
void menuEventHandler(int param) {
    key_press( param, ~0, ~0 );
}

void initMenu() {

    // create the menu and
    // tell glut that "menuEventHandler" will 
    // handle the events
    
    int submenu = glutCreateMenu(menuEventHandler);
    
    glutAddMenuEntry("Join", JOIN);
    glutAddMenuEntry("Start", START);
    glutAddMenuEntry("Concede", CONCEDE);
    glutAddMenuEntry("Toggle Auto-Play", AUTOPLAY);
    glutAddMenuEntry("Quit", QUIT);

    int propertymenu = glutCreateMenu(menuEventHandler);

    glutAddMenuEntry("Upgrade", UPGRADE);
    glutAddMenuEntry("Downgrade", DOWNGRADE);
    glutAddMenuEntry("Mortgage", MORTGAGE);
    glutAddMenuEntry("Unmortgage", UNMORTGAGE);
    
    int menu = glutCreateMenu(menuEventHandler);
    
    //add entries to our menu
    glutAddSubMenu("Game", submenu);
    glutAddSubMenu("Deed", propertymenu);
    glutAddMenuEntry("Roll Dice",ROLL);
    glutAddMenuEntry("Buy Property",BUY);
//    glutAddMenuEntry("Mortgage Menu",MORTGAGE);
//    glutAddMenuEntry("Trade Menu",TRADE);
    glutAddMenuEntry("Pay Flat Tax", FLATTAX);
    glutAddMenuEntry("Pay Percentage Tax", PERCENTTAX);
    glutAddMenuEntry("Start/End Chat Message", TYPE);
    glutAddMenuEntry("End Turn", ENDTURN);
    
    // attach the menu to the right button
    menu = menu;
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

///////////////////////////////////////////////////////////////////////////////

int glut_bitmap_width(void *font, std::string const &s) {
    int accum = 0;
    for (unsigned i = 0; i < s.size(); ++i) {
        accum += glutBitmapWidth( font, s[i] );
    }                     
    return accum;
}
void glut_bitmap_string(void *font, std::string const &s) {
    for (unsigned i = 0; i < s.size(); ++i) {
        glutBitmapCharacter( font, s[i] );
    }                     
}
/*
void glut_bitmap_string_center(void *font, std::string const &s) {
    GLint rp[4];
    glGetIntegerv( GL_CURRENT_RASTER_POSITION, rp );
    glRasterPos2i( rp[0]-glut_bitmap_width(font, s)/2, rp[1] );

}
void glut_bitmap_string_border(void *font, std::string const &s) {
    GLint rp[4];
    glGetIntegerv( GL_CURRENT_RASTER_POSITION, rp );

    glRasterPos2i( rp[0]-1, rp[1]-1 );
    glut_bitmap_string( font, s );
    glRasterPos2i( rp[0]-1, rp[1]+1 );
    glut_bitmap_string( font, s );
    glRasterPos2i( rp[0]+1, rp[1]-1 );
    glut_bitmap_string( font, s );
    glRasterPos2i( rp[0]+1, rp[1]+1 );
    glut_bitmap_string( font, s );

    glRasterPos2i( rp[0], rp[1] );
}
*/

///////////////////////////////////////////////////////////////////////////////

void push_message() {
    ptime now = get_now();
    ptime lnow = second_clock::local_time();
    time_duration walltime = lnow.time_of_day();
    messagetimes.push_back( now );
    messagequeue.push_back( to_simple_string(walltime) );
    messagequeue.back() = "(" + messagequeue.back() + ") " + message;
    std::string::size_type p = std::string::npos-1;
    while ( glut_bitmap_width( GLUT_BITMAP_HELVETICA_12, messagequeue.back() ) > viewport_width / 3 ) {
        std::string::size_type oldp = p;
        p = messagequeue.back().rfind(' ', oldp);
        if ( p == std::string::npos || p == 0) {
            if (oldp == std::string::npos - 1) {
                break;
            } else { 
                p = oldp+1;
            }
        }
        std::string prefix = messagequeue.back().substr(0, p);
std::clog << "Trying split " << prefix << "\n";
        if ( p == oldp+1 || glut_bitmap_width(GLUT_BITMAP_HELVETICA_12, prefix ) < viewport_width/3 ) {
            std::string suffix = messagequeue.back().substr(p);
std::clog << "Accepted.  Suffix " << suffix << "\n";
            messagequeue.back() = prefix;
            messagetimes.push_back( now );
            messagequeue.push_back( suffix );
            p = std::string::npos;
        }
        --p;
    }
    while ( (int)messagequeue.size() * 12 > viewport_height/3 ) {
            messagetimes.pop_front();
            messagequeue.pop_front();
    }
}

///////////////////////////////////////////////////////////////////////////////

vector2f unproject(int x, int y) {
    GLfloat depth;
    glReadPixels( x, y, 1, 1,
                  GL_DEPTH_COMPONENT, GL_FLOAT,
                  &depth );

    GLdouble winx = x, winy = y, winz = depth;
    GLdouble objx, objy, objz;
    gluUnProject( winx, winy, winz,
                  modelview_matrix, projection_matrix, viewport_values,
                  &objx, &objy, &objz );
//std::clog << "Z coord: " << objz << "\n";
    RET_VEC_2F( objx, objy );
}

vector2f project(vector2f const &p) {
    GLdouble winx, winy, winz;
    gluProject( p.x(), p.y(), 0,
                modelview_matrix, projection_matrix, viewport_values,
                &winx, &winy, &winz );
//std::clog << "Screen Z: " << winz << "\n";
    RET_VEC_2F( winx, winy );                
}

void gl_draw() {

    float const yrot = camera_angle->now().x();
    float const user_zoom = camera_zoom->now().x();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glTranslatef( 0.0f, 0.0f, -1.0f + user_zoom );

    glTranslatef(0.0f,0.0f, cos(yrot*4 * M_PI/180 )/8 );
    
    glRotatef( -45, 1, 0, 0 );

    glTranslatef(0, 0.25f, 0); // focus on our side
//    glTranslatef(0, 0, 0.1f); // lift it a bit

    glRotatef(yrot,0.0f,0.0f,1.0f);        // Rotate On The Y Axis

    glTranslatef(-0.5f, -0.5f, 0); // center the board

    // For later use in project/unproject
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview_matrix );

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    gl_color( white_color );

    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  0.0f,  0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  0.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

/*
    gl_color( white_color );
    for (unsigned i = 0; i < 41; ++i) {
        vector2f p = property_coordinates(i);
        if ( i == 40 ) {
            p = jail_coordinates();
        }
        glPushMatrix();
            glTranslatef( p.x(), p.y(), 0 );
//            glutSolidTeapot( 10./1024 );
            glutSolidSphere( 10./1024, 8, 8 );
        glPopMatrix();
    }
*/

    for (unsigned i = 0; i < 40; ++i) {
        vector2f pp = property_coordinates(i);

        // Draw Ownership Markers
        if ( players.find( owners[i] ) != players.end() ) {
            vector3ub color = players[owners[i]].color;
            if ( mortgaged[i] ) color /= 2;
            gl_color( color );
            glPushMatrix();
                glScalef(1,1,0.01);
                glTranslatef( pp.x(), pp.y(), -45./1024 );
    //            glTranslatef( i->second.end().x(), i->second.end().y(), 0 );
    //            glutSolidTeapot( 10./1024 );
                glutSolidSphere( 45./1024, 16, 4 );
            glPopMatrix();
        }
        
        // Draw Houses/Hotels
        if ( rent_levels[i] != 0 ) {
			if(rent_levels[i] < 5){			//Houses
				glColor3f(0.0f, 1.0f, 0.0f);
				glPushMatrix();
				if(i < 10){
					glTranslatef(pp.x() - 30./1024, pp.y()+ 100./1024, 0);
					for(int j=0; j<rent_levels[i]; j++){
						glPushMatrix();
							glTranslatef(j*(20./1024), 0, 0);
							glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
							glScalef(0.00015f, 0.00015f,0.00015f);
							models[7].Draw();
						glPopMatrix();
					}

				}
				else if(i < 20){
					glTranslatef(pp.x() + 100./1024, pp.y()+ 30./1024, 0);
					for(int j=0; j<rent_levels[i]; j++){
						glPushMatrix();
							glTranslatef(0, j*(-20./1024), 0);
							glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
							glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
							glScalef(0.00015f, 0.00015f,0.00015f);
							models[7].Draw();
						glPopMatrix();
					}

				}
				else if(i < 30){
					glTranslatef(pp.x() + 30./1024, pp.y()- 100./1024, 0);
					for(int j=0; j<rent_levels[i]; j++){
						glPushMatrix();
							glTranslatef(j*(-20./1024), 0, 0);
							glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
							glScalef(0.00015f, 0.00015f,0.00015f);
							models[7].Draw();
						glPopMatrix();
					}
				}
				else{
					glTranslatef(pp.x() - 100./1024, pp.y()- 30./1024, 0);
					for(int j=0; j<rent_levels[i]; j++){
						glPushMatrix();
							glTranslatef(0, j*(20./1024), 0);
							glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
							glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
							glScalef(0.00015f, 0.00015f,0.00015f);
							models[7].Draw();
						glPopMatrix();
					}

				}
				glPopMatrix();
				glColor3f(1.0f,1.0f,1.0f);
			}
			else{							//Hotel
				glColor3f(1.0f, 0.0f, 0.0f);
				glPushMatrix();
				if(i < 10){
					glTranslatef(pp.x(), pp.y()+ 100./1024, 0);
					glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
					glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					glScalef(0.00015f, 0.00015f,0.00015f);
					models[8].Draw();
					
				}
				else if(i < 20){
					glTranslatef(pp.x()+ 100./1024, pp.y(), 0);
					glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
					glScalef(0.00015f, 0.00015f,0.00015f);
					models[8].Draw();
				}
				else if(i < 30){
					glTranslatef(pp.x(), pp.y()- 100./1024, 0);
					glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
					glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					glScalef(0.00015f, 0.00015f,0.00015f);
					models[8].Draw();
				}
				else{
					glTranslatef(pp.x()- 100./1024, pp.y(), 0);
					glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
					glScalef(0.00015f, 0.00015f,0.00015f);
					models[8].Draw();
				}
				glPopMatrix();
				glColor3f(1.0f,1.0f,1.0f);
			}
			
			/*
			gl_color(white_color);
            glRasterPos2fv( pp.v() );
            glut_bitmap_string( GLUT_BITMAP_HELVETICA_18,
                                "Rent Level " + stringify(rent_levels[i]) );*/
        }
    }

    // Draw Player Piece
    for (players_type::iterator i = players.begin(); i != players.end(); ++i) {
        gl_color( i->second.color );
        glPushMatrix();
            glTranslatef( i->second.pos().x(), i->second.pos().y(), 0 );
//            glTranslatef( i->second.end().x(), i->second.end().y(), 0 );
//            glutSolidTeapot( 10./1024 );

			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			
			float angle = 0.0f;
			if(i->second.last_square <= 11){
				angle = 90.0f;
			}
			else if(i->second.last_square > 10 && i->second.last_square < 21){
				angle = 180.0f;
			}
			else if(i->second.last_square > 20 && i->second.last_square < 31){
				angle = 270.0f;
			}
			else{
				angle = 0.0f;
			}
			
			if(i->second.model_index == 2){
				angle -= 90.0f;
			}
			
			glRotatef(angle, 0.0f, 1.0f, 0.0f);
			glScalef(0.0002f, 0.0002f,0.0002f);
			models[i->second.model_index].Draw();
			//glutSolidSphere( 10./1024, 8, 8 );
        glPopMatrix();
        gl_color(white_color);
    }

    /*
    // Draw Camera Target
    if (players.find(active_player) != players.end()) {
        vector2f p = ( players[active_player].pos()
                     + players[active_player].end() )/2;
        p /= 2;
        p.x() += 0.5/2;
        p.y() += 0.5/2;
        gl_color( black_color );
        glPushMatrix();
            glTranslatef( p.x(), p.y(), 0 );
//            glTranslatef( i->second.end().x(), i->second.end().y(), 0 );
//            glutSolidTeapot( 10./1024 );
            glutSolidSphere( 10./1024, 8, 8 );
        glPopMatrix();
    }   
    */

    glPushMatrix();
    for (unsigned i = 0; i < 16; ++i) {
        glTranslatef( 0, 0, 1/1024. );
        gl_color( chance_color );
        glBegin(GL_QUADS);
            glVertex2fv( chance_tl.v() );
            glVertex2fv( chance_tr.v() );
            glVertex2fv( chance_br.v() );
            glVertex2fv( chance_bl.v() );
        glEnd();
        gl_color( chest_color );
        glBegin(GL_QUADS);
            glVertex2fv( chest_tl.v() );
            glVertex2fv( chest_tr.v() );
            glVertex2fv( chest_br.v() );
            glVertex2fv( chest_bl.v() );
        glEnd();
    }
    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
    gl_go_ortho();
    {
        glLoadIdentity();

        // Player Name Labels
        for (players_type::iterator i = players.begin(); i != players.end(); ++i) {
            vector2f const tl = {{1, viewport_height-2}};
            vector2f const br = {{viewport_width-2, 1}};

            vector2f sp = project( i->second.pos() );
            sp.y() += 18;
            if ( !inside( sp, tl, br ) ) continue;

            {
            vector2f xp = sp;
            unsigned w = glut_bitmap_width( GLUT_BITMAP_HELVETICA_18,
                                            i->first );
            unsigned h = 18;                                           
            xp.x() -= w/2;

            glEnable(GL_BLEND);
            gl_color_alpha( bg_color, 64*3-1 );
            glRectf( xp.x()-3, xp.y()+h,
                     xp.x()+w+3, xp.y()-6 );
            glDisable(GL_BLEND);

            gl_color( i->second.color );
            glRasterPos2fv( xp.v() );
            glut_bitmap_string( GLUT_BITMAP_HELVETICA_18,
                                i->first );

            sp.y() += 18;
            }
            
            if (i->second.moneydelta) {
                sp.y() += 24;
                if ( !inside( sp, tl, br ) ) continue;

                std::string moneydeltastring = money_stringify(i->second.moneydelta);

                vector2f xp = sp;
                unsigned w = glut_bitmap_width( GLUT_BITMAP_TIMES_ROMAN_24,
                                                moneydeltastring );
                unsigned h = 24;                                           
                xp.x() -= w/2;

                glEnable(GL_BLEND);
                gl_color_alpha( bg_color, 64*3-1 );
                glRectf( xp.x()-4, xp.y()+h,
                         xp.x()+w+4, xp.y()-8 );
                glDisable(GL_BLEND);

                gl_color( money_color(i->second.moneydelta) );
                glRasterPos2fv( xp.v() );
                glut_bitmap_string( GLUT_BITMAP_TIMES_ROMAN_24,
                                    moneydeltastring );
                sp.y() += 24;
            }
        }

        
        // Message Console
        glEnable(GL_BLEND);
        gl_color_alpha( bg_color, 64*3-1 );
        glRecti( 0, viewport_height,
                 viewport_width/3 + 4, viewport_height-16*messagequeue.size()-4*2 );    
        glDisable(GL_BLEND);
        gl_color( fg_color );
        for (unsigned i = 0; i < messagequeue.size(); ++i) {
            glRasterPos2i( 4, viewport_height-16*(i+1) );    
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_12, messagequeue[i]);
        }
        
        //Player List
        std::string playerName;
        int tempCounter = 0;
        
        glEnable(GL_BLEND);
        gl_color_alpha( bg_color, 64*3-1 );
        glRecti( viewport_width*2/3, viewport_height,
                 viewport_width, viewport_height - 20*players.size()-1 );
        glDisable(GL_BLEND);

        for (players_type::iterator i = players.begin(); i != players.end(); ++i) {
            tempCounter++;
            
            gl_color( white_color );
            if(i->first == active_player){
                glBegin(GL_LINE_STRIP);
                    glVertex2i( viewport_width*2/3, viewport_height - (tempCounter-1)*20 - 1 );
                    glVertex2i( viewport_width-1, viewport_height - (tempCounter-1)*20 - 1 );
                    glVertex2i( viewport_width-1, viewport_height - tempCounter*20 - 1 );
                    glVertex2i( viewport_width*2/3, viewport_height - tempCounter*20 - 1 );
                    glVertex2i( viewport_width*2/3, viewport_height - (tempCounter-1)*20 - 1 );
                glEnd();
            }
        
            gl_color( i->second.color );
            glRasterPos2i(viewport_width*2/3 + 5, viewport_height - tempCounter*20 + 5);
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_12, i->first);

            gl_color( money_color(i->second.balance) );
            glRasterPos2i(viewport_width*5/6, viewport_height - tempCounter*20 + 5);
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_12, money_stringify(i->second.balance));
            
        }

        glEnable(GL_TEXTURE_2D);
        gl_color( white_color );
        glPushMatrix();
        {
            int delta = 16;
            glTranslatef( viewport_width-delta, delta, 0 );
            if ( can["communityChest gojf"] ) {
                glBindTexture(GL_TEXTURE_2D, chest_cards[15]);
                glBegin(GL_QUADS);
                    glNormal3f( 0.0f, 0.0f, 1.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex3i(   0,   0, 0);
                    glTexCoord2f(0.0f, 1.0f); glVertex3i(-256,   0, 0);
                    glTexCoord2f(0.0f, 0.0f); glVertex3i(-256, 128, 0);
                    glTexCoord2f(1.0f, 0.0f); glVertex3i(   0, 128, 0);
                glEnd();
                glTranslatef( 0, 128+delta, 0 );
            }
            if ( can["chance gojf"] ) {
                glBindTexture(GL_TEXTURE_2D, chance_cards[15]);
                glBegin(GL_QUADS);
                    glNormal3f( 0.0f, 0.0f, 1.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex3i(   0,   0, 0);
                    glTexCoord2f(0.0f, 1.0f); glVertex3i(-256,   0, 0);
                    glTexCoord2f(0.0f, 0.0f); glVertex3i(-256, 128, 0);
                    glTexCoord2f(1.0f, 0.0f); glVertex3i(   0, 128, 0);
                glEnd();
                glTranslatef( 0, 128+delta, 0 );
            }
        }
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);

        if ( can["see deed"] || active_deed != -1 ) {
        
            glPushMatrix();

                glTranslatef( viewport_width/2, viewport_height/2, 0 );
                glTranslatef( deed_center.now().x(), deed_center.now().y(), 0 );

                glEnable(GL_TEXTURE_2D);
                gl_color( white_color );

                glBindTexture(GL_TEXTURE_2D, deed_fronts[active_deed]);
                glBegin(GL_QUADS);
                    glNormal3f( 0.0f, 0.0f, 1.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(  deed_size.now().x()/2, -deed_size.now().y()/2,  0.0f);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f( -deed_size.now().x()/2, -deed_size.now().y()/2,  0.0f);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f( -deed_size.now().x()/2,  deed_size.now().y()/2,  0.0f);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(  deed_size.now().x()/2,  deed_size.now().y()/2,  0.0f);
                glEnd();

                glDisable(GL_TEXTURE_2D);

            glPopMatrix();

        }

        if ( can["see card"] || active_card != -1 ) {
        
            glPushMatrix();

                glTranslatef( viewport_width/2, viewport_height/2, 0 );

                glEnable(GL_TEXTURE_2D);
                gl_color( white_color );

                glBindTexture(GL_TEXTURE_2D, card_textures[active_card]);
                glBegin(GL_QUADS);
                    glNormal3f( 0.0f, 0.0f, 1.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex2fv( card_br.now().v() );
                    glTexCoord2f(0.0f, 1.0f); glVertex2fv( card_bl.now().v() );
                    glTexCoord2f(0.0f, 0.0f); glVertex2fv( card_tl.now().v() );
                    glTexCoord2f(1.0f, 0.0f); glVertex2fv( card_tr.now().v() );
                glEnd();

                glDisable(GL_TEXTURE_2D);

            glPopMatrix();

        }

        if ( can["see dice"] ) {

            glPushMatrix();

                glTranslatef( viewport_width/2, viewport_height/2, 0 );

                glEnable(GL_TEXTURE_2D);
                gl_color( white_color );

                glBindTexture(GL_TEXTURE_2D, textures[dice1]);
                glBegin(GL_QUADS);
                    glNormal3f( 0.0f, 0.0f, 1.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex3i(-192, -64, 0);
                    glTexCoord2f(0.0f, 1.0f); glVertex3i( -64, -64, 0);
                    glTexCoord2f(0.0f, 0.0f); glVertex3i( -64,  64, 0);
                    glTexCoord2f(1.0f, 0.0f); glVertex3i(-192,  64, 0);
                glEnd();

                glBindTexture(GL_TEXTURE_2D, textures[dice2]);
                glBegin(GL_QUADS);
                    glNormal3f( 0.0f, 0.0f, 1.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex3i( 64, -64,  0);
                    glTexCoord2f(0.0f, 1.0f); glVertex3i(192, -64,  0);
                    glTexCoord2f(0.0f, 0.0f); glVertex3i(192,  64,  0);
                    glTexCoord2f(1.0f, 0.0f); glVertex3i( 64,  64,  0);
                glEnd();
                glDisable(GL_TEXTURE_2D);

            glPopMatrix();

        }
        
        if ( can["type"] ) {
            gl_color( bg_color );
            glRecti(   viewport_width/4 - 10, 42,
                     3*viewport_width/4 + 10, 10 );
            
            gl_color( fg_color );
            glRasterPos2i( viewport_width/4, 20 );    
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_18, userim);
        }

        /*
            gl_color( bg_color );
            glRasterPos2i( viewport_width/2-1, viewport_height/2-1 );    
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_18, userim);
            glRasterPos2i( viewport_width/2-1, viewport_height/2+1 );    
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_18, userim);
            glRasterPos2i( viewport_width/2+1, viewport_height/2+1 );    
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_18, userim);
            glRasterPos2i( viewport_width/2+1, viewport_height/2-1 );    
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_18, userim);
            gl_color( fg_color );
            glRasterPos2i( viewport_width/2, viewport_height/2 );    
            glut_bitmap_string(GLUT_BITMAP_HELVETICA_18, userim);
        */
    }
    gl_un_ortho();


    glEnable(GL_DEPTH_TEST);
    glutSwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////

int mouse_to_property(int x, int y) {
std::clog << "Mouse at " << x << ", " << y << "\n";
    vector2f c = unproject( x, y );
std::clog << "AKA " << c.x() << ", " << c.y() << "\n";
    int p = property_at_coordinates(c);
std::clog << "Property " << p << "\n" << std::flush;
    return p;
}

void key_press(unsigned char key, int x, int y) {
    /* avoid thrashing this procedure */
//    usleep(100);

    can["see dice"] = false;
    
//std::clog << "typed " << key << " (" << (int)key << ")\n";    

    if ( can["type"] && ~x && ~y ) {
        if ( std::isalnum(key) || ispunct(key) || key == ' ' ) {
            userim += key;
            if ( glut_bitmap_width( GLUT_BITMAP_HELVETICA_18, userim)
               > viewport_width/2 ) {
               
                key = 8;
               
            } else {
                return;
            }
        }
        if ( key == BACKSPACE ) {
            if ( !userim.empty() ) {
                userim.erase( userim.end() - 1 );
            }
            return;
        }
        if ( key == TYPE ) {
            if ( !userim.empty() ) {
                if (userim[0] == '.' || userim[0] == '!') {
                    // For debugging
                    channel_put(userim);
                } else {
                    message = "<> " + userim;
                    push_message();
                    channel_put(".tell * <" + alias + "> " + userim);
                }
            }
            can["type"] = false;
            userim = "";
            return;
        }
    } else {
        if ( key == TYPE ) {
            can["type"] = true;
            return;
        }
    }

    // window managers give coordinates upside-down
    y = viewport_values[3] - y;
    x = x;

    switch (key) {    
      case QUIT:
        glutDestroyWindow(window); 
        std::exit(0);                       
        break;

      case CONCEDE:
        channel_put("!quit");
        message = "Conceding...";
        push_message();
        break;

      case AUTOPLAY:
        can["auto"] ^= true;
        message = "Auto-play turned ";
        message += can["auto"] ? "on." : "off.";
        push_message();
        break;

      case BUY:
        if (!can["buy"]) {
            message = "Warning: Can't buy now.";
            push_message();
        }
        channel_put("!buy");
        message = "Buying...";
        push_message();
        break;

      case UPGRADE:
        if (0 <= active_deed && active_deed < 40) {
            if (owners[active_deed] != alias) {
                message = "Warning: You don't own this property.";
                push_message();
            }
            channel_put("!upgrade " + stringify(active_deed));
            message = "Upgrading...";
            push_message();
        } else {
            message = "Error: Open a deed to upgrade the property.";
            push_message();
        }
        break;

      case DOWNGRADE:
        if (0 <= active_deed && active_deed < 40) {
            if (owners[active_deed] != alias) {
                message = "Warning: You don't own this property.";
                push_message();
            }
            channel_put("!downgrade " + stringify(active_deed));
            message = "Downgrading...";
            push_message();
        } else {
            message = "Error: Open a deed to downgrade the property.";
            push_message();
        }
        break;

      case ENDTURN:
        if (!can["end"]) {
            message = "Warning: Can't end turn yet.";
            push_message();
        }
        channel_put("!endTurn");
        message = "Ending turn...";
        push_message();
        break;

      case JOIN:
        if (!can["join"]) {
            message = "Warning: Can't join game now.";
            push_message();
        }
        channel_put("!join");
        message = "Joining game...";
        push_message();
        break;

      case MORTGAGE:
        if (0 <= active_deed && active_deed < 40) {
            if (owners[active_deed] != alias) {
                message = "Warning: You don't own this property.";
                push_message();
            }
            if (mortgaged[active_deed]) {
                message = "Warning: This property is already mortgaged.";
                push_message();
            }
            channel_put("!mortgage " + stringify(active_deed));
            message = "Mortgaging...";
            push_message();
        } else {
            message = "Error: Open a deed to mortgage the property.";
            push_message();
        }
        break;

      case UNMORTGAGE:
        if (0 <= active_deed && active_deed < 40) {
            if (owners[active_deed] != alias) {
                message = "Warning: You don't own this property.";
                push_message();
            }
            if (!mortgaged[active_deed]) {
                message = "Warning: This property is not mortgaged.";
                push_message();
            }
            channel_put("!unmortgage " + stringify(active_deed));
            message = "Unmortgaging...";
            push_message();
        } else {
            message = "Error: Open a deed to unmortgage the property.";
            push_message();
        }
        break;


      case ROLL:
        if (!can["roll"]) {
            message = "Warning: Can't roll now.";
            push_message();
        }
        channel_put("!roll");
        message = "Rolling...";
        push_message();
        can["rolling"] = true;
        can["see dice"] = true;
        diceupdate = get_now();
        break;
      
      case START:
        if (!can["start"]) {
            message = "Warning: Can't start game now.";
            push_message();
        }
        channel_put("!start");
        message = "Starting game...";
        push_message();
        break;
      
      case FLATTAX:
        if (!can["tax"]) {
            message = "Warning: Tax not currently due.";
            push_message();
        }
        channel_put("!taxFlat");
        message = "Paying flat tax...";
        push_message();
        break;
      
      case PERCENTTAX:
        if (!can["tax"]) {
            message = "Warning: Tax not currently due.";
            push_message();
        }
        channel_put("!taxPercent");
        message = "Paying percentage tax...";
        push_message();
        break;

      case GOJ:
        if (players[alias].last_square != ~0) {
            message = "Warning: Not in jail.";
            push_message();
        }
        channel_put("!payJail");
        message = "Paying to get out of jail...";
        push_message();
        break;
      
      default:
        break;
    }    
}

void special_press(int key, int x, int y) {

    // window managers give coordinates upside-down
    y = viewport_values[3] - y;
    x = x;
    
    switch (key) {    
      case GLUT_KEY_PAGE_UP:
        move_camera_zoom( camera_zoom->target().x() + 0.05 );
        break;
        
      case GLUT_KEY_PAGE_DOWN:
        move_camera_zoom( camera_zoom->target().x() - 0.05 );
        break;
        
      default:
        break;
    }    
    
}

void mouse_action(int button, int state, int x, int y) {

//std::clog << "Button " << button << " callback\n";
    
    if ( y != ~0 ) {
        // window managers give coordinates upside-down
        y = viewport_values[3] - y;
        x = x;
    }

    switch (button) {
      case GLUT_LEFT_BUTTON:
      case GLUT_MIDDLE_BUTTON:
      case GLUT_RIGHT_BUTTON:
        if ( state == GLUT_DOWN ) {

            if (can["see card"]) {

                card_tl.start_pos = card_tl.now();
                card_tr.start_pos = card_tr.now();
                card_bl.start_pos = card_bl.now();
                card_br.start_pos = card_br.now();

                vector2f z = {{0,0}};
            
                card_tl.end_pos = z;
                card_tr.end_pos = z;
                card_bl.end_pos = z;
                card_br.end_pos = z;

                ptime now = get_now();
                card_tl.start_time = now;
                card_tr.start_time = now;
                card_bl.start_time = now;
                card_br.start_time = now;

                ptime e = now + seconds(1);
                card_tl.end_time = e;
                card_tr.end_time = e;
                card_bl.end_time = e;
                card_br.end_time = e;

                can["see card"] = false;
            }

            if (can["see deed"]) {
                
                deed_center.start_pos = deed_center.now();
                deed_center.end_pos = project( property_coordinates(active_deed) );                
                deed_center.end_pos.x() -= viewport_width/2;
                deed_center.end_pos.y() -= viewport_height/2;
                deed_center.start_time = get_now();
                deed_center.end_time = deed_center.start_time + seconds(1);
                
                deed_size.start_pos = deed_size.now();
                deed_size.end_pos.x() = 0;
                deed_size.end_pos.y() = 0;
                deed_size.start_time = deed_center.start_time;
                deed_size.end_time = deed_center.end_time;
                
                can["see deed"] = false;
                
            } else if ( x != ~0 ) {
                int p = mouse_to_property(x, y);
                if (p == -1) break;
                if (!purchasable[p]) break;
                active_deed = p;
                
                can["see deed"] = true;
                
                deed_center.start_pos.x() = x - viewport_width/2;
                deed_center.start_pos.y() = y - viewport_height/2;
                deed_center.end_pos.x() = 0;
                deed_center.end_pos.y() = 0;
                deed_center.start_time = get_now();
                deed_center.end_time = deed_center.start_time + seconds(1);
                
                deed_size.start_pos = deed_center.end_pos;
                deed_size.end_pos.x() = 221*2;
                deed_size.end_pos.y() = 256*2;
                deed_size.start_time = deed_center.start_time;
                deed_size.end_time = deed_center.end_time;
            }
            
        } else {
            // screen button checks go here
        }
        break;
      case 3: // SCROLL UP
        if ( state == GLUT_DOWN) {
            special_press( GLUT_KEY_PAGE_UP, x, y );
        }
        break;
      case 4: // SCROLL DOWN
        if ( state == GLUT_DOWN) {
            special_press( GLUT_KEY_PAGE_DOWN, x, y );
        }
        break;
    }

}

///////////////////////////////////////////////////////////////////////////////

void gl_idle() {

    lines->sleep(1);

    ptime const &now = get_now();
    
    if (can["rolling"] && now >= diceupdate) {
        dice1 = rand()%6 + 1;
        dice2 = rand()%6 + 1;
        diceupdate += millisec(100);
    }
    
    while ( (int)messagequeue.size() * 12 > viewport_height/6 &&
            now > messagetimes.front() + seconds(30) ) {
        messagetimes.pop_front();
        messagequeue.pop_front();
    }
    
    if ( active_deed != -1 ) {
        if ( !can["see deed"] ) {
            if ( deed_center.done(now) ) {
                active_deed = -1;
            }
            deed_center.end_pos = project( property_coordinates(active_deed) );
            deed_center.end_pos.x() -= viewport_width/2;
            deed_center.end_pos.y() -= viewport_height/2;
        }
        deed_center.update(now);
        deed_size.update(now);
    }        

    if ( active_card != -1 ) {
        if ( !can["see card"] ) {
            if ( card_tl.done(now) ) {
                active_card = -1;
            }
        }
        card_tl.update(now);
        card_tr.update(now);
        card_bl.update(now);
        card_br.update(now);
    }        

    for (;;) {
        std::pair<std::string, std::string>
        line = channel_get();
        if (line.first.empty()) break;
        
        while (!line.second.empty() && *line.second.rbegin() == ' ') {
            line.second.erase( line.second.end()-1 );
        }

        if (line.second.find(".turn ") == 0) {

            can["see dice"] = false;

        }
        
        if (line.second.find(".rollagain ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".rollagain" );
            if ( ss >> s && s == alias ) {
                can["roll"] = true;
                can["end"] = false;
            
                if ( can["auto"] && can["roll"] ) {
                    key_press( ROLL, ~0, ~0 );
                }
            }
            can["see dice"] = false;
        }
        
        if (line.second.find(".turn ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".turn" );
            if ( ss >> s ) active_player = s;
            can["roll"] = (active_player == alias);
            if (active_player == alias) {
                message = "Your turn.";
                push_message();
            } else {
                message = s + " is up.";
                push_message();
            }
            can["end"] = false;
            can["tax"] = false;

            if ( can["auto"] && can["roll"] ) {
                key_press( ROLL, ~0, ~0 );
            }

            if ( can["see card"] ) {
                mouse_action( GLUT_MIDDLE_BUTTON, GLUT_DOWN, ~0, ~0 );
            }
        }

        if (line.second.find(".join ") == 0) {
            std::istringstream ss(line.second);
            std::string s, t;
            ss >> s;
            assert( s == ".join" );
            if (ss >> s) {
                if (s == alias) {
                    can["join"] = false;
                    can["start"] = true;
                    message = "Joined; Waiting for start.";
                    push_message();
                } else {
                    message = s + " joined the game.";
                    push_message();
                }
                add_player(s);
            }
        }

        if (line.second.find(".players ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".players" );
            can["start"] = false;
            message = "Game in Progress";
            push_message();
        }

        if (line.second.find(".players ") == 0) {
            std::istringstream ss(line.second);
            std::string s, t;
            ss >> s;
            assert( s == ".players" );
            while(true){
                t = s;
                ss >> s;
                if(s.compare(t) == 0) 
                    break;
                add_player(s);
            }
        }

        if (line.second.find(".fail ") == 0) {
            std::istringstream ss(line.second);
            std::string s, t;
            ss >> s;
            assert( s == ".fail" );
            if (ss >> s) {
                if (s == alias || s == "*") {
                    std::getline( ss >> std::ws, message );
                    message = "Error: " + message;
                    push_message();
                    can["see dice"] = false;
                }
            }
        }

        if (line.second.find(".tell ") == 0) {
            std::istringstream ss(line.second);
            std::string s, t;
            ss >> s;
            assert( s == ".tell" );
            if (ss >> s) {
                if (s == alias || s == "*") {
                    std::getline( ss >> std::ws, message );
                    push_message();
                }
            }
        }

        if (line.second.find(".roll ") == 0) {
            std::istringstream ss(line.second);
            std::string s, t;
            ss >> s;
            assert( s == ".roll" );
            ss >> s;
            ss >> dice1;
            ss >> dice2;
            //std::cout << stringify(dice1) + " " + stringify(dice2);
            can["see dice"] = true;
            can["rolling"] = false;
            
            if ( s == alias ) {
                can["roll"] = (dice1 == dice2);
                can["end"] = !can["roll"];
                message = "Rolled";
            } else {
                message = s + " rolled";
            }
            message += " a " + stringify(dice1) + " and a " + stringify(dice2);
            if (dice1 == dice2) message += " -- Doubles!";
            else message += ".";
            push_message();

            if ( can["auto"] && s == alias && players[s].last_square == ~0 ) {
                can["end"] = true;
                key_press( ENDTURN, ~0, ~0 );
            }
            
            if ( can["see card"] ) {
                mouse_action( GLUT_MIDDLE_BUTTON, GLUT_DOWN, ~0, ~0 );
            }
        }

        if (line.second.find(".move ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".move" );
            if (ss >> s) {
                int square;
                if ( ss >> square ) {
                    std::string t;
                    ss >> t;
                    if ( t != "backward" && t != "teleport" ) {
                        t = "forward";
                    }

                    if (players.find(s) != players.end()) {
        std::cout << "Adding movement: " << s << " to " << square << std::endl;                
                    
                        switch ( t[0] ) {
                          case 'f':
                            players[s].add_forward_movement(square);
                            break;
                          case 'b':
                            players[s].add_backward_movement(square);
                            break;
                          case 't':
                            players[s].add_teleport_movement(square);
                            break;
                        }
                    }
                    
                    can["buy"] = false;
                    if ( s == alias ) {
                        if ( !purchasable[square] ) {
                            message = "This property is unavailable for purchase.";
                        } else if ( owners[square] == alias ) {
                            message = "You own this property.";
                        } else if ( owners[square].empty() ) {
                            message = "You landed on an unowned property.";
                            can["buy"] = true;
                        } else {
                            message = "You landed on a property owned by " + owners[square] + ".";
                        }
                    } else {
                        if ( !purchasable[square] ) {
                            message = s + " landed on an property unavailable for purchase.";
                        } else if ( owners[square].empty() ) {
                            message = s + " landed on an unowned property.";
                        } else if (s == owners[square]) {
                            message = s + " already owns that property.";
                        } else {
                            message = s + " landed on a property owned by " + owners[square] + ".";
                        }
                    }
                    push_message();
               } else {
                    ss.clear();
                    std::string jail;
                    if ( (ss >> jail) && jail == "jail" ) {
                        std::string t;
                        ss >> t;

                        if (players.find(s) != players.end()) {
        std::cout << "Adding movement: " << s << " to jail" << std::endl;                
                            players[s].add_teleport_movement(~0);
                        }
                    }
                }
            }
            if ( s == alias && players[s].last_square == ~0 ) {
                can["roll"] = false;
                can["end"] = true;
            }
            if ( can["auto"] && can["buy"] && players[alias].balance >= 500 ) {
                key_press( BUY, ~0, ~0 );
            }
            if ( can["auto"] && !can["roll"] && can["end"]) {
                key_press( ENDTURN, ~0, ~0 );
            }
        }

        if (line.second.find(".balance ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".balance" );
            ss >> s;
            int dollars;
            if ( ss >> dollars ) {
                if (players.find(s) != players.end()) {
                    int delta = dollars - players[s].balance;
                    players[s].moneydelta = delta;
                    players[s].moneydelta_time = now;
                    players[s].balance = dollars;
                    if ( delta ) {
                        std::string action = delta > 0 ? "brought in" : "paid";
                        if (delta < 0) delta = -delta;
                        if (s == alias) {
                            message = "You " + action + " " + money_stringify(delta) + ".";
                        } else {
                            message = s + " " + action + " " + money_stringify(delta) + ".";
                        }
                        push_message();
                    }
                }
            }
        }

        if (line.second.find(".mortgage ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".mortgage" );
            ss >> s;
            int property;
            if ( ss >> property ) {
                mortgaged[property] = true;
                if ( s == alias ) {
                    message = "Property mortgaged successfully.";
                } else {
                    message = s + " mortgaged a property.";
                }
                push_message();
           }
        }

        if (line.second.find(".unmortgage ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".unmortgage" );
            ss >> s;
            int property;
            if ( ss >> property ) {
                mortgaged[property] = false;
                if ( s == alias ) {
                    message = "Property unmortgaged successfully.";
                } else {
                    message = s + " unmortgaged a property.";
                }
                push_message();
            }
        }

        if (line.second.find(".offer ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".offer" );
            ss >> s;
            int location, dollars;
            if ( ss >> location >> dollars ) {
                if (s == alias) {
                    can["buy"] = true;
                    message = "You can buy this property for " + money_stringify(dollars) + ".";
                    push_message();
                }
            }
        }

        if (line.second.find(".acquired ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".acquired" );
            ss >> s;
            int location;
            if ( ss >> location ) {
                owners[location] = s;
                if (s == alias) {
                    message = "Property bought.";
                } else {
                    message = s + " bought the property.";
                }
                push_message();
            }
        }

        if (line.second.find(".incomeTaxes ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".incomeTaxes" );
            ss >> s;
            if ( ss ) {
                if ( s == alias ) {
                    message = "You must pay income tax.";
                    can["tax"] = true;
                } else {
                    message = s + " must pay income tax.";
                }
                push_message();
            }
            if ( can["auto"] && can["tax"] ) {
                key_press( players[alias].balance == 1500 ? PERCENTTAX : FLATTAX, ~0, ~0 );
            }
        }

        if (line.second.find(".paidTax ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".paidTax" );
            ss >> s;
            if ( ss ) {
                if ( s == alias ) {
                    message = "Taxes paid.";
                    can["tax"] = false;
                } else {
                    message = s + " paid tax.";
                }
                push_message();
            }
            if ( can["auto"] && can["roll"] ) {
                key_press( ROLL, ~0, ~0 );
            }
            if ( can["auto"] && can["end"] ) {
                key_press( ENDTURN, ~0, ~0 );
            }
        }

        if (line.second.find(".getCard ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".getCard" );
            ss >> s;
            std::string which;
            if ( ss >> which ) {
                if (s == alias) {
                    can[which + " gojf"] = true;
                    message = "You got a Get Out of Jail Free card.";
                } else {
                    message = s + " got a Get Out of Jail Free card.";
                }
                push_message();
            }
        }

        if (line.second.find(".loseCard ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".loseCard" );
            ss >> s;
            std::string which;
            if ( ss >> which ) {
                if (s == alias) {
                    can[which + " gojf"] = false;
                    message = "You used a Get Out of Jail Free card.";
                } else {
                    message = s + " used a Get Out of Jail Free card.";
                }
                push_message();
            }
        }
        
        if (line.second.find(".rentLevel ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".rentLevel" );
            int location, level;
            if ( ss >> location >> level ) {
                location %= 40;
                if (location < 0) location += 40;
                std::string what = rent_levels[location] < level ? "upgraded" : "downgraded";
                if (owners[location] == alias) {
                    message = "Property " + what + ".";
                } else {
                    message = owners[location] + " " + what + " a property.";
                }
                push_message();
                rent_levels[location] = level;
            }
        }

        if (line.second.find(".chance ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".chance" );
            int which;
            if ( ss >> s >> which ) {
                --which; // 1-based message to 0-based index
                if (s == alias) {
                    message = "You drew a chance card.";
                } else {
                    message = s + " drew a chance card.";
                }
                push_message();
                active_card = 0 + which;
                can["see dice"] = false;
                can["see card"] = true;
                
                vector2f mid = {{viewport_width/2, viewport_height/2}};

                card_tl.start_pos = project(chance_tl) - mid;
                card_tr.start_pos = project(chance_tr) - mid;
                card_bl.start_pos = project(chance_bl) - mid;
                card_br.start_pos = project(chance_br) - mid;
                
                vector2f dx = {{ 256, 0 }};
                vector2f dy = {{ 0, 128 }};
                
                card_tl.end_pos = - dx + dy;
                card_tr.end_pos = + dx + dy;
                card_bl.end_pos = - dx - dy;
                card_br.end_pos = + dx - dy;
                
                card_tl.start_time = now;
                card_tr.start_time = now;
                card_bl.start_time = now;
                card_br.start_time = now;

                ptime e = now + seconds(1);
                card_tl.end_time = e;
                card_tr.end_time = e;
                card_bl.end_time = e;
                card_br.end_time = e;
            }
        }
        
        if (line.second.find(".communityChest ") == 0) {
            std::istringstream ss(line.second);
            std::string s;
            ss >> s;
            assert( s == ".communityChest" );
            int which;
            if ( ss >> s >> which ) {
                --which; // 1-based message to 0-based index
                if (s == alias) {
                    message = "You drew a community chest card.";
                } else {
                    message = s + " drew a community chest card.";
                }
                push_message();
                active_card = 16 + which;
                can["see dice"] = false;
                can["see card"] = true;

                vector2f mid = {{viewport_width/2, viewport_height/2}};

                card_tl.start_pos = project(chest_tl) - mid;
                card_tr.start_pos = project(chest_tr) - mid;
                card_bl.start_pos = project(chest_bl) - mid;
                card_br.start_pos = project(chest_br) - mid;
                
                vector2f dx = {{ 256, 0 }};
                vector2f dy = {{ 0, 128 }};
                
                card_tl.end_pos = - dx + dy;
                card_tr.end_pos = + dx + dy;
                card_bl.end_pos = - dx - dy;
                card_br.end_pos = + dx - dy;
                
                card_tl.start_time = now;
                card_tr.start_time = now;
                card_bl.start_time = now;
                card_br.start_time = now;

                ptime e = now + seconds(1);
                card_tl.end_time = e;
                card_tr.end_time = e;
                card_bl.end_time = e;
                card_br.end_time = e;
            }
        }
        
    }
    
    update_camera(now);

    for (players_type::iterator i = players.begin(); i != players.end(); ++i) {
        i->second.update_movements(now);
//        std::cout << i->second.movements.front() << "\t";

        if ( now > i->second.moneydelta_time + seconds(5) ) {
            i->second.moneydelta = 0;
        }
    }

    if (players.find(active_player) == players.end()) {
        move_camera_angle( camera_angle->target().x() + 0.01 );
    } else {
//        vector2f p = players[active_player].pos();
//        vector2f p = players[active_player].end();
        vector2f p = ( players[active_player].pos()
                     + players[active_player].end() )/2;
        p.x() -= 0.5;
        p.y() -= 0.5;
        if (p.y()) {
            using std::atan2;
            move_camera_angle( 180 + 180/M_PI * atan2(p.x(),p.y()) );
        }
    }

    glutPostWindowRedisplay(window);
}

///////////////////////////////////////////////////////////////////////////////

int glut_init() {

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  

    glutInitWindowSize(viewport_width, viewport_height);

    window = glutCreateWindow( ("McGill Monopoly Client -- " + alias).c_str() );  

    glutDisplayFunc(gl_draw);  

//    glutFullScreen();

    glutIdleFunc(gl_idle);

    glutReshapeFunc(gl_resize);

    glutKeyboardFunc(key_press);

    glutSpecialFunc(special_press);

    glutMouseFunc(mouse_action);

    gl_init();
    initMenu();
    glutMainLoop();  

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    glutInit(&argc, argv);  

    init_can();
    init_camera();
    

    if (argc < 2) {
        std::cerr << "No argv[1]\n";
        return 1;
    }
    
    alias = argv[1];
    
    lines.reset( new irc_lines(alias) );
//    lines->poll();
//    lines->sleep(100);
    lines->put("JOIN " + channel);
    
//    channel_put( "!join" );

    message = "Welcome to McGill Monopoly presented by Team QuidQuid";
    push_message();

    return glut_init();
    
}

