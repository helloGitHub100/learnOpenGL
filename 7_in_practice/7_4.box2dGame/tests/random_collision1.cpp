// MIT License

// Copyright (c) 2019 Erin Catto

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "test.h"

class RandomCollision1 : public Test
{
public:

	enum
	{
		e_count = 800
	};

	RandomCollision1()
	{
		m_world->SetGravity(b2Vec2(0.0f, 0.0f));
		const float k_restitution = 1.0f;// 碰撞恢复系数，1.0表示碰撞没有能量损失
		
		b2Body* ground = NULL;
		{
			b2BodyDef bd;
			ground = m_world->CreateBody(&bd);
		}

		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.allowSleep = false;
			bd.position.Set(0.0f, 20.0f);
			b2Body* body = m_world->CreateBody(&bd);

			b2PolygonShape shape;

			b2FixtureDef sd;
			sd.shape = &shape;
			sd.density = 5.0f;
			sd.friction = 0.0f;
			sd.restitution = k_restitution;

			shape.SetAsBox(1.0f, 20.0f, b2Vec2( 20.0f, 0.0f), 0.0);
			body->CreateFixture(&sd);
			shape.SetAsBox(1.0f, 20.0f, b2Vec2(-20.0f, 0.0f), 0.0);
			body->CreateFixture(&sd);
			shape.SetAsBox(20.0f, 1.0f, b2Vec2(0.0f, 20.0f), 0.0);
			body->CreateFixture(&sd);
			shape.SetAsBox(20.0f, 1.0f, b2Vec2(0.0f, -20.0f), 0.0);
			body->CreateFixture(&sd);

			b2RevoluteJointDef jd;
			jd.bodyA = ground;
			jd.bodyB = body;
			jd.localAnchorA.Set(0.0f, 20.0f);
			jd.localAnchorB.Set(0.0f, 0.0f);
			jd.referenceAngle = 0.0f;
			jd.motorSpeed = 0.05f * b2_pi;
			jd.maxMotorTorque = 1e8f;
			jd.enableMotor = true;
			m_joint = (b2RevoluteJoint*)m_world->CreateJoint(&jd);
		}

		m_count = 0;
	}

	void Step(Settings& settings) override
	{
		Test::Step(settings);

		if (m_count < e_count)
		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(0.0f, 20.0f);
			b2Body* body = m_world->CreateBody(&bd);
			b2Vec2 velocity(RandomFloat(-50.0f, 50.0f), RandomFloat(-50.0f, 50.0f));
			body->SetLinearVelocity(velocity);

			b2PolygonShape shape;
			shape.SetAsBox(0.125f, 0.125f);

			b2FixtureDef sd;
			sd.shape = &shape;
			sd.density = 1.0f;
			sd.friction = 0.0f;
			sd.restitution = 1.0f;
			
			body->CreateFixture(&sd);

			++m_count;
		}
	}

	static Test* Create()
	{
		return new RandomCollision1;
	}

	b2RevoluteJoint* m_joint;
	int32 m_count;
};

static int testIndex = RegisterTest("User Custom", "Random Collision1", RandomCollision1::Create);
