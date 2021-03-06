#ifndef PARTICLE_H
#define PARTICLE_H

#include "game/CGameObject.h"
#include "part_type.h"
#include "CVec.h"
#include "util/angle.h"
#include "timer_event.h"
#include <vector>
#include "netstream.h"

#ifndef DEDICATED_ONLY
class Sprite;
class BaseAnimator;
struct BlitterContext;
class CViewport;
#endif
struct LuaEventDef;
class CWormInputHandler;
class ParticleInterceptor;

class Particle : public CGameObject
{
public:
	friend class ParticleInterceptor;
	
	static LuaReference metaTable;
	virtual LuaReference getMetaTable() const { return metaTable; }

	enum Flags
	{
		FaceRight = (1<<0),
		RepPos    = (1<<1)
	};
	
	static Net_ClassID  classID;
		
	Particle(PartType* type, Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), int dir = 1, CWormInputHandler* owner = NULL, Angle angle = Angle(0));
	~Particle();

	void assignNetworkRole( bool authority );
#ifndef DEDICATED_ONLY
	void draw(CViewport* viewport);
#endif
	void think();
	Angle getPointingAngle();
	void setPointingAngle(Angle v) { m_angle = v; }
	void addAngleSpeed(AngleDiff);
#ifndef DEDICATED_ONLY
	void setAlphaFade(int frames, int dest);
#endif
	void customEvent( size_t index );
	void sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, BitStream* userdata, Net_ConnID connID);
	void damage(float amount, CWormInputHandler* damager );
	void remove();
	//virtual void deleteThis();
	virtual void finalize();
	
	int getDir() { return (flags & FaceRight) ? 1 : -1; }
	
	void setFlag(int flag) { flags |= flag; }
	void resetFlag(int flag) { flags &= ~flag; }
	
	PartType* getType()
	{
		return m_type;
	}
	
	void* operator new(size_t count);
	
	void operator delete(void* block);
	
private:

#ifndef DEDICATED_ONLY
	void drawLine2Origin(CViewport* viewport, BlitterContext const& blitter);
#endif
	
	std::vector< TimerEvent::State > timer; // This could cause a penalty
	PartType* m_type;
	float m_health;
	Angle m_angle;
	AngleDiff m_angleSpeed;
#ifndef DEDICATED_ONLY
	float m_fadeSpeed;
	float m_alpha;
	int m_alphaDest;
	SpriteSet* m_sprite;
	BaseAnimator* m_animator;
#endif
	Vec m_origin;
	
	Net_Node *m_node;
	ParticleInterceptor* interceptor;
	
	char flags;
};



#endif  // _PARTICLE_H_
