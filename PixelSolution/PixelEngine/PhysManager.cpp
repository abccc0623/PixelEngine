#include "pch.h"
#include <Jolt/Math/Math.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include "PhysManager.h"
#include "PixelEngineAPI.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "Type/GlobalEnum.h"
#include "ColliderFactory.h"
#include "PixelGraphicsAPI.h"
#include "PhysListener.h"
#include "EventManager.h"
#include "PixelEngine.h"
#include "Core/GameObject.h"

using namespace JPH;

namespace Hierachy
{
    // 타입 인식을 위해 안쪽에서도 네임스페이스 사용
    using namespace JPH;

    namespace Layers {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING     = 1;
        static constexpr uint32 NUM_LAYERS      = 2;
    }

    namespace BP_Layers {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint32 NUM_LAYERS = 2;
    }

    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter {
    public:
        virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override {
            if (inObject1 == Layers::NON_MOVING) return inObject2 == Layers::MOVING;
            return true;
        }
    };

    class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface {
    public:
        BPLayerInterfaceImpl() {
            mObjectToBroadPhase[Layers::NON_MOVING] = BP_Layers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BP_Layers::MOVING;
        }
        virtual uint GetNumBroadPhaseLayers() const override { return BP_Layers::NUM_LAYERS; }
        virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override { return mObjectToBroadPhase[inLayer]; }
        //virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override { return "Layer"; }
    private:
        BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter {
    public:
        virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override {
            if (inLayer1 == Layers::NON_MOVING) return inLayer2 == BP_Layers::MOVING;
            return true;
        }
    };
}


PhysManager::PhysManager()
{
    
}

PhysManager::~PhysManager()
{
    delete tempAllocator;
    delete jobSystem;
    delete physicsSystem;

    delete mBpInterface;
    delete mObjVsBpFilter;
    delete mObjVsObjFilter;

    delete colliderFactory;
    delete eventListener;
}
void MyTrace(const char* inFMT, ...) {
    char buffer[1024];
    va_list list;
    va_start(list, inFMT);
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    std::string log(buffer);
    PixelLog::Info(log);
}

extern PixelEngine* Engine;
void PhysManager::Initialize()
{
    // 2. Jolt에게 "할 말 있으면 여기다 말해"라고 등록
    JPH::Trace = MyTrace;

    RegisterDefaultAllocator();
    Factory::sInstance = new Factory();
    RegisterTypes();

    // 2. 실행 환경 메모리/스레드 설정
    tempAllocator = new TempAllocatorImpl(10 * 1024 * 1024);
    jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);


    // 3. 필터 인스턴스 생성 (나중에 해제 필요)
    mBpInterface = new Hierachy::BPLayerInterfaceImpl();
    mObjVsBpFilter = new Hierachy::ObjectVsBroadPhaseLayerFilterImpl();
    mObjVsObjFilter = new Hierachy::ObjectLayerPairFilterImpl();

    // 4. 물리 시스템 생성
    physicsSystem = new PhysicsSystem();
    physicsSystem->Init(
        10240, 0, 1024, 1024,
        *static_cast<Hierachy::BPLayerInterfaceImpl*>(mBpInterface),
        *static_cast<Hierachy::ObjectVsBroadPhaseLayerFilterImpl*>(mObjVsBpFilter),
        *static_cast<Hierachy::ObjectLayerPairFilterImpl*>(mObjVsObjFilter)
    );

    colliderFactory = new ColliderFactory();
    eventListener = new PhysListener();

    event = Engine->GetFactory<EventManager>();
    mBodyInterface = &physicsSystem->GetBodyInterface();
    physicsSystem->SetContactListener(eventListener);
}

void PhysManager::Update()
{
    physicsSystem->Update(GetDeltaTime(), 1, tempAllocator, jobSystem);

    mFrameCollisionEvents.clear();
    eventListener->FlushEvents(mFrameCollisionEvents);

    for (const auto& ev : mFrameCollisionEvents)
    {
        Event eventMessage;
        eventMessage.Collision.targetIn = ev.isEnter;
        eventMessage.Collision.target1 = reinterpret_cast<GameObject*>(mBodyInterface->GetUserData(ev.body1));
        eventMessage.Collision.target2 = reinterpret_cast<GameObject*>(mBodyInterface->GetUserData(ev.body2));
        if (ev.isEnter)
        {
            event->TriggerEvent(EventType::CollisionIn, eventMessage);
        }
        else
        {
            event->TriggerEvent(EventType::CollisionOut, eventMessage);
        }
    }
}

void PhysManager::Clear()
{
    if (mBodyInterface == nullptr) return;

    // 1. 모든 바디 ID를 다 가져옵니다.
    JPH::BodyIDVector allBodies;
    physicsSystem->GetBodies(allBodies);

    if (!allBodies.empty())
    {
        mBodyInterface->RemoveBodies(allBodies.data(), (int)allBodies.size());
        mBodyInterface->DestroyBodies(allBodies.data(), (int)allBodies.size());
    }
    colliderMap.clear();
    colliderFactory->Clear();
}

void PhysManager::Release()
{
	
}

void PhysManager::SetVelocity(JPH::BodyID id, float x, float y, float z)
{
    JPH::Vec3 targetVelocity(x, y, z);
    physicsSystem->GetBodyInterface().SetLinearVelocity(id, targetVelocity);
}

void PhysManager::SetVelocityX(JPH::BodyID id, float x)
{
    JPH::Vec3 targetVelocity = physicsSystem->GetBodyInterface().GetLinearVelocity(id);
    targetVelocity.SetX(x);
    physicsSystem->GetBodyInterface().SetLinearVelocity(id, targetVelocity);
}

void PhysManager::SetVelocityY(JPH::BodyID id, float y)
{
    JPH::Vec3 targetVelocity = physicsSystem->GetBodyInterface().GetLinearVelocity(id);
    targetVelocity.SetY(y);
    physicsSystem->GetBodyInterface().SetLinearVelocity(id, targetVelocity);
}

void PhysManager::SetVelocityZ(JPH::BodyID id, float z)
{
    JPH::Vec3 targetVelocity = physicsSystem->GetBodyInterface().GetLinearVelocity(id);
    targetVelocity.SetZ(z);
    physicsSystem->GetBodyInterface().SetLinearVelocity(id, targetVelocity);
}

void PhysManager::SetPosition(JPH::BodyID id, float x, float y, float z,bool active)
{
    JPH::Vec3 targetPosition(x, y, z);
    physicsSystem->GetBodyInterface().SetPosition(id, targetPosition, (active) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

void PhysManager::SetRotation(JPH::BodyID id, float x, float y, float z, bool active)
{
    constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
    JPH::Vec3 eulerRadians(x * DEG_TO_RAD, y * DEG_TO_RAD, z * DEG_TO_RAD);
    JPH::Quat targetRotation = JPH::Quat::sEulerAngles(eulerRadians);
    physicsSystem->GetBodyInterface().SetRotation(
        id,
        targetRotation,
        (active) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate
    );
}

void PhysManager::AddImpulse(JPH::BodyID id, float x, float y, float z)
{
    physicsSystem->GetBodyInterface().AddImpulse(id, JPH::Vec3(x, y, z));
}
void PhysManager::AddForce(JPH::BodyID id, float x, float y, float z)
{
    physicsSystem->GetBodyInterface().AddForce(id, JPH::Vec3(x, y, z));
}

JPH::ShapeRefC PhysManager::CreateCollider(PhysCollider& collider)
{
    switch (collider.colliderType)
    {
    case 0: return colliderFactory->CreateBox2D(collider);
    case 1: return colliderFactory->CreateCircle2D(collider);
    }
}

JPH::BodyID PhysManager::CreateRigidbody(PhysRigidbody& rigidbody, JPH::ShapeRefC shapeRef, void* pOwner)
{
    JPH::ObjectLayer objectLayer = Hierachy::Layers::MOVING;
    JPH::EMotionType eMotionType = JPH::EMotionType::Dynamic;
    JPH::EActivation active = (rigidbody.Active) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
    switch (rigidbody.MotionType)
    {
    case (int)ColliderMotionType::Static:
        objectLayer = Hierachy::Layers::NON_MOVING;
        eMotionType = EMotionType::Static;
        break;
    case (int)ColliderMotionType::Kinematic:
        eMotionType = EMotionType::Kinematic;
        break;
    case (int)ColliderMotionType::Dynamic:
        objectLayer = Hierachy::Layers::MOVING;
        eMotionType = EMotionType::Dynamic;
        break;
    }
    JPH::BodyCreationSettings bodySettings
    (
        shapeRef,
        JPH::Vec3(0, 0, 0),
        JPH::Quat::sIdentity(), // 초기 회전값
        eMotionType,
        objectLayer
    );
    bodySettings.mGravityFactor = rigidbody.Gravity;
    bodySettings.mAllowSleeping = rigidbody.AutoSleep;
    bodySettings.mIsSensor = rigidbody.Sensor;
    bodySettings.mRestitution = rigidbody.Restitution;
    bodySettings.mFriction = rigidbody.Friction;
    bodySettings.mLinearDamping = rigidbody.LinearDamping;

    bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::All;
    auto ApplyLock = [&](int index, JPH::EAllowedDOFs dofFlag)
        {
            bool lock = rigidbody.Lock[index];
            if (lock == true) bodySettings.mAllowedDOFs &= ~dofFlag;
        };
    ApplyLock(0, JPH::EAllowedDOFs::TranslationX);
    ApplyLock(1, JPH::EAllowedDOFs::TranslationY);
    ApplyLock(2, JPH::EAllowedDOFs::TranslationZ);
    ApplyLock(3, JPH::EAllowedDOFs::RotationX);
    ApplyLock(4, JPH::EAllowedDOFs::RotationY);
    ApplyLock(5, JPH::EAllowedDOFs::RotationZ);
    auto id = mBodyInterface->CreateAndAddBody(bodySettings, active);
    mBodyInterface->SetUserData(id, reinterpret_cast<JPH::uint64>(pOwner));
    return id;
}

void PhysManager::DebugDraw(JPH::BodyID id)
{
   JPH::ShapeRefC rootShape = mBodyInterface->GetShape(id);
    if (rootShape == nullptr) return;

    JPH::RVec3 pos = mBodyInterface->GetPosition(id);
    JPH::Quat rot = mBodyInterface->GetRotation(id);

    // 1. 데코레이터(껍데기) 해제 및 로컬 오프셋 추출
    JPH::Vec3 localOffset = JPH::Vec3::sZero();
    const JPH::Shape* actualShape = rootShape.GetPtr();

    if (actualShape->GetSubType() == JPH::EShapeSubType::RotatedTranslated)
    {
        auto rtShape = static_cast<const JPH::RotatedTranslatedShape*>(actualShape);
        localOffset = rtShape->GetPosition();
        
        actualShape = rtShape->GetInnerShape(); 
    }

    // 2. 실제 Shape 타입에 따른 렌더링 분기
    switch (actualShape->GetSubType())
    {
        case JPH::EShapeSubType::Box:
        {
            auto boxShape = static_cast<const JPH::BoxShape*>(actualShape);
            JPH::Vec3 extent = boxShape->GetHalfExtent(); // AABox 대신 실제 박스의 절반 크기를 가져옴

            JPH::Vec3 p[4] = {
                JPH::Vec3(localOffset.GetX() - extent.GetX(), localOffset.GetY() - extent.GetY(), 0),
                JPH::Vec3(localOffset.GetX() + extent.GetX(), localOffset.GetY() - extent.GetY(), 0),
                JPH::Vec3(localOffset.GetX() + extent.GetX(), localOffset.GetY() + extent.GetY(), 0),
                JPH::Vec3(localOffset.GetX() - extent.GetX(), localOffset.GetY() + extent.GetY(), 0)
            };
            JPH::Vec3 w[4];
            for (int i = 0; i < 4; ++i) 
            {
                w[i] = pos + rot * p[i];
            }
            DrawLine(w[0].GetX(), w[0].GetY(), 0, w[1].GetX(), w[1].GetY(), 0, 1.0f, 0.0f, 0.0f);
            DrawLine(w[1].GetX(), w[1].GetY(), 0, w[2].GetX(), w[2].GetY(), 0, 1.0f, 0.0f, 0.0f);
            DrawLine(w[2].GetX(), w[2].GetY(), 0, w[3].GetX(), w[3].GetY(), 0, 1.0f, 0.0f, 0.0f);
            DrawLine(w[3].GetX(), w[3].GetY(), 0, w[0].GetX(), w[0].GetY(), 0, 1.0f, 0.0f, 0.0f);
            break;
        }
        case JPH::EShapeSubType::Sphere:
        {
            auto sphereShape = static_cast<const JPH::SphereShape*>(actualShape);
            float radius = sphereShape->GetRadius();
            JPH::Vec3 worldCenter = pos + rot * localOffset;
            DrawCircle2D(worldCenter.GetX(), worldCenter.GetY(), 0, radius, 0.0f, 1.0f, 0.0f);
            break;
        }
        default:
        {
            break;
        }
    }
}

void PhysManager::SyncPhysics(JPH::BodyID id)
{
    uint64 userData = mBodyInterface->GetUserData(id);
    if (userData == 0) return;

    GameObject* obj = reinterpret_cast<GameObject*>(userData);

    if (obj == nullptr) return;
    JPH::RVec3 pos = mBodyInterface->GetPosition(id);
    JPH::Quat rot = mBodyInterface->GetRotation(id);

    auto tr = obj->GetTransform();
    if (tr == nullptr) return;

    tr->Position.X = pos.GetX();
    tr->Position.Y = pos.GetY();

    float angleInRadians = rot.GetEulerAngles().GetZ();
    tr->Rotation.Z = -JPH::RadiansToDegrees(angleInRadians);
    DebugDraw(id);
}


