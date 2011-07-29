//
//  Particle.cpp
//  Floor
//
//  Created by AJ Austinson on 7/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Particle.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"


Particle::Particle(){
    position = Vec3f::zero();
    velocity = Vec3f::zero();
    acceleration = Vec3f::zero();
    color = ColorA( 1.0f, 1.0f, 1.0f, 1.0f );
    scale = 1.0f;
    
    perlin = Perlin();
}

void Particle::update(){
    acceleration = perlin.dfBm( position ) * 0.02;
    acceleration += -position * 0.001;
    
    velocity += acceleration;
    position += velocity;
}

void Particle::draw(){
//    gl::drawBillboard( position, Vec2f(scale,scale), 0, bbUp, bbRight);
    gl::drawSphere(position,scale);
}

// --

void Particle::setBillboard( Vec3f eyePosition ){
//    bbUp = eyePosition.cross(position);
//    bbRight = bbUp * Vec3f(1.0f,0.0f,0.0f);
}