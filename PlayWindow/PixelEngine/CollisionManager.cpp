#include "CollisionManager.h"
#include "BoxCollision2D.h"
#include "GameObject.h"
#include <algorithm>
CollisionManager::CollisionManager()
{
	//CollisionList = std::vector<WPointer<BoxCollision2D>>();
}

CollisionManager::~CollisionManager()
{

}

void CollisionManager::Initialize()
{

}

void CollisionManager::Update()
{

}

void CollisionManager::Release()
{
	CollisionList.clear();
}

void CollisionManager::CollisionUpdate()
{
	//비어 있는 객체는 제거
	CollisionList.erase
	(
		std::remove_if(CollisionList.begin(), CollisionList.end(),
			[](const WPointer<BoxCollision2D>& K)
			{
				return !(K.IsValid());
			}),
		CollisionList.end()
	);


	for (auto& K : CollisionList)
	{
		for (auto& T : CollisionList)
		{
			if (K == T) continue;
			if (K.IsValid() == false || T.IsValid() == false) continue;
			PPointer<BoxCollision2D> K1 = K.Lock();
			PPointer<BoxCollision2D> T1 = T.Lock();
	
			auto r1 = K1->GetRect();
			auto r2 = T1->GetRect();
			
			if (r1.Intersects(r2))
			{
				K1->OnCollision2D(MakePixel<GameObject>(T1->targetObject));
				T1->OnCollision2D(MakePixel<GameObject>(K1->targetObject));
			}
		}
	}
}

void CollisionManager::Push(PPointer<BoxCollision2D> target)
{
	CollisionList.push_back(target);
}
