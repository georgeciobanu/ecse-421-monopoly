#ifndef PROPERTY_COORDINATES
#define PROPERTY_COORDINATES

#include <cassert>

#include "vectors.hpp"

float const baseline_ = 20./1024;

float property_coordinate_(int pos) {
    assert( 0 < pos );
    assert( pos < 10 );
    
    return (1./1024)*(140 + (pos-0.5)*(744./9));
}

vector2f property_coordinates(int pos) {
    if (pos < 0) {
        return property_coordinates( 40 - ( (-pos) % 40 ) );
    }
    pos %= 40;
    
    if (pos == 0) RET_VEC_2F( 1 - baseline_, baseline_);
    if (pos < 10) RET_VEC_2F( 1 - property_coordinate_(pos), baseline_ );
    if (pos == 10) RET_VEC_2F( baseline_, baseline_);
    if (pos < 20) RET_VEC_2F( baseline_, property_coordinate_(pos-10) );
    if (pos == 20) RET_VEC_2F( baseline_, 1 - baseline_);
    if (pos < 30) RET_VEC_2F( property_coordinate_(pos-20), 1 - baseline_ );
    if (pos == 30) RET_VEC_2F( 1 - baseline_, 1 - baseline_);
    if (pos < 40) RET_VEC_2F( 1 - baseline_, 1 - property_coordinate_(pos-30) );
    assert( 0 );
}

vector2f jail_coordinates() {
    RET_VEC_2F( 90./1024, 90./1024 );
}

int property_at_coordinates(vector2f p) {

    // [0,10)
    if ( p.y() <= 140./1024 ) {
        if ( p.x() > 1 - 140./1024 ) {
            return 0;
        }
        if ( p.x() > 140./1024 ) {
            return int( 10 - (p.x()*1024 - 140)*9/744 );
        }
    }

    // [10,20)
    if ( p.x() <= 140./1024 ) {
        if ( p.y() < 140./1024 ) {
            return 10;
        }
        if ( p.y() < 1 - 140./1024 ) {
            return int( 11 + (p.y()*1024 - 140)*9/744 );
        }
    }
    
    // [20,30)
    if ( p.y() >= 1 - 140./1024 ) {
        if ( p.x() < 140./1024 ) {
            return 20;
        }
        if ( p.x() < 1 - 140./1024 ) {
            return int( 21 + (p.x()*1024 - 140)*9/744 );
        }
    }
    
    // [30,40)
    if ( p.x() >= 1 - 140./1024 ) {
        if ( p.y() > 1 - 140./1024 ) {
            return 30;
        }
        if ( p.y() > 140./1024 ) {
            return int( 40 - (p.y()*1024 - 140)*9/744 );
        }
    }

    return -1;
}

#endif

