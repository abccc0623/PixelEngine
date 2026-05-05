#include <Jolt/Physics/Collision/ContactListener.h>
#include <mutex>
#include <vector>

// 충돌 이벤트 데이터를 담을 구조체
struct CollisionEvent
{
    JPH::BodyID body1;
    JPH::BodyID body2;
    JPH::Vec3 contactPosition;
    JPH::Vec3 contactNormal;
    bool isEnter; // true: OnContactAdded (충돌 시작), false: OnContactRemoved (충돌 종료)
};

class EventManager;
class PhysListener : public JPH::ContactListener 
{
public:
	PhysListener();
	virtual ~PhysListener();

    virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
    virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
    void FlushEvents(std::vector<CollisionEvent>& outEvents);
private:
    void EnqueueEvent(const CollisionEvent& ev);

	std::mutex mEventMutex;
	std::vector<CollisionEvent> mEventQueue;

    EventManager* event;
};