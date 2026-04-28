#include "pch.h"
#include <Jolt/Math/Math.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include "PhysManager.h"
#include "PixelEngineAPI.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "PixelGraphicsAPI.h"

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

    mBodyInterface = &physicsSystem->GetBodyInterface();
}

void PhysManager::Update()
{
    physicsSystem->Update(GetDeltaTime(), 1, tempAllocator, jobSystem);
}

void PhysManager::Clear()
{
    if (mBodyInterface == nullptr) return;

    // 1. 모든 바디 ID를 다 가져옵니다.
    JPH::BodyIDVector allBodies;
    physicsSystem->GetBodies(allBodies);

    if (!allBodies.empty())
    {
        // 2. 월드에서 모든 바디를 제거하고 파괴합니다.
        mBodyInterface->RemoveBodies(allBodies.data(), (int)allBodies.size());
        mBodyInterface->DestroyBodies(allBodies.data(), (int)allBodies.size());
    }

    // 3. 콜라이더 캐시(Map)도 비워줍니다 (새 스테이지에선 새 콜라이더를 쓸 수도 있으니까요)
    //mShapeCache.clear();
}

void PhysManager::Release()
{
	
}

void PhysManager::CollisionUpdate()
{
	
}

JPH::ShapeRefC PhysManager::CreateBoxCollider2D(float x, float y)
{
    float halfX = x * 0.5f;
    float halfY = y * 0.5f;
    float halfZ = 1.0f;
    JPH::BoxShapeSettings box = JPH::BoxShapeSettings(JPH::Vec3(halfX, halfY, halfZ));
    box.mConvexRadius = 0.01f;
    JPH::ShapeSettings::ShapeResult boxResult = box.Create();
    if (!boxResult.IsValid())
    {
        PixelLog::Info(boxResult.GetError().c_str());
        return nullptr;
    }
    return boxResult.Get();
}

JPH::BodyID PhysManager::CreateBody2D(JPH::BodyCreationSettings* setting, bool active,void* pOwner)
{
    auto id =  mBodyInterface->CreateAndAddBody(*setting, active == true? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
    mBodyInterface->SetUserData(id, reinterpret_cast<JPH::uint64>(pOwner));
    return id;
}


void PhysManager::SyncPhysics(JPH::BodyID id)
{
    uint64 userData = mBodyInterface->GetUserData(id);
    if (userData == 0) return;

    GameObject* obj = reinterpret_cast<GameObject*>(userData);
    if (obj == nullptr)return;

    JPH::RVec3 pos = mBodyInterface->GetPosition(id);
    JPH::Quat rot = mBodyInterface->GetRotation(id);

    auto tr = obj->GetTransform();

    tr->Position.X = pos.GetX();
    tr->Position.Y = pos.GetY();

    float angleInRadians = rot.GetEulerAngles().GetZ();
    float angleInDegrees = angleInRadians * (180.0f / 3.141592f);
    tr->Rotation.Z = -angleInDegrees;

    JPH::ShapeRefC shape = mBodyInterface->GetShape(id);
    JPH::AABox localBounds = shape->GetLocalBounds();
    float hw = localBounds.GetExtent().GetX(); // 가로 절반
    float hh = localBounds.GetExtent().GetY(); // 세로 절반

    JPH::Vec3 p1(-hw, -hh, 0);
    JPH::Vec3 p2(hw, -hh, 0);
    JPH::Vec3 p3(hw, hh, 0);
    JPH::Vec3 p4(-hw, hh, 0);

    auto ToWorld = [&](JPH::Vec3 localPoint) {
        JPH::Vec3 worldPoint = pos + rot * localPoint;
        return worldPoint;
        };

    JPH::Vec3 w1 = ToWorld(p1);
    JPH::Vec3 w2 = ToWorld(p2);
    JPH::Vec3 w3 = ToWorld(p3);
    JPH::Vec3 w4 = ToWorld(p4);

    // 5. 4개의 선을 그어 박스 완성! (빨간색 255, 0, 0 가정)
    // DrawLine(x1, y1, z1, x2, y2, z2, r, g, b)
    DrawLine(w1.GetX(), w1.GetY(), 0, w2.GetX(), w2.GetY(), 0, 1.0f, 0.0f, 0.0f);
    DrawLine(w2.GetX(), w2.GetY(), 0, w3.GetX(), w3.GetY(), 0, 1.0f, 0.0f, 0.0f);
    DrawLine(w3.GetX(), w3.GetY(), 0, w4.GetX(), w4.GetY(), 0, 1.0f, 0.0f, 0.0f);
    DrawLine(w4.GetX(), w4.GetY(), 0, w1.GetX(), w1.GetY(), 0, 1.0f, 0.0f, 0.0f);
}
