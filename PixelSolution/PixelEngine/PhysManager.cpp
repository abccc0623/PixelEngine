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

    JPH::Quat targetRotation = JPH::Quat::sIdentity();
    physicsSystem->GetBodyInterface().SetRotation(id, targetRotation, (active) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

void PhysManager::CollisionUpdate()
{
	
}

void PhysManager::DebugDraw(JPH::BodyID id)
{
    JPH::ShapeRefC shape = mBodyInterface->GetShape(id);

    float cx = 0.0f;
    float cy = 0.0f;
    float hw = 0.0f;
    float hh = 0.0f;

    JPH::RVec3 pos = mBodyInterface->GetPosition(id);
    JPH::Quat rot = mBodyInterface->GetRotation(id);

    // 3. 오프셋(Center)이 적용된 모양인지 확인 후 값 직접 추출!
    if (shape->GetSubType() == JPH::EShapeSubType::RotatedTranslated)
    {
        // Jolt의 껍데기를 벗겨서 우리가 만든 RotatedTranslatedShape로 캐스팅합니다.
        auto rtShape = static_cast<const JPH::RotatedTranslatedShape*>(shape.GetPtr());

        // 우리가 BoxCollider2D에서 넣었던 (centerX, centerY) 값을 직접 뜯어옵니다.
        JPH::Vec3 offset = rtShape->GetPosition();
        cx = offset.GetX();
        cy = offset.GetY();

        // 진짜 알맹이(Box)의 크기를 가져옵니다.
        JPH::AABox innerBounds = rtShape->GetInnerShape()->GetLocalBounds();
        hw = innerBounds.GetExtent().GetX();
        hh = innerBounds.GetExtent().GetY();
    }
    else
    {
        // 오프셋이 없는 기본 박스라면 기존 방식대로 처리
        JPH::AABox localBounds = shape->GetLocalBounds();
        cx = localBounds.GetCenter().GetX();
        cy = localBounds.GetCenter().GetY();
        hw = localBounds.GetExtent().GetX();
        hh = localBounds.GetExtent().GetY();
    }

    // 4. (cx, cy)를 기준으로 박스의 4개 꼭짓점 계산
    JPH::Vec3 p1(cx - hw, cy - hh, 0);
    JPH::Vec3 p2(cx + hw, cy - hh, 0);
    JPH::Vec3 p3(cx + hw, cy + hh, 0);
    JPH::Vec3 p4(cx - hw, cy + hh, 0);

    // 5. 로컬 좌표를 월드 좌표로 변환
    auto ToWorld = [&](JPH::Vec3 localPoint)
        {
        // 회전과 위치를 적용하여 월드에 배치
        return pos + rot * localPoint;
        };

    JPH::Vec3 w1 = ToWorld(p1);
    JPH::Vec3 w2 = ToWorld(p2);
    JPH::Vec3 w3 = ToWorld(p3);
    JPH::Vec3 w4 = ToWorld(p4);

    // 6. 드디어! 정확한 위치에 디버그 선 그리기
    DrawLine(w1.GetX(), w1.GetY(), 0, w2.GetX(), w2.GetY(), 0, 1.0f, 0.0f, 0.0f);
    DrawLine(w2.GetX(), w2.GetY(), 0, w3.GetX(), w3.GetY(), 0, 1.0f, 0.0f, 0.0f);
    DrawLine(w3.GetX(), w3.GetY(), 0, w4.GetX(), w4.GetY(), 0, 1.0f, 0.0f, 0.0f);
    DrawLine(w4.GetX(), w4.GetY(), 0, w1.GetX(), w1.GetY(), 0, 1.0f, 0.0f, 0.0f);
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
    if (obj == nullptr) return;

    JPH::RVec3 pos = mBodyInterface->GetPosition(id);
    JPH::Quat rot = mBodyInterface->GetRotation(id);

    auto tr = obj->GetTransform();

    // 1. 스프라이트(돼지) 위치 및 회전 동기화
    tr->Position.X = pos.GetX();
    tr->Position.Y = pos.GetY();

    float angleInRadians = rot.GetEulerAngles().GetZ();
    float angleInDegrees = angleInRadians * (180.0f / 3.141592f);
    tr->Rotation.Z = -angleInDegrees;

    DebugDraw(id);
}
