#include <iostream>
#include <Box2D/Box2D.h>
int main()
{
   // Creating a World
   b2World world(b2Vec2(0.0f, -9.8f));
   
   // Creating a Ground Box
   b2BodyDef groundBodyDef;
   groundBodyDef.position.Set(0.0f, -10.0f);
   b2Body* groundBody = world.CreateBody(&groundBodyDef);
   b2PolygonShape groundBox;
   groundBox.SetAsBox(50.0f, 10.0f);
   groundBody->CreateFixture(&groundBox, 0.0f);

   // Creating a Dynamic Body
   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position.Set(0.0f, 4.0f);
   b2Body* body = world.CreateBody(&bodyDef);
   b2PolygonShape dynamicBox;
   dynamicBox.SetAsBox(1.0f, 1.0f);
   b2FixtureDef fixtureDef;
   fixtureDef.shape = &dynamicBox;
   fixtureDef.density = 1.0f;
   fixtureDef.friction = 0.3f;
   body->CreateFixture(&fixtureDef);

   // Simulating the World
   float timeStep = 1.0f / 60.0f;
   int32 velocityIterations = 6;
   int32 positionIterations = 2;
   for (int32 i = 0; i < 60; ++i)
   {
       world.Step(timeStep, velocityIterations, positionIterations);
       b2Vec2 position = body->GetPosition();
       float angle = body->GetAngle();
       printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
   }
   return 0;
}