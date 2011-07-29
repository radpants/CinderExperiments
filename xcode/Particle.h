//
//  Particle.h
//  Floor
//
//  Created by AJ Austinson on 7/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Perlin.h"

using namespace ci;

class Particle {
public:
    Particle();
    
    void setup();
    void update();
    void draw();
    
    void setBillboard( Vec3f eyePosition );
    
    Vec3f position, velocity, acceleration;
    float scale;
    ColorA color;
    
    Perlin perlin;
    
    Vec3f bbUp, bbRight;
};