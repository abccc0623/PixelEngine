#pragma once
class EngineManager
{
public:
    EngineManager() = default;
    virtual ~EngineManager() = default;

   virtual void Initialize() = 0;
   virtual void Update() = 0;
   virtual void ReleaseShared() = 0;
};

